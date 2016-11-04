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
#include <string.h>
#include <stdint.h>
#include <ti/npi/hal_defs.h>

#include <ti/npi/npi_task.h>
#include <ti/npi/npi_data.h>
#include "snp_rpc_synchro.h"
#include "snp_rpc.h"

/*********************************************************************
*  EXTERNAL VARIABLES
*/

uint8_t SNP_NumConns = 0;

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * MACROS
 */

#ifdef SNP_LOCAL
#define MALLOC_FRAME(paramLen) SNP_mallocNPIFrame(paramLen)
#define SEND_MESSAGE(pReq)     SAP_SendMessage(pReq)
#define ENTER_CS()
#define EXIT_CS()
#else //!SNP_LOCAL
#define MALLOC_FRAME(paramLen) NPITask_mallocFrame(paramLen)
#define SEND_MESSAGE(pReq)     NPITask_sendToHost(pReq)
#define ENTER_CS()             SNP_enterCS()
#define EXIT_CS()              SNP_exitCS()
#endif //SNP_LOCAL



/*********************************************************************
 * GLOBAL VARIABLES
 */

// Callback function to the NP API to report events to application callbacks.
// This layer does not care about the callbacks and on passes the events back up.
SNP_RPC_eventCBRouter_t SNP_eventCB = NULL;

// Callback function to the NP API to report asynchronous events.
// Applications must register there own callbacks for this callback to pass them on.
SNP_RPC_asyncCB_t SNP_asyncCB = NULL;

/*********************************************************************
 * LOCAL VARIABLES
 */

// Structure to hold application allocated buffer for synchronous response data
// when it arrives.
snpSyncRspData_t npiRetMsg;

/*********************************************************************
 * HELPER FUNCTIONS
 */

#ifndef SNP_LOCAL
static void SNP_sendSynchronousCmd(_npiFrame_t *pReq, uint8_t opcode,
                                   snp_msg_t *pRsp, uint16_t *rspLen)
{
  // Enter Critical Section.
  ENTER_CS();

  npiRetMsg.pMsg = pRsp;

  // Send command.
  SEND_MESSAGE(pReq);

  // Wait for a response from the NP.
  SNP_waitForResponse();

  // Update Length after response is received
  if (rspLen)
  {
    *rspLen = npiRetMsg.len;
  }

  // Exit Critical Section
  EXIT_CS();
}
#endif //SNP_LOCAL

static void SNP_sendAsyncCmd(_npiFrame_t *pPkt)
{
  // Enter Critical Section.
  ENTER_CS();

  // Send command.
  SEND_MESSAGE(pPkt);

  // Exit Critical section
  EXIT_CS();
}

/*
 Allocate and build a basic NPI packet before adding command specific data.
 */
static _npiFrame_t * SNP_buildNPIPacket(uint8_t msgType, uint8_t cmd,
                                        uint16_t paramLen)
{
  _npiFrame_t *pPkt;

  // Allocated an empty data packet.
  if ((pPkt = MALLOC_FRAME(paramLen)))
  {
    // This is just the parameter length.
    pPkt->dataLen = paramLen;
    pPkt->cmd0 = msgType;
    pPkt->cmd1 = cmd;
  }

  // Return the packet pointer.
  return pPkt;
}

/*********************************************************************
 * @fn      SNP_RPC_registerSAPCBs
 *
 * @brief   Register SAP with SNP
 *
 * @param   asyncCB - pointer to NP asynchronous call back function
 * @param   eventCB - pointer to NP event call back function
 *
 * @return  none
 */
void SNP_RPC_registerSAPCBs(SNP_RPC_asyncCB_t asyncCB,
                            SNP_RPC_eventCBRouter_t eventCB)
{
  SNP_asyncCB = asyncCB;
  SNP_eventCB = eventCB;
}

/*********************************************************************
 * API FUNCTIONS
 */

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_maskEvent
 *
 * @brief   mask of events to to receive from the Network Processor
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_maskEvent(snpMaskEventReq_t *pReq,
                          snpMaskEventRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE , SNP_MASK_EVT_REQ,
                            sizeof(snpMaskEventReq_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Initialize Request packet
    pPkt->pData[0] = LO_UINT16(pReq->eventMask);
    pPkt->pData[1] = HI_UINT16(pReq->eventMask);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_MASK_EVT_REQ, (snp_msg_t *)pRsp,NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_getRevision
 *
 * @brief   Get SNP Image revision and Stack revision numbers
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_getRevision(snpGetRevisionRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_GET_REVISION_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_GET_REVISION_REQ, (snp_msg_t *)pRsp, NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_getRand
 *
 * @brief   Get random number generated using TRNG on SNP
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_getRand(snpGetRandRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_GET_RAND_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_GET_RAND_REQ, (snp_msg_t *)pRsp, NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_sendHCICommand
 *
 * @brief   Send an HCI command over SNP message
 *
 * @param   pReq - pointer to SNP request message
 * @param   paramLen - length of data field of SNP request
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_sendHCICommand(snpHciCmdReq_t *pReq, uint8_t paramLen)
{
  _npiFrame_t *pPkt;
  uint16_t reqSize = sizeof(snpHciCmdReq_t) - sizeof(pReq->pData);
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_HCI_CMD_REQ,
                            paramLen + reqSize);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = LO_UINT16(pReq->opcode);
    pPkt->pData[1] = HI_UINT16(pReq->opcode);

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize], pReq->pData, paramLen);

    // Send param
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_getStatus
 *
 * @brief   Get SNP status
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_getStatus(snpGetStatusCmdRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_GET_STATUS_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_GET_STATUS_REQ, (snp_msg_t *)pRsp, NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_testCommand
 *
 * @brief   Get SNP memory statistics
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_testCommand(void)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_TEST_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send param
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_startAdvertising
 *
 * @brief   Start advertising.  Advertising data must be set with
 *          SNP_updateAdvData first. Command sets the following:
 *
 * - Advertising type:  connectable, non-connectable discoverable
 * or non-connectable non-discoverable (broadcasting)
 * - Timeout: advertising will cease after the timeout (in ms) expires.
 * - Interval: the delta (in ms) between two consecutive advertisements.
 * - Behavior after a connection occurs: if FALSE stop advertising after
 * connection.  Else continue advertising.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_SUCCESS, SNP_ADV_DATA_NOT_READY,
 *                    or SNP_ADV_NOT_AVAILABLE
 */
uint8_t SNP_RPC_startAdvertising(snpStartAdvReq_t *pReq)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_START_ADV_REQ,
                            sizeof(snpStartAdvReq_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy members of request struct into packet
    pPkt->pData[0] = pReq->type;
    pPkt->pData[1] = LO_UINT16(pReq->timeout);
    pPkt->pData[2] = HI_UINT16(pReq->timeout);
    pPkt->pData[3] = LO_UINT16(pReq->interval);
    pPkt->pData[4] = HI_UINT16(pReq->interval);
    pPkt->pData[5] = pReq->filterPolicy;
    pPkt->pData[6] = pReq->initiatorAddrType;
    memcpy(&pPkt->pData[7], pReq->initiatorAddress,
           sizeof(pReq->initiatorAddress));
    pPkt->pData[13] = pReq->behavior;

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_stopAdvertising
 *
 * @brief   Stop advertising
 *
 * @param   none
 *
 * @return  uint8_t - SNP_SUCCESS, SNP_NOT_ADVERTISING
 */
uint8_t SNP_RPC_stopAdvertising(void)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Prepare Command
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_STOP_ADV_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send command0
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_setAdvertisementData
 *
 * @brief   Configure Advertising or Scan Response Data.
 *
 * @param   pReq - pointer to SNP request message
 * @param   dataLen - length of data field of SNP request
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_setAdvertisementData(snpSetAdvDataReq_t *pReq, uint8_t datalen)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t reqSize = sizeof(snpSetAdvDataReq_t) - sizeof(pReq->pData);

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_SET_ADV_DATA_REQ,
                            reqSize + datalen);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = pReq->type;

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize], pReq->pData, datalen);

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_terminateConnection
 *
 * @brief   Terminate the existing connection.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_terminateConnection(snpTermConnReq_t *pReq)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_TERMINATE_CONN_REQ,
                            sizeof(snpTermConnReq_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = LO_UINT16(pReq->connHandle);
    pPkt->pData[1] = HI_UINT16(pReq->connHandle);
    pPkt->pData[2] = pReq->option;

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_updateConnectionParams
 *
 * @brief   Update the parameters of an existing connection.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_updateConnectionParams(snpUpdateConnParamReq_t *pReq)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_UPDATE_CONN_PARAM_REQ,
                            sizeof(snpUpdateConnParamReq_t));


  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = LO_UINT16(pReq->connHandle);
    pPkt->pData[1] = HI_UINT16(pReq->connHandle);
    pPkt->pData[2] = LO_UINT16(pReq->intervalMin);
    pPkt->pData[3] = HI_UINT16(pReq->intervalMin);
    pPkt->pData[4] = LO_UINT16(pReq->intervalMax);
    pPkt->pData[5] = HI_UINT16(pReq->intervalMax);
    pPkt->pData[6] = LO_UINT16(pReq->slaveLatency);
    pPkt->pData[7] = HI_UINT16(pReq->slaveLatency);
    pPkt->pData[8] = LO_UINT16(pReq->supervisionTimeout);
    pPkt->pData[9] = HI_UINT16(pReq->supervisionTimeout);

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_setGAPparam
 *
 * @brief   Set GAP parameter on SNP
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_setGAPparam(snpSetGapParamReq_t *pReq,
                            snpSetGapParamRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_SET_GAP_PARAM_REQ,
                            sizeof(snpSetGapParamReq_t));


  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = LO_UINT16(pReq->paramId);
    pPkt->pData[1] = HI_UINT16(pReq->paramId);
    pPkt->pData[2] = LO_UINT16(pReq->value);
    pPkt->pData[3] = HI_UINT16(pReq->value);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_SET_GAP_PARAM_REQ, (snp_msg_t *)pRsp,NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_getGAPparam
 *
 * @brief   Get GAP parameter from SNP
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_getGAPparam(snpGetGapParamReq_t *pReq,
                            snpGetGapParamRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_GET_GAP_PARAM_REQ,
                            sizeof(snpSetGapParamReq_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = LO_UINT16(pReq->paramId);
    pPkt->pData[1] = HI_UINT16(pReq->paramId);
    pPkt->pData[2] = LO_UINT16(pReq->value);
    pPkt->pData[3] = HI_UINT16(pReq->value);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_ADD_SERVICE_REQ, (snp_msg_t *)pRsp,NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_setSecurityParam
 *
 * @brief   Set the Security Requirements for SNP.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_setSecurityParam(snpSetSecParamReq_t *pReq,
                                 snpSetSecParamRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_SET_SECURITY_PARAM_REQ,
                            sizeof(snpSetSecParamReq_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    pPkt->pData[0] = pReq->paramId;
    pPkt->pData[1] = pReq->value;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_SET_SECURITY_PARAM_REQ, (snp_msg_t *)pRsp,
                           NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_sendSecurityRequest
 *
 * @brief   Set the Security Requirements for SNP.
 *
 * @param   none
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_sendSecurityRequest(void)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE,
                            SNP_SEND_SECURITY_REQUEST_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send Command.
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_stAuthenticationData
 *
 * @brief   Set the Authentication data for a pairing procedure.
 *
 * @param   pReq - pointer to SPN request message
 *
 * @return  uint8_t - status of this command
 */
uint8_t SNP_RPC_setAuthenticationData(snpSetAuthDataReq_t *pReq)
{
  snpSetAuthDataRsp_t rsp;
  _npiFrame_t *pPkt;
  uint8_t *ptr;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE,
                            SNP_SET_AUTHENTICATION_DATA_REQ, 4);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    ptr = pPkt->pData;
    UINT32_TO_BUF_LITTLE_ENDIAN(ptr, pReq->authData);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_SET_AUTHENTICATION_DATA_REQ,
                           (snp_msg_t *)&rsp, NULL);

    status = rsp.status;
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_setWhiteListParam
 *
 * @brief   Set the White List policy.
 *
 * @param   pReq - pointer to SPN request message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_setWhiteListParam(snpSetWhiteListReq_t *pReq)
{
  snpSetWhiteListRsp_t rsp;
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE,
                            SNP_SET_WHITE_LIST_POLICY_REQ, 1);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    pPkt->pData[0] = pReq->useWhiteList;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_SET_WHITE_LIST_POLICY_REQ, (snp_msg_t *)&rsp,
                           NULL);

    status = rsp.status;
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_addService
 *
 * @brief   Add a service to the GATT server on the SNP.
 *
 * @param   pReq - pointer to SNP request message
 * @param   uuidLen - Length of UUID field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_addService(snpAddServiceReq_t *pReq, uint8_t uuidLen,
                           snpAddServiceRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t reqSize = sizeof(snpAddServiceReq_t) - sizeof(pReq->UUID);

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_ADD_SERVICE_REQ,
                            reqSize + uuidLen);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = pReq->type;

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize],pReq->UUID, uuidLen);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_ADD_SERVICE_REQ, (snp_msg_t *)pRsp,NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_addCharValueDec
 *
 * @brief   Add a characteristic value to a service table. Used after
 *          calling SNP_addService to include characteristics within the
 *          service.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pReq - Length of UUID field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_addCharValueDec(snpAddCharValueDeclReq_t *pReq, uint8_t uuidLen,
                                snpAddCharValueDeclRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t reqSize = sizeof(snpAddCharValueDeclReq_t) - sizeof(pReq->UUID);

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_ADD_CHAR_VAL_DECL_REQ,
                            reqSize + uuidLen);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = pReq->charValPerms;
    pPkt->pData[1] = LO_UINT16(pReq->charValProps);
    pPkt->pData[2] = HI_UINT16(pReq->charValProps);
    pPkt->pData[3] = pReq->mgmtOption;
    pPkt->pData[4] = LO_UINT16(pReq->charValMaxLen);
    pPkt->pData[5] = HI_UINT16(pReq->charValMaxLen);

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize], pReq->UUID, uuidLen);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_ADD_CHAR_VAL_DECL_REQ, (snp_msg_t *)pRsp,
                           NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_addCharDescDec
 *
 * @brief   Add a characteristic user description, CCCD, format attribute
 *          to a characteristic on SNP.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pReq - Length of UUID field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_addCharDescDec(snpAddCharDescDeclReq_t *pReq,
                               snpAddCharDescDeclRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t pktLen = 0;
  uint16_t pktIdx = 0;

  // Verify only sending supported Attributes
  if (pReq->header & SNP_DESC_HEADER_UNSUPPORTED_MASK)
  {
    return SNP_FAILURE;
  }

  // Determine Size of Packet
  pktLen += sizeof(pReq->header);
  pktLen += (pReq->header & SNP_DESC_HEADER_GEN_SHORT_UUID) ?
            sizeof(snpAddAttrGenShortUUID_t) : 0;
  pktLen += (pReq->header & SNP_DESC_HEADER_GEN_LONG_UUID) ?
            sizeof(snpAddAttrGenLongUUID_t) : 0;
  pktLen += (pReq->header & SNP_DESC_HEADER_CCCD) ?
            sizeof(snpAddAttrCccd_t) : 0;
  pktLen += (pReq->header & SNP_DESC_HEADER_FORMAT) ?
            sizeof(snpAddAttrFormat_t) : 0;
  pktLen += (pReq->header & SNP_DESC_HEADER_USER_DESC) ?
            sizeof(snpAddAttrUserDesc_t) - sizeof(pReq->pUserDesc->pDesc) +
            pReq->pUserDesc->initLen : 0;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_ADD_CHAR_DESC_DECL_REQ,
                            pktLen);

  if ( pPkt )
  {
    status = SNP_SUCCESS;
    // Copy statically sized members of struct
    pPkt->pData[pktIdx++] = pReq->header;

    // Short UUID
    if (pReq->header & SNP_DESC_HEADER_GEN_SHORT_UUID)
    {
      pPkt->pData[pktIdx++] = pReq->pShortUUID->perms;
      pPkt->pData[pktIdx++] = LO_UINT16(pReq->pShortUUID->maxLen);
      pPkt->pData[pktIdx++] = HI_UINT16(pReq->pShortUUID->maxLen);
      pPkt->pData[pktIdx++] = pReq->pShortUUID->UUID[0];
      pPkt->pData[pktIdx++] = pReq->pShortUUID->UUID[1];
    }

    // Long UUID
    if (pReq->header & SNP_DESC_HEADER_GEN_LONG_UUID)
    {
      pPkt->pData[pktIdx++] = pReq->pLongUUID->perms;
      pPkt->pData[pktIdx++] = LO_UINT16(pReq->pLongUUID->maxLen);
      pPkt->pData[pktIdx++] = HI_UINT16(pReq->pLongUUID->maxLen);

      memcpy(&pPkt->pData[pktIdx],pReq->pLongUUID->UUID,
             sizeof(pReq->pLongUUID->UUID));
      pktIdx += sizeof(pReq->pLongUUID->UUID);
    }

    // CCCD
    if (pReq->header & SNP_DESC_HEADER_CCCD)
    {
      // Copy statically sized members of struct
      pPkt->pData[pktIdx++] = pReq->pCCCD->perms;
    }

    // Format
    if (pReq->header & SNP_DESC_HEADER_FORMAT)
    {
      // Copy statically sized members of struct
      pPkt->pData[pktIdx++] = pReq->pFormat->format;
      pPkt->pData[pktIdx++] = pReq->pFormat->exponent;
      pPkt->pData[pktIdx++] = LO_UINT16(pReq->pFormat->unit);
      pPkt->pData[pktIdx++] = HI_UINT16(pReq->pFormat->unit);
      pPkt->pData[pktIdx++] = pReq->pFormat->name_space;
      pPkt->pData[pktIdx++] = LO_UINT16(pReq->pFormat->desc);
      pPkt->pData[pktIdx++] = HI_UINT16(pReq->pFormat->desc);
    }

    // User Desc
    if (pReq->header & SNP_DESC_HEADER_USER_DESC)
    {
      // Copy statically sized members of struct
      pPkt->pData[pktIdx++] = pReq->pUserDesc->perms;
      pPkt->pData[pktIdx++] = LO_UINT16(pReq->pUserDesc->maxLen);
      pPkt->pData[pktIdx++] = HI_UINT16(pReq->pUserDesc->maxLen);
      pPkt->pData[pktIdx++] = LO_UINT16(pReq->pUserDesc->initLen);
      pPkt->pData[pktIdx++] = HI_UINT16(pReq->pUserDesc->initLen);

      // Copy dynamic sized members of struct
      memcpy(&pPkt->pData[pktIdx],pReq->pUserDesc->pDesc,
             pReq->pUserDesc->initLen);
      pktIdx += pReq->pUserDesc->initLen;
    }

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_ADD_CHAR_DESC_DECL_REQ, (snp_msg_t *)pRsp,
                           NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_registerService
 *
 * @brief   Registers the service within the GATT Server.  This is called
 *          after a service is added with SNP_addService and all service
 *          characteristics for that service are added with
 *          SNP_addCharacteristic.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_registerService(snpRegisterServiceRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_REGISTER_SERVICE_REQ, 0);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_REGISTER_SERVICE_REQ, (snp_msg_t *)pRsp,
                           NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

/*********************************************************************
 * @fn      SNP_RPC_readCharCnf
 *
 * @brief   Respond to a characteristic read request from a GATT Client.
 *
 * @param   pCnf - pointer to SNP confirmation message
 * @param   size - length of the confirmation data payload
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_readCharCnf(snpCharReadCnf_t *pCnf, uint16_t size)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t reqSize = sizeof(snpCharReadCnf_t) - sizeof(pCnf->pData);

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_CHAR_READ_CNF,
                            reqSize + size);

  if ( pPkt )
  {
    status = SNP_SUCCESS;
    // Copy non-pointer members of request struct
    pPkt->pData[0] = pCnf->status;
    pPkt->pData[1] = LO_UINT16(pCnf->connHandle);
    pPkt->pData[2] = HI_UINT16(pCnf->connHandle);
    pPkt->pData[3] = LO_UINT16(pCnf->attrHandle);
    pPkt->pData[4] = HI_UINT16(pCnf->attrHandle);
    pPkt->pData[5] = LO_UINT16(pCnf->offset);
    pPkt->pData[6] = HI_UINT16(pCnf->offset);

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize], pCnf->pData, size);

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}

/*********************************************************************
 * @fn      SNP_RPC_writeCharCnf
 *
 * @brief   Respond to a characteristic write request from GATT Client
 *
 * @param   pCnf - pointer to SNP confirmation message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_writeCharCnf(snpCharWriteCnf_t *pCnf)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_CHAR_WRITE_CNF,
                            sizeof(snpCharWriteCnf_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;
    // Copy non-pointer members of request struct
    pPkt->pData[0] = pCnf->status;
    pPkt->pData[1] = LO_UINT16(pCnf->connHandle);
    pPkt->pData[2] = HI_UINT16(pCnf->connHandle);

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}

/*********************************************************************
 * @fn      SNP_RPC_sendNotifInd
 *
 * @brief   Send a notification for a characteristic value to GATT Client
 *
 * @param   pReq - pointer to SNP request message
 * @param   dataLen - length of the notification data payload
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_sendNotifInd(snpNotifIndReq_t *pReq, uint8_t dataLen)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t reqSize = sizeof(snpNotifIndReq_t) - sizeof(pReq->pData);

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_SEND_NOTIF_IND_REQ,
                            reqSize + dataLen);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = LO_UINT16(pReq->connHandle);
    pPkt->pData[1] = HI_UINT16(pReq->connHandle);
    pPkt->pData[2] = LO_UINT16(pReq->attrHandle);
    pPkt->pData[3] = HI_UINT16(pReq->attrHandle);
    pPkt->pData[4] = pReq->authenticate;
    pPkt->pData[5] = pReq->type;

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize], pReq->pData, dataLen);

    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}

/*********************************************************************
 * @fn      SNP_RPC_charConfigUpdatedRsp
 *
 * @brief   A GATT Client has requested to updated the CCCD value. the AP
 *          has a choice of validating the request or rejecting.
 *
 * @param   pReq - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_charConfigUpdatedRsp(snpCharCfgUpdatedRsp_t *pReq)
{
  // Prepare Command
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_ASYNC_CMD_TYPE, SNP_CCCD_UPDATED_CNF,
                            sizeof(snpCharCfgUpdatedRsp_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = pReq->status;
    pPkt->pData[1] = LO_UINT16(pReq->connHandle);
    pPkt->pData[2] = HI_UINT16(pReq->connHandle);

    // Send Command
    SNP_sendAsyncCmd(pPkt);
  }

  // Return Status
  return status;
}

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_setGATTParam
 *
 * @brief   Overwrite characteristic values of the Device Info Service.
 *
 * @param   pReq - pointer to SNP request message
 * @param   dataLen - Length of data field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_setGATTParam(snpSetGattParamReq_t *pReq, uint16_t dataLen,
                             snpSetGattParamRsp_t *pRsp)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;
  uint16_t reqSize = sizeof(snpSetGattParamReq_t) - sizeof(pReq->pData);

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_SET_GATT_PARAM_REQ,
                            reqSize + dataLen);

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = pReq->serviceID;
    pPkt->pData[1] = pReq->paramID;

    // Copy pointer members
    memcpy(&pPkt->pData[reqSize], pReq->pData, dataLen);

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_SET_GATT_PARAM_REQ, (snp_msg_t *)pRsp, NULL);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL

#ifndef SNP_LOCAL
/*********************************************************************
 * @fn      SNP_RPC_getGATTParam
 *
 * @brief   Get characteristic values of the Device Info Service.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 * @param   dataLen - Length of data field in response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
uint8_t SNP_RPC_getGATTParam(snpGetGattParamReq_t *pReq,
                             snpGetGattParamRsp_t *pRsp, uint16_t *dataLen)
{
  _npiFrame_t *pPkt;
  uint8_t status = SNP_OUT_OF_RESOURCES;

  // Allocated an empty data NPI packet.
  pPkt = SNP_buildNPIPacket(SNP_NPI_SYNC_REQ_TYPE, SNP_GET_GATT_PARAM_REQ,
                            sizeof(snpGetGattParamReq_t));

  if ( pPkt )
  {
    status = SNP_SUCCESS;

    // Copy non-pointer members of request struct
    pPkt->pData[0] = pReq->serviceID;
    pPkt->pData[1] = pReq->paramID;

    // Send a synchronous command.
    SNP_sendSynchronousCmd(pPkt, SNP_GET_GATT_PARAM_REQ, (snp_msg_t *)&pRsp,
                           dataLen);
  }

  // Return Status
  return status;
}
#endif //SNP_LOCAL


/*********************************************************************
*********************************************************************/
