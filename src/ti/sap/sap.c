/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <string.h>
#include <ti/npi/hal_defs.h>

#include "npi_ss_ble_sap.h"

#include "snp_rpc_synchro.h"
#include "snp_rpc.h"
#include "sap.h"

#ifdef SNP_LOCAL
#include "simple_np_dev.h"
#include "simple_np_gap.h"
#include "simple_np_gatt.h"
#include "simple_np.h"
#include "util.h"
#endif //SNP_LOCAL

#include <ti/npi/npi_task.h>

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

#define SAP_MAX_ADV_SCAN_DATA_LEN 31 // maximum of 31 bytes allowed in advertising
                                     // and scan reponse data by BLE protocol

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

// GAP event callback node.
typedef struct
{
  struct eventCBNode_t *next;      // pointer to next callback node in the list.
  uint16_t             eventmask;  // eventMask of events this callback listens for.
  pfnEventCB_t         eventCB;    // Callback function
} eventCBNode_t;

// Asynchronous event callback node.
typedef struct
{
  struct asyncCBNode_t *next;  // pointer to next callback node in the list.
  uint8_t subystem;            // Specify the NP subsystem: GAP(HCI), Conn or ADV or GATT.
  pfnAsyncCB_t asyncCB;
} asyncCBNode_t;

// Service node
typedef struct
{
  struct serviceNode_t *next; // pointer to next service node in the list
  void *context; // stored context of service
  pfnGATTReadAttrCB_t charReadCB; // pointer to registered service read call back
  pfnGATTWriteAttrCB_t charWriteCB; // pointer to registered service write call back
  pfnCCCDIndCB_t cccdIndCB; // pointer to registered service cccb request call back
  uint16_t minHandle; // minimum handle in service
  uint16_t maxHandle; // maximum handle in service
} serviceNode_t;

// Root of the event callback list.
eventCBNode_t *eventCBListRoot = NULL;

// Root of the async callback list.
asyncCBNode_t *asyncCBListRoot = NULL;

// Root of the service list
serviceNode_t *serviceListRoot = NULL;


/* Default Remote Port UART */
const SAP_RemotePort_t default_remUARTPort = {
    .stackSize          = 1024,
    .bufSize            = 530,
    .mrdyPinID          = (uint32_t)~(0),
    .srdyPinID          = (uint32_t)~(0),
    .boardID            = 0,
    .bitRate            = 115200
};

/* Default Remote Port SPI */
const SAP_RemotePort_t default_remSPIPort = {
    .stackSize          = 1024,
    .bufSize            = 530,
    .mrdyPinID          = (uint32_t)~(0),
    .srdyPinID          = (uint32_t)~(0),
    .boardID            = 0,
    .bitRate            = 800000
};

/*********************************************************************
 * LOCAL VARIABLES
 */

uint8_t SAP_isConnected = 0;

/*********************************************************************
 * HELPER FUNCTIONS
 */

static uint8_t SAP_addToEventCBList(pfnEventCB_t eventCB, uint16_t eventMask)
{
  eventCBNode_t *newNode;

  //Create node
  newNode = (eventCBNode_t *)SNP_malloc(sizeof(eventCBNode_t));

  if (newNode == NULL)
  {
    return SNP_FAILURE;
  }

  // Populate the node.
  newNode->eventCB = eventCB;
  newNode->eventmask = eventMask;
  newNode->next = NULL;

  // Check if this is the first event
  if(eventCBListRoot == NULL)
  {
    eventCBListRoot = newNode;
  }
  else
  {
    eventCBNode_t *iter = eventCBListRoot;

    // Iterate through the list.
    while(iter->next != NULL)
    {
      iter = (eventCBNode_t *)iter->next;
    }

    // add to the end of the list.
    iter->next = (struct eventCBNode_t *)newNode;
  }

  return SNP_SUCCESS;
}

/*
 * Add an application asynchronous callback to the ASYNC callback list.
 */
static uint8_t SAP_addToAsyncCBList(pfnAsyncCB_t eventCB)
{
  asyncCBNode_t *newNode;

  // Create node.
  newNode = (asyncCBNode_t *)SNP_malloc(sizeof(asyncCBNode_t));

  if (newNode == NULL)
  {
    return SNP_FAILURE;
  }

  // Populate the node.
  newNode->asyncCB = eventCB;
  newNode->next = NULL;

  // Check if this is the first event.
  if(asyncCBListRoot == NULL)
  {
    // Make this node the root.
    asyncCBListRoot = newNode;
  }
  else
  {
    asyncCBNode_t *iter = asyncCBListRoot;

    // Iterate through the list.
    while(iter->next != NULL)
    {
      iter = (asyncCBNode_t *)iter->next;
    }

    // Add to the end of the list.
    iter->next = (struct asyncCBNode_t *)newNode;
  }

  return SNP_SUCCESS;
}

/*
 * Iterate through event callbacks and send an event up to those which are
 * listening for it.
 */
static void getEventCallbacks(snpEvt_t *pEvt)
{
  eventCBNode_t *iter = eventCBListRoot;

  // Send to all callbacks which requested this event.
  while(iter != NULL)
  {
    // If this callback listens for this event.
    if (iter->eventmask & pEvt->event != 0x0000)
    {
      // Send to callback.
      if (iter->eventCB)
      {
        iter->eventCB(pEvt->event, (snpEventParam_t *) pEvt->pEvtParams);
      }
    }

    // Next callback.
    iter = (eventCBNode_t *)iter->next;
  }
}

/*
 * Iterate through list of asynchronous callbacks and
 */
void handleAsyncCB(uint8_t cmd1, snp_msg_t *pMsg, uint16_t msgLen)
{
  asyncCBNode_t *iter = asyncCBListRoot;
  serviceNode_t *curr = serviceListRoot;

  switch(SNP_GET_OPCODE_HDR_CMD1(cmd1))
  {
    case SNP_GATT_GRP:
      // GATT Request, determine which service call back to invoke based upon
      // the handle of the request.
        switch (cmd1)
        {
          case SNP_CHAR_READ_IND:
            {
              snpCharReadCnf_t lCnf;
              uint16_t size = 0;
              snpCharReadInd_t *rI = (snpCharReadInd_t *)pMsg;

              // Initialize Confirmation Struct
              lCnf.connHandle = rI->connHandle;
              lCnf.attrHandle = rI->attrHandle;
              lCnf.offset = rI->offset;
              lCnf.pData = (uint8_t *)SNP_malloc(rI->maxSize);
              lCnf.status = SNP_FAILURE;

              // Searching for service that contains handle
              while (curr != NULL)
              {
                if (rI->attrHandle >= curr->minHandle &&
                     rI->attrHandle <= curr->maxHandle)
                {
                  // Found correct service. If call back is registered invoke,
                  // else return error status
                  if (curr->charReadCB)
                  {
                    lCnf.status = curr->charReadCB(curr->context, rI->connHandle,
                                                        rI->attrHandle, rI->offset,
                                                        rI->maxSize, &size,
                                                        lCnf.pData);
                  }
                  break;
                }
                curr = (serviceNode_t *)curr->next;
              }

              /*
               * Note: event when SNP_LOCAL is defined, in order to not send
               * an ICall message to the stack while executing in the context of
               * the stack, the confirmation is queued up to be executed within
               * the application's context.
               */
              SNP_RPC_readCharCnf(&lCnf, size);

              SNP_free(lCnf.pData);
            }
            break;

          case SNP_CHAR_WRITE_IND:
            {
              snpCharWriteCnf_t lCnf;
              snpCharWriteInd_t *wI = (snpCharWriteInd_t *)pMsg;
              // msgLen is the amount of payload bytes. Adjust it to only be
              // the amount of bytes to be written to the characteristic
              msgLen = msgLen - sizeof(pMsg->charWriteInd) +
                       sizeof(pMsg->charWriteInd.pData);

              // Initialize Confirmation Struct
              lCnf.status = SNP_FAILURE;
              lCnf.connHandle = wI->connHandle;

              // Searching for service that contains handle
              while (curr != NULL)
              {
                if (wI->attrHandle >= curr->minHandle &&
                     wI->attrHandle <= curr->maxHandle)
                {
                  // Found correct service. If call back is registered invoke,
                  // else return error status
                  if (curr->charWriteCB)
                  {

                    lCnf.status = curr->charWriteCB(curr->context, wI->connHandle,
                                                         wI->attrHandle, msgLen,
                                                         wI->pData);
                  }
                  break;
                }
                curr = (serviceNode_t *)curr->next;
              }

              /*
               * Note: event when SNP_LOCAL is defined, in order to not send
               * an ICall message to the stack while executing in the context of
               * the stack, the confirmation is queued up to be executed within
               * the application's context.
               */
              // Respond to write request
              SNP_RPC_writeCharCnf(&lCnf);
            }
            break;

          case SNP_CCCD_UPDATED_IND:
            {
              snpCharCfgUpdatedRsp_t lRsp;
              snpCharCfgUpdatedInd_t *cu  = (snpCharCfgUpdatedInd_t *)pMsg;

              // Initialize Response Struct
              lRsp.status = SNP_FAILURE;
              lRsp.connHandle = cu->connHandle;

              while (curr != NULL)
              {
                if (cu->cccdHandle >= curr->minHandle &&
                     cu->cccdHandle <= curr->maxHandle)
                {
                  if (curr->cccdIndCB)
                  {
                    // Found correct service. If call back is registered invoke,
                    // else return error status
                    lRsp.status = curr->cccdIndCB(curr->context, cu->connHandle,
                                                       cu->cccdHandle, cu->rspNeeded,
                                                       cu->value);
                  }
                  break;
                }
                curr = (serviceNode_t *)curr->next;
              }

              /*
               * Note: event when SNP_LOCAL is defined, in order to not send
               * an ICall message to the stack while executing in the context of
               * the stack, the confirmation is queued up to be executed within
               * the application's context.
               */
              // Respond to CCCD Indication
              SNP_RPC_charConfigUpdatedRsp(&lRsp);
            }
            break;

          default:
            break;
        }
      break;

    default:
      // Not GATT request, pass on to all registered call backs
      while(iter != NULL)
      {
        // Send to callback.
        iter->asyncCB(cmd1, pMsg);

        // Next callback.
        iter = (asyncCBNode_t *)iter->next;
      }
      break;
  }
}

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @fn      SAP_initParams
 *
 * @brief   Initializes network processor port parameters
 *
 * @param   portType - if port is NPI UART or SPI or local
 * @param   params   - parameter data to initialize.
 *
 * @return  SNP_SUCCESS if port supported by build, else SNP_INVALID_PARAMS.
 */
uint8_t SAP_initParams(uint8_t portType, SAP_Params *params)
{
  uint8_t status = SNP_SUCCESS;

  if (params != NULL)
  {
    params->portType = portType;

    if (portType == SAP_PORT_LOCAL)
    {
#ifdef SNP_LOCAL
      // Do nothing.
#else //!SNP_LOCAL
      status = SNP_INVALID_PARAMS;
#endif //SNP_LOCAL
    }
    else if (portType == SAP_PORT_REMOTE_UART)
    {
#ifdef NPI_USE_UART
      params->port.remote = default_remUARTPort;
#else //!NPI_USE_UART
      status = SNP_INVALID_PARAMS;
#endif //NPI_USE_UART
    }
    else if (portType == SAP_PORT_REMOTE_SPI)
    {
#ifdef NPI_USE_SPI
      params->port.remote = default_remSPIPort;
#else //!NPI_USE_SPI
      status = SNP_INVALID_PARAMS;
#endif //NPI_USE_SPI
    }
  }

  return status;
}

/**
 * @fn      SAP_open
 *
 * @brief   Opens the port with the network processor
 *
 * @param   params - list of parameters needed to initialize serial port used
 *                   with network processor
 *
 * @return  uint8_t - SNP_SUCCESS if NPI is open
 */
uint8_t SAP_open(SAP_Params *params)
{
#ifndef SNP_LOCAL
  NPI_Params npiParams;
#endif //SNP_LOCAL
  SAP_Params sapParams;

  // Check to see if params were given
  if (params == NULL)
  {
    SAP_initParams(SAP_PORT_REMOTE_UART,&sapParams);
    params = &sapParams;
  }

  // set SNP TL Callback to receive events from NP.
  SNP_RPC_registerSAPCBs(handleAsyncCB, getEventCallbacks);

#ifdef SNP_LOCAL
  SNP_open(params->port.local.syncHandle);

  SAP_RegisterCBack(NPISS_BLE_SNP_msgFromSNP);

  return SNP_SUCCESS;
#else //!SNP_LOCAL
  SNP_open(NULL);

  // Initialize Network Processor Interface
  if (params->portType == SAP_PORT_REMOTE_UART ||
      params->portType == SAP_PORT_REMOTE_SPI)
  {
    NPITask_Params_init(&npiParams);
  }
  else
  {
    return SNP_FAILURE;
  }

  // Update npi params with sap port params
  npiParams.stackSize         = params->port.remote.stackSize;
  npiParams.bufSize           = params->port.remote.bufSize;
  npiParams.mrdyPinID         = params->port.remote.mrdyPinID;
  npiParams.srdyPinID         = params->port.remote.srdyPinID;
  npiParams.portBoardID       = params->port.remote.boardID;

  if (params->portType == SAP_PORT_REMOTE_UART)
  {
    npiParams.portParams.uartParams.baudRate = params->port.remote.bitRate;
  }
  else if (params->portType == SAP_PORT_REMOTE_SPI)
  {
    npiParams.portParams.spiParams.bitRate = params->port.remote.bitRate;
  }

  if (NPITask_open(&npiParams) == NPI_SUCCESS)
  {
    // Setup NPI SNP subsystem
    NPISS_BLE_SNP_init();
    return SNP_SUCCESS;
  }

  return SNP_FAILURE;
#endif //SNP_LOCAL
}

/**
 * @fn      SAP_close
 *
 * @brief   Tears down the port with the network processor
 *
 * @param   None.
 *
 * @return  uint8_t - SNP_SUCCESS if able to close
 */
uint8_t SAP_close(void)
{
  asyncCBNode_t *aNode = asyncCBListRoot;
  asyncCBNode_t *aTempNode = NULL;
  eventCBNode_t *eNode = eventCBListRoot;
  eventCBNode_t *eTempNode = NULL;
  serviceNode_t *sNode = serviceListRoot;
  serviceNode_t *sTempNode = NULL;

  // Clean up call back and service lists
  while(aNode != NULL)
  {
    aTempNode = (asyncCBNode_t *)aNode->next;
    SNP_free(aNode);
    aNode = aTempNode;
  }
  asyncCBListRoot = NULL;

  while(eNode != NULL)
  {
    eTempNode = (eventCBNode_t *)eNode->next;
    SNP_free(eNode);
    eNode = eTempNode;
  }
  eventCBListRoot = NULL;

  while(sNode != NULL)
  {
    sTempNode = (serviceNode_t *)sNode->next;
    SNP_free(sNode);
    sNode = sTempNode;
  }
  serviceListRoot = NULL;

  SNP_close();

#ifdef SNP_LOCAL
  return NPI_SUCCESS;
#else //!SNP_LOCAL
  return NPITask_close();
#endif //SNP_LOCAL
}

/**
 * @fn      SAP_setAsyncCB
 *
 * @brief   setup Applications' asynchronous callbacks.  This must be called before
 *          using any other calls to SAP.  This function may be called multiple times
 *          to register multiple Callbacks.
 *
 * @param   asyncCB - the asynchronous callback.
 *
 * @return  None.
 */
uint8_t SAP_setAsyncCB(pfnAsyncCB_t asyncCB)
{
  // Register callback.
  return SAP_addToAsyncCBList(asyncCB);
}

/**
 * @fn      SAP_reset
 *
 * @brief   Reset the network processor.
 *
 * @param   None.
 *
 * @return  None.
 */
uint8_t SAP_reset(void)
{
  snpHciCmdReq_t lReq;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Initialize Request Struct
  lReq.opcode = SNP_HCI_OPCODE_EXT_RESET_SYSTEM;
  if ((lReq.pData = (uint8_t *)SNP_malloc(sizeof(uint8_t))))
  {
    lReq.pData[0] = 0x01; // Default to Soft Reset

#ifdef SNP_LOCAL
    status = SNP_executeHCIcmd(&lReq, sizeof(uint8_t));
#else
    status = SNP_RPC_sendHCICommand(&lReq, sizeof(uint8_t));
#endif //SNP_LOCAL

    // Free Allocated Mem in Request Struct
    SNP_free(lReq.pData);
  }

  return status;
}

/**
 * @fn          SAP_registerService
 *
 * @brief       Add a service to the GATT server.
 *
 * @param       service - data to construct the service.
 *
 * @return      SNP_SUCCESS: The service has been registered.
 *              SNP_FAILURE: service registration failed.
 */

uint8_t SAP_registerService(SAP_Service_t *service)
{
  snpAddServiceReq_t lAddServReq;
  snpRegisterServiceRsp_t regServRsp;
  serviceNode_t *newNode = NULL;
  serviceNode_t *currNode = serviceListRoot;
  uint8_t status;

  // Initialize Add Service Request Struct
  lAddServReq.type = service->serviceType;
  memcpy(lAddServReq.UUID, service->serviceUUID.pUUID,
         service->serviceUUID.len);

#if defined SNP_LOCAL
  status = SNP_addService(&lAddServReq, service->serviceUUID.len, NULL);
#else
  // Add the service, Does not return until handle is received.
  // NULL Passed for response, not currently checking status
  status = SNP_RPC_addService(&lAddServReq, service->serviceUUID.len,
                              NULL);
#endif //SNP_LOCAL

  if (status != SNP_SUCCESS)
  {
    return status;
  }

  // Add the characteristics.
  uint8_t i;
  for (i = 0; i < service->charTableLen; i++)
  {
    snpAddCharValueDeclReq_t lValReq;
    snpAddCharValueDeclRsp_t lValRsp;
    SAP_Char_t ch = service->charTable[i];

    // Initialize Request
    lValReq.charValPerms = ch.permissions;
    lValReq.charValProps = ch.properties;
    lValReq.mgmtOption = SNP_CHAR_MANAGED_BY_AP;
    lValReq.charValMaxLen = SNP_GATT_CHAR_MAX_LENGTH;
    memcpy(lValReq.UUID, ch.UUID.pUUID, ch.UUID.len);

    // Add the characteristic value and save returned handle
#ifdef SNP_LOCAL
    status = SNP_addCharValueDecl(&lValReq, ch.UUID.len, &lValRsp);
#else
    status = SNP_RPC_addCharValueDec(&lValReq, ch.UUID.len, &lValRsp);
#endif //SNP_LOCAL

    if (status != SNP_SUCCESS)
    {
      return status;
    }

    service->charAttrHandles[i].valueHandle = lValRsp.attrHandle;

    // If there are any descriptors
    if (ch.pUserDesc || ch.pCccd || ch.pFormat || ch.pShortUUID || ch.pLongUUID)
    {
      uint8_t hIdx = 0;
      snpAddCharDescDeclReq_t lDescReq;
      snpAddCharDescDeclRsp_t lDescRsp;

      // Build Header
      lDescReq.header = (ch.pCccd)? SNP_DESC_HEADER_CCCD : 0;
      lDescReq.header |= (ch.pFormat)? SNP_DESC_HEADER_FORMAT : 0;
      lDescReq.header |= (ch.pUserDesc)? SNP_DESC_HEADER_USER_DESC : 0;
      lDescReq.header |= (ch.pShortUUID)? SNP_DESC_HEADER_GEN_SHORT_UUID: 0;
      lDescReq.header |= (ch.pLongUUID)? SNP_DESC_HEADER_GEN_LONG_UUID: 0;

      // Initialize pointers to sub-structs
      lDescReq.pCCCD = ch.pCccd;
      lDescReq.pFormat = ch.pFormat;
      lDescReq.pUserDesc = ch.pUserDesc;
      lDescReq.pShortUUID = ch.pShortUUID;
      lDescReq.pLongUUID = ch.pLongUUID;

#ifdef SNP_LOCAL
      status = SNP_addDescriptionValue(&lDescReq, &lDescRsp);
#else
      status = SNP_RPC_addCharDescDec(&lDescReq, &lDescRsp);
#endif // SNP_LOCAL

      if (status == SNP_SUCCESS)
      {
        // Copy handles from response into the service characteristic handle array
        service->charAttrHandles[i].sUUIDHandle =
          (ch.pShortUUID) ? lDescRsp.handles[hIdx++] : SNP_INVALID_HANDLE;
        service->charAttrHandles[i].lUUIDHandle =
          (ch.pLongUUID) ? lDescRsp.handles[hIdx++] : SNP_INVALID_HANDLE;
        service->charAttrHandles[i].cccdHandle =
          (ch.pCccd) ? lDescRsp.handles[hIdx++] : SNP_INVALID_HANDLE;
        service->charAttrHandles[i].formatHandle =
          (ch.pFormat) ? lDescRsp.handles[hIdx++] : SNP_INVALID_HANDLE;
        service->charAttrHandles[i].userDescHandle =
          (ch.pUserDesc) ? lDescRsp.handles[hIdx++] : SNP_INVALID_HANDLE;
      }
      else
      {
        // Return failure code
        return lDescRsp.status;
      }
    }
  }

  // Register the service.
#ifdef SNP_LOCAL
  status = SNP_registerService(&regServRsp);
#else
  status = SNP_RPC_registerService(&regServRsp);
#endif //SNP_LOCAL

  if (status != SNP_SUCCESS)
  {
    return status;
  }

  // Set the service handle
  service->serviceHandle = regServRsp.startHandle;

  // Create node
  newNode = (serviceNode_t *)SNP_malloc(sizeof(serviceNode_t));

  if (newNode != NULL)
  {
    // Tie service into service list
    if (serviceListRoot == NULL)
    {
      serviceListRoot = newNode;
    }
    else
    {
      while(currNode != NULL)
      {
        if(currNode->next == NULL)
        {
          // Last service in list
          currNode->next = (struct serviceNode_t *)newNode;
          break;
        }

        currNode = (serviceNode_t *)currNode->next;
      }
    }

    newNode->context = service->context;
    newNode->charReadCB = service->charReadCallback;
    newNode->charWriteCB = service->charWriteCallback;
    newNode->cccdIndCB = service->cccdIndCallback;
    newNode->next = NULL;

    // Determine min handle of this service and determine the max handle of the prev service
    newNode->minHandle = service->serviceHandle; //assumption min handle is service handle
    newNode->maxHandle = 0xFFFF;

    if (currNode != NULL)
    {
       currNode->maxHandle = service->serviceHandle - 1;
    }

  }

  return SNP_SUCCESS;
}

/**
 * @brief       Register a callback to receive a GAP event.  This shall be
 *              called once for each callback to be registered.
 *
 * @param       bleEventCallback - a Callback function to register: @ref SNP_RPC_GAP_EVENT_CB
 * @param       bleEventMask     - the mask of events which trigger this
 *                                 callback. Events types: @ref SNP_RPC_GAP_EVENTS
 *
 * @return      SNP_SUCCESS: The callback is registered.
 *              SNP_FAILURE: Callback registration failed.
 */
uint8_t SAP_registerEventCB(pfnEventCB_t eventCB, uint16_t eventMask)
{
  uint8_t status;

  // Register callback.
  status = SAP_addToEventCBList(eventCB, eventMask);

  return status;
}

/**
 * @fn          SAP_setServiceParam
 *
 * @brief       Write a characteristic value of a service.
 *
 * @param       serviceID - the UUID of the service
 * @param       charID    - the unique handle of the characteristic
 * @param       len       - length of the data to write
 * @param       pData     - pointer to buffer of data to write
 *
 * @return      SNP_SUCCESS: The write completed successfully.
 *              SNP_FAILURE: The write failed.
 */
uint8_t SAP_setServiceParam(uint8_t serviceID, uint8_t charID,
                            uint16_t len, uint8_t *pData)
{
  snpSetGattParamReq_t lReq;

  // Initialize Request
  lReq.serviceID = serviceID;
  lReq.paramID = charID;
  lReq.pData = pData;

  // Send a set parameter request to the SAP. Ignore response. It only
  // contains status which is also what the function returns
#ifdef SNP_LOCAL
  snpSetGattParamRsp_t lRsp;
  return SNP_setGATTParam(&lReq, len, &lRsp);
#else
  return SNP_RPC_setGATTParam(&lReq, len, NULL);
#endif //SNP_LOCAL
}

/**
 * @fn          SAP_getServiceParam
 *
 * @brief       Read a characteristic value of a service.
 *
 * @param       serviceID - the UUID of the service
 * @param       charID    - the unique handle of the characteristic
 * @param       len       - length of the data read
 * @param       pData     - pointer to buffer to write to
 *
 * @return      SNP_SUCCESS: the read completed successfully.
 *              SNP_FAILURE: The read failed.
 */
uint8_t SAP_getServiceParam(uint8_t serviceID, uint8_t charID,
                            uint16_t * len, uint8_t * pData)
{
  snpGetGattParamReq_t lReq;
  snpGetGattParamRsp_t lRsp;

  // Initialize Request
  lReq.serviceID = serviceID;
  lReq.paramID = charID;

  // Initialize Response data field. SNP call will copy into this buffer
  lRsp.pData = pData;

#ifdef SNP_LOCAL
  return SNP_getGATTParam(&lReq, &lRsp, len);
#else
  return SNP_RPC_getGATTParam(&lReq, &lRsp, len);
#endif //SNP_LOCAL
}

/**
 * @brief       Write to a stack parameter on the SAP. Some responses will
 *              Return immediately, others will generate an event for which
 *              a callback must be registered with the correct event mask.
 *
 * @param       subsystemID - the subsystem ID: @ref SNP_RPC_PARAM_SUBSYSTEMS
 * @param       paramID     - the parameter within the subsystem to write
 * @param       len         - length of the data to write
 * @param       pData       - pointer to buffer of data to write
 *
 * @return      SNP_SUCCESS: the write completed successfully.<BR>
 *              SNP_FAILURE: stack parameter write failed.<BR>
 */
uint8_t SAP_setParam(uint8_t subsystemID, uint16_t paramID, uint16_t len,
                     uint8_t *pData)
{
  uint8_t status = SNP_SUCCESS;

  // Determine the subsystem.
  switch(subsystemID)
  {
    // HCI command subsystem.
    case SAP_PARAM_HCI:
      {
        snpHciCmdReq_t lReq;

        // Initialize Request
        lReq.opcode = paramID;
        lReq.pData = pData;

#ifdef SNP_LOCAL
        SNP_executeHCIcmd(&lReq, len);
#else //!SNP_LOCAL
        status = SNP_RPC_sendHCICommand(&lReq, len);
#endif //SNP_LOCAL
      }
      break;

    // Advertising subsystem
    case SAP_PARAM_ADV:
      {
        // Determine parameter to write
        switch(paramID)
        {
          // These all send in the same command.
          case SAP_ADV_DATA_NOTCONN:
          case SAP_ADV_DATA_CONN:
          case SAP_ADV_DATA_SCANRSP:
            {
              if (len <= SAP_MAX_ADV_SCAN_DATA_LEN)
              {
              snpSetAdvDataReq_t lReq;

              // Initialize Request
              lReq.type = (paramID == SAP_ADV_DATA_NOTCONN) ? SNP_NONCONN_ADV_DATA :
                          (paramID == SAP_ADV_DATA_CONN) ? SNP_CONN_ADV_DATA :
                            SNP_SCANRSP_DATA ;
              lReq.pData = pData;

#ifdef SNP_LOCAL
              {
                snpSetAdvDataCnf_t cnf;

                status = SNP_setAdvData(&lReq, len);

                cnf.status = status;

                handleAsyncCB(SNP_SET_ADV_DATA_CNF, (snp_msg_t *)&cnf,
                              sizeof(snpSetAdvDataCnf_t));
              }
#else
              status = SNP_RPC_setAdvertisementData(&lReq , len);
#endif
              }
              else
              {
                status = SNP_FAILURE;
              }
            }
            break;
          case SAP_ADV_STATE:
            if (pData[0] == SAP_ADV_STATE_DISABLE)
            {
              // Stop advertising.
#ifdef SNP_LOCAL
              // Sending NULL is ok, parameter is not used.
              SNP_stopAdv();
#else
              SNP_RPC_stopAdvertising();
#endif //SNP_LOCAL
            }
            else
            {
              snpStartAdvReq_t lReq;

              // Check for application configuration
              if (pData != NULL && len == sizeof (snpStartAdvReq_t))
              {
                lReq = *(snpStartAdvReq_t *)pData;
              }
              else
              {
                // Initialize request with default configuration
                lReq.type = SNP_ADV_TYPE_CONN; // connectable advertising
                lReq.timeout = 0;              // never stops
                lReq.interval = 160;           // 160 * 625us = 100 ms interval between advertisement events
                lReq.behavior = SNP_ADV_RESTART_ON_CONN_TERM; // advertising stops upon connection and resumes after the connection terminates
              }

#ifdef SNP_LOCAL
              {
                snpEvt_t evt;
                uint8_t status;

                status = SNP_startAdv(&lReq);

                // Only send in failure case.
                if (status)
                {
                  evt.event = SNP_ADV_STARTED_EVT;
                  evt.pEvtParams = (snpEventParam_t *)&status;

                  getEventCallbacks(&evt);
                }
              }
#else //!SNP_LOCAL
              status = SNP_RPC_startAdvertising(&lReq);
#endif //SNP_LOCAL
            }
            break;
          default:
            // Unknown command.
            status = SNP_FAILURE;
            break;
        }
      }
      break;

    // Connection subsystem.
    case SAP_PARAM_CONN:
      {
        switch(paramID)
        {
          case SAP_CONN_PARAM:
            {
              snpUpdateConnParamReq_t *lReq;

              // Initialize Req
              lReq = (snpUpdateConnParamReq_t *)pData;

#ifdef SNP_LOCAL
              {
                snpUpdateConnParamCnf_t cnf;
                uint8_t status;

                status = SNP_updateConnParam(lReq);

                cnf.connHandle = lReq->connHandle;
                cnf.status = status;

                // send async
                handleAsyncCB(SNP_UPDATE_CONN_PARAM_CNF, (snp_msg_t *)&cnf,
                              sizeof(snpUpdateConnParamCnf_t));
              }
#else
              status = SNP_RPC_updateConnectionParams(lReq);
#endif //SNP_LOCAL
            }
            break;
          case SAP_CONN_STATE:
            {
              snpTermConnReq_t lReq;

              // Initialize Request
              memcpy(&lReq.connHandle,pData,len);
              lReq.option = 0;

              // For now, we are peripheral only, so this can only terminate a connection.
#ifdef SNP_LOCAL
              {
                uint8_t status;

                status = SNP_terminateConn(&lReq);

                // only send in failure case
                if (status == SNP_SUCCESS)
                {
                  snpEvt_t evt;
                  snpConnTermEvt_t term;

                  term.connHandle = 0xFFFF;
                  term.reason = SNP_INVALID_PARAMS;

                  evt.event = SNP_CONN_TERM_EVT;
                  evt.pEvtParams = (snpEventParam_t *)&term;

                  getEventCallbacks(&evt);
                }
              }
#else
              SNP_RPC_terminateConnection(&lReq);
#endif //SNP_LOCAL
            }
            break;
        }
      }
      break;

    case SAP_PARAM_SECURITY:
      {
        snpSetSecParamReq_t lReq;

        switch(paramID)
        {
          case SAP_SECURITY_IOCAPS:
            lReq.paramId = SNP_GAPBOND_IO_CAPABILITIES;
            break;

          case SAP_SECURITY_BEHAVIOR:
            lReq.paramId = SNP_GAPBOND_PAIRING_MODE;
            break;

          case SAP_SECURITY_BONDING:
            lReq.paramId = SNP_GAPBOND_BONDING_ENABLED;
            break;

          case SAP_ERASE_ALL_BONDS:
            lReq.paramId = SNP_GAPBOND_ERASE_ALLBONDS;
            break;

          case SAP_ERASE_LRU_BOND:
            lReq.paramId = SNP_GAPBOND_LRU_BOND_REPLACEMENT;
            break;

          default:
            // Unknown command
            status = SNP_FAILURE;
            break;
        }

        if (status != SNP_FAILURE)
        {
          if (pData)
          {
            lReq.value = *pData;
          }
          else
          {
            lReq.value = NULL;
          }

          // Send set parameter request
#ifdef SNP_LOCAL
          status = SNP_setSecurityParams(&lReq);
#else
          {
            snpSetSecParamRsp_t lRsp;

            SNP_RPC_setSecurityParam(&lReq, &lRsp);

            status = lRsp.status;
          }
#endif //SNP_LOCAL
        }
      }
      break;

    case SAP_PARAM_WHITELIST:
      {
        snpSetWhiteListReq_t lReq;

        lReq.useWhiteList = (*pData == SAP_WHITELIST_ENABLE) ?
                                SNP_FILTER_POLICY_WHITE : SNP_FILTER_POLICY_ALL;

#ifdef SNP_LOCAL
        status = SNP_setWhiteListFilterPolicy(&lReq);
#else
        status = SNP_RPC_setWhiteListParam(&lReq);
#endif //SNP_LOCAL
      }
      break;

    case SAP_PARAM_GAP:
      {
        if (len == sizeof(uint16_t))
        {
          snpSetGapParamReq_t req;
#ifndef SNP_LOCAL
          snpSetGapParamRsp_t rsp;
#endif //SNP_LOCAL

          req.paramId = paramID;
          req.value = (uint16_t)*pData;

#ifdef SNP_LOCAL
          status = SNP_setGapParam(&req);
#else
          SNP_RPC_setGAPparam(&req, &rsp);

          status = rsp.status;
#endif //SNP_LOCAL
        }
        else
        {
          status = SNP_INVALID_PARAMS;
        }
      }
      break;

    default:
      // Unknown command
      status = SNP_FAILURE;
      break;
  }

  return status;
}

/**
 * @brief       Read a stack parameter on the SAP. Some responses will
 *              Return immediately, others will generate an event for which
 *              a callback must be registered with the correct event mask.
 *
 * @param       subsystemID - the subsystem ID: @ref SNP_RPC_PARAM_SUBSYSTEMS
 * @param       paramID     - the parameter within the subsystem to read
 * @param       len         - length of the data to read
 * @param       pData       - pointer to buffer to write to
 *
 * @return      SNP_SUCCESS: the read completed successfully.<BR>
 *              SNP_RPC_FAILRUE: stack param read failed.<BR>
 */
uint8_t SAP_getParam(uint8_t subsystemID, uint8_t paramID, uint16_t len,
                     uint8_t *pData)
{
  uint8_t status = SNP_SUCCESS;

  // Determine the subsystem.
  switch(subsystemID)
  {
    // HCI command subsystem.
    case SAP_PARAM_HCI:
      {
        // Add HCI commands with accessible fields.
        snpHciCmdReq_t lReq;

        // Initialize Request
        lReq.opcode = paramID;
        lReq.pData = pData;

#ifdef SNP_LOCAL
        {
          status = SNP_executeHCIcmd(&lReq, len);

          // Only send in failure case.
          if (status)
          {
            snpHciCmdRsp_t hciRsp;

            hciRsp.status = status;
            hciRsp.opcode = paramID;
            hciRsp.pData = NULL;

            handleAsyncCB(SNP_HCI_CMD_RSP, (snp_msg_t *)&hciRsp,
                          sizeof(snpHciCmdRsp_t));
          }
        }
#else
        status = SNP_RPC_sendHCICommand(&lReq, len);
#endif //SNP_LOCAL
      }
      break;

    case SAP_PARAM_GAP:
      {
        snpGetGapParamReq_t req;
#ifndef SNP_LOCAL
        snpGetGapParamRsp_t rsp;
#endif //SNP_LOCAL

        req.paramId = paramID;

        if (len == sizeof(uint16_t))
        {
#ifdef SNP_LOCAL
          status = SNP_getGapParam(&req);

          *pData = req.value;
#else
          SNP_RPC_getGAPparam(&req, &rsp);

          status = rsp.status;
          *pData = rsp.value;
#endif //SNP_LOCAL
        }
        else
        {
          status = SNP_INVALID_PARAMS;
        }
      }
      break;

    default:
      // Unknown parameter
      status = SNP_FAILURE;
      break;
  }

  return status;
}

/**
 * @brief       Send a Security Request.  For a Peripheral device, this is a
 *              Slave Security Request not a Pairing Request.
 *
 * @param       None
 *
 * @return      SNP_SUCCESS: security was requested.<BR>
 *              SNP_RPC_FAILRUE: security request failed.<BR>
 */
uint8_t SAP_sendSecurityRequest(void)
{
#ifdef SNP_LOCAL
  uint8_t status;

  status = SNP_sendSecurityRequest();

  if (status)
  {
    snpEvt_t evt;
    snpSecurityEvt_t security;

    security.state = SNP_GAPBOND_PAIRING_STATE_COMPLETE;
    security.status = status;

    evt.event = SNP_SECURITY_EVT;
    evt.pEvtParams = (snpEventParam_t *)&security;

    getEventCallbacks(&evt);
  }

  return status;
#else
  return SNP_RPC_sendSecurityRequest();
#endif //SNP_LOCAL
}

/**
 * @brief       Send authentication data to SNP.
 *
 * @param       authData - authentication data.  This is the passkey used for
 *                         for the passkey entry protocol.
 *                         For numeric comparisons, this is TRUE if equivalent
 *                         or FALSE if not.
 *
 * @return      SNP_SUCCESS: authentication data sent.<BR>
 *              SNP_RPC_FAILRUE: authentication data failed to be sent.<BR>
 */
uint8_t SAP_setAuthenticationRsp(uint32_t authData)
{
  snpSetAuthDataReq_t pReq;

  pReq.authData = authData;

#ifdef SNP_LOCAL
  return SNP_setAuthenticationData(&pReq);
#else
  return SNP_RPC_setAuthenticationData(&pReq);
#endif //SNP_LOCAL
}

/**
 * @brief       Set event mask of SNP events
 *
 * @param       eventMask - mask of event flags for SNP to send. event types
 *                          not included in this mask will not be sent.
 *
 * @return      SNP_SUCCESS: mask set.<BR>
 */
uint8_t SAP_setSNPEventMask(uint16_t eventMask)
{
  snpMaskEventReq_t req;
  snpMaskEventRsp_t rsp;

  req.eventMask = eventMask;

#ifdef SNP_LOCAL
  // return parameter not used.
  SNP_maskEvt(&req, &rsp);
#else
  SNP_RPC_maskEvent(&req, &rsp);
#endif //SNP_LOCAL

  return SNP_SUCCESS;
}

/*********************************************************************
 * @fn      SAP_getRevision
 *
 * @brief   Get SNP Image revision and Stack revision numbers
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  None
 */
void SAP_getRevision(snpGetRevisionRsp_t *pRsp)
{
#ifdef SNP_LOCAL
  SNP_getRev(pRsp);
#else
  SNP_RPC_getRevision(pRsp);
#endif //SNP_LOCAL
}

/*********************************************************************
 * @fn      SAP_getRand
 *
 * @brief   Get a random number generated by the SNP TRNG
 *
 * @param   None
 *
 * @return  32 bit random number
 */
uint32_t SAP_getRand(void)
{
  snpGetRandRsp_t pRsp;

#ifdef SNP_LOCAL
  SNP_getRand(&pRsp);
#else
  SNP_RPC_getRand(&pRsp);
#endif //SNP_LOCAL

  return pRsp.rand;
}

/*********************************************************************
 * @fn      SAP_testCommand
 *
 * @brief   Get SNP memory statistics, returned as an asynchronous event.
 *
 * @return  None
 */
void SAP_testCommand(void)
{
#ifdef SNP_LOCAL
  snpTestCmdRsp_t pRsp;

  SNP_executeTestCmd(&pRsp);

  handleAsyncCB(SNP_TEST_RSP, (snp_msg_t *)&pRsp, sizeof(snpTestCmdRsp_t));
#else
  SNP_RPC_testCommand();
#endif //SNP_LOCAL
}

/*********************************************************************
 * @fn      SAP_getStatus
 *
 * @brief   Get SNP status
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  None
 */
void SAP_getStatus(snpGetStatusCmdRsp_t *pRsp)
{
#ifdef SNP_LOCAL
  SNP_getStatus(pRsp);
#else
  SNP_RPC_getStatus(pRsp);
#endif //SNP_LOCAL
}

/*********************************************************************
*********************************************************************/

