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

// ****************************************************************************
// includes
// ****************************************************************************

#include <string.h>

#include "snp.h"
#include "snp_rpc.h"
#include "snp_rpc_synchro.h"

#include <ti/npi/npi_data.h>
#include <ti/npi/npi_task.h>
#include "npi_ss_ble_sap.h"

#include <ti/npi/hal_defs.h>

// ****************************************************************************
// defines
// ****************************************************************************

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// externs
//*****************************************************************************

// Defined in snp_rpc.c
extern SNP_RPC_eventCBRouter_t SNP_eventCB;
extern SNP_RPC_asyncCB_t SNP_asyncCB;

extern snpSyncRspData_t npiRetMsg;

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      NPI BLE Subsystem initialization function
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPISS_BLE_SNP_init()
{
    // Register for messages from Host with RPC_SYS_BLE ssID
    NPITask_regSSFromHostCB(RPC_SYS_BLE_SNP,NPISS_BLE_SNP_msgFromSNP);
}

// -----------------------------------------------------------------------------
//! \brief      Call back function provided to NPI Task. All incoming NPI
//!             received by NPI Task with the subsystem ID of this subsystem
//!             will be sent to this call back through the NPI routing system
//!
//!             *** This function MUST free pNPIMsg
//!
//! \param[in]  pNPIMsg    Pointer to a "framed" NPI message
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPISS_BLE_SNP_msgFromSNP(_npiFrame_t *pNPIMsg)
{
  uint16_t msgLen = pNPIMsg->dataLen;

  switch(SNP_GET_OPCODE_HDR_CMD1(pNPIMsg->cmd1))
  {
    /* Device group */
    case SNP_DEVICE_GRP:
      {
        switch( pNPIMsg->cmd1 )
        {
#ifndef SNP_LOCAL
          // NP is powered up indication.
          case SNP_POWER_UP_IND:
            // Device has restarted.
            if (SNP_asyncCB)
            {
              SNP_asyncCB(pNPIMsg->cmd1, NULL, msgLen);
            }
            break;


          // Mask Events response.
          case SNP_MASK_EVENT_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->maskEventCnf.maskedEvent =
                BUILD_UINT16(pNPIMsg->pData[0],pNPIMsg->pData[1]);
            }
            break;

          case SNP_GET_REVISION_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->revisionRsp.status = pNPIMsg->pData[0];
              npiRetMsg.pMsg->revisionRsp.snpVer =
                      BUILD_UINT16(pNPIMsg->pData[1],pNPIMsg->pData[2]);
              memcpy(npiRetMsg.pMsg->revisionRsp.stackBuildVer,
                     &pNPIMsg->pData[3],
                     sizeof(npiRetMsg.pMsg->revisionRsp.stackBuildVer));
            }
            break;

          case SNP_GET_RAND_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->randRsp.rand = BUILD_UINT32(pNPIMsg->pData[0],
                                                          pNPIMsg->pData[1],
                                                          pNPIMsg->pData[2],
                                                          pNPIMsg->pData[3]);

            }
            break;

          // HCI command response
          case SNP_HCI_CMD_RSP:
            {
              snpHciCmdRsp_t hciRsp;

              // Initialize Response Struct
              hciRsp.status = pNPIMsg->pData[0];
              hciRsp.opcode = BUILD_UINT16(pNPIMsg->pData[1],pNPIMsg->pData[2]);
              hciRsp.pData = (uint8_t *)&pNPIMsg->pData[3];

              if (SNP_asyncCB)
              {
                SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&hciRsp, msgLen);
              }
            }
            break;
#endif //SNP_LOCAL

          case SNP_EVENT_IND:
            {
              snpEvt_t pEvt;

              // Copy non-Pointer members of Event Struct
              pEvt.event = BUILD_UINT16(pNPIMsg->pData[0],pNPIMsg->pData[1]);

              // Send event back up to NP.
              switch(pEvt.event)
              {
                case SNP_CONN_EST_EVT:
                  {
                    snpConnEstEvt_t connEstEvt;

                    // Initialize Event
                    connEstEvt.connHandle =
                      BUILD_UINT16(pNPIMsg->pData[2],pNPIMsg->pData[3]);
                    connEstEvt.connInterval =
                      BUILD_UINT16(pNPIMsg->pData[4],pNPIMsg->pData[5]);
                    connEstEvt.slaveLatency =
                      BUILD_UINT16(pNPIMsg->pData[6],pNPIMsg->pData[7]);
                    connEstEvt.supervisionTimeout =
                      BUILD_UINT16(pNPIMsg->pData[8],pNPIMsg->pData[9]);
                    connEstEvt.addressType = pNPIMsg->pData[10];

                    memcpy(connEstEvt.pAddr, &pNPIMsg->pData[11],
                           sizeof(connEstEvt.pAddr));

                    pEvt.pEvtParams = (snpEventParam_t *)&connEstEvt;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;

                case SNP_CONN_TERM_EVT:
                  {
                    snpConnTermEvt_t connTermEvt;

                    // Initialize Event
                    connTermEvt.connHandle =
                      BUILD_UINT16(pNPIMsg->pData[2],pNPIMsg->pData[3]);
                    connTermEvt.reason = pNPIMsg->pData[4];

                    pEvt.pEvtParams = (snpEventParam_t *)&connTermEvt;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;

                case SNP_CONN_PARAM_UPDATED_EVT:
                  {
                    snpUpdateConnParamEvt_t event;

                    // Initialize Event
                    event.connHandle =
                      BUILD_UINT16(pNPIMsg->pData[2],pNPIMsg->pData[3]);
                    event.connInterval =
                      BUILD_UINT16(pNPIMsg->pData[4],pNPIMsg->pData[5]);
                    event.slaveLatency =
                      BUILD_UINT16(pNPIMsg->pData[6],pNPIMsg->pData[7]);
                    event.supervisionTimeout =
                      BUILD_UINT16(pNPIMsg->pData[8],pNPIMsg->pData[9]);

                    pEvt.pEvtParams = (snpEventParam_t *)&event;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;

                case SNP_ADV_STARTED_EVT:
                case SNP_ADV_ENDED_EVT:
                  {
                    snpAdvStatusEvt_t event;

                    // Initialize Event
                    event.status = pNPIMsg->pData[2];

                    pEvt.pEvtParams = (snpEventParam_t *)&event;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;

                case SNP_ATT_MTU_EVT:
                  {
                    snpATTMTUSizeEvt_t event;

                    // Initialize Event
                    event.connHandle =
                      BUILD_UINT16(pNPIMsg->pData[2],pNPIMsg->pData[3]);
                    event.attMtuSize =
                      BUILD_UINT16(pNPIMsg->pData[4],pNPIMsg->pData[5]);

                    pEvt.pEvtParams = (snpEventParam_t *)&event;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;

                case SNP_SECURITY_EVT:
                  {
                    snpSecurityEvt_t event;

                    // Initialize Event
                    event.state = pNPIMsg->pData[2];
                    event.status = pNPIMsg->pData[3];

                    pEvt.pEvtParams = (snpEventParam_t *)&event;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;

                case SNP_AUTHENTICATION_EVT:
                  {
                    snpAuthenticationEvt_t event;

                    // Initialize Event
                    event.display = pNPIMsg->pData[2];
                    event.input   = pNPIMsg->pData[3];
                    event.numCmp = BUILD_UINT32(pNPIMsg->pData[4],
                                                pNPIMsg->pData[5],
                                                pNPIMsg->pData[6],
                                                pNPIMsg->pData[7]);

                    pEvt.pEvtParams = (snpEventParam_t *)&event;

                    // Send to NP layer.
                    if ( SNP_eventCB )
                    {
                      SNP_eventCB(&pEvt);
                    }
                  }
                  break;
              }
            }
            break;

          case SNP_GET_STATUS_RSP:
          if ( npiRetMsg.pMsg )
          {
            npiRetMsg.len = msgLen;

            // Explicitly copy response
            npiRetMsg.pMsg->getStatusRsp.gapRoleStatus = pNPIMsg->pData[0];
            npiRetMsg.pMsg->getStatusRsp.advStatus = pNPIMsg->pData[1];
            npiRetMsg.pMsg->getStatusRsp.ATTstatus = pNPIMsg->pData[2];
            npiRetMsg.pMsg->getStatusRsp.ATTmethod = pNPIMsg->pData[3];
          }
          break;

#ifndef SNP_LOCAL
        case SNP_TEST_RSP:
          {
            snpTestCmdRsp_t rsp;

            // Initialize Response Struct
            rsp.memAlo = BUILD_UINT16(pNPIMsg->pData[0],pNPIMsg->pData[1]);
            rsp.memMax = BUILD_UINT16(pNPIMsg->pData[2],pNPIMsg->pData[3]);
            rsp.memSize = BUILD_UINT16(pNPIMsg->pData[4],pNPIMsg->pData[5]);

            if (SNP_asyncCB)
            {
              SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&rsp, msgLen);
            }
          }
          break;
#endif //SNP_LOCAL

          default:
            // Unknown command
            break;
        }
      }
      break;

    /* GAP group */
    case SNP_GAP_GRP:
      {
        switch(pNPIMsg->cmd1)
        {
#ifndef SNP_LOCAL
          // GAP Role initialized.
          case SNP_INIT_DEVICE_CNF:
            // RFU
            break;

          // Advertisement data confirmation
          case SNP_SET_ADV_DATA_CNF:
            {
              snpSetAdvDataCnf_t cnf;

              cnf.status = pNPIMsg->pData[0];

              if (SNP_asyncCB)
              {
                SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&cnf, msgLen);
              }
            }

          case SNP_UPDATE_CONN_PARAM_CNF:
            {
              snpUpdateConnParamCnf_t cnf;

              cnf.status = pNPIMsg->pData[0];
              cnf.connHandle =
                BUILD_UINT16(pNPIMsg->pData[1], pNPIMsg->pData[2]);

              if (SNP_asyncCB)
              {
                SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&cnf, msgLen);
              }
            }
            break;
#endif //SNP_LOCAL

          // Set GAP parameter response.
          case SNP_SET_GAP_PARAM_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->setGapParamRsp.status = pNPIMsg->pData[0];
            }
            break;

          // Get GAP parameter response.
          case SNP_GET_GAP_PARAM_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->getGapParamRsp.status = pNPIMsg->pData[0];
              npiRetMsg.pMsg->getGapParamRsp.paramId =
                BUILD_UINT16(pNPIMsg->pData[1],pNPIMsg->pData[2]);
              npiRetMsg.pMsg->getGapParamRsp.status =
                BUILD_UINT16(pNPIMsg->pData[3],pNPIMsg->pData[4]);
            }
            break;

#ifdef SNP_LOCAL
          case SNP_SET_SECURITY_PARAM_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->setSecParamRsp.status = pNPIMsg->pData[0];
            }
            break;

          case SNP_SEND_AUTHENTICATION_DATA_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->setAuthDataRsp.status = pNPIMsg->pData[0];
            }
            break;

          case SNP_SET_WHITE_LIST_POLICY_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              //Explicitly copy reponse
              npiRetMsg.pMsg->setWhiteListRsp.status = pNPIMsg->pData[0];
            }
            break;
#endif //!SNP_LOCAL
          default:
            // Unknown command
            break;
        }
      }
      break;

    /* GATT group */
    case SNP_GATT_GRP:
      {
        switch(pNPIMsg->cmd1)
        {
#ifndef SNP_LOCAL
          case SNP_ADD_SERVICE_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->addServiceRsp.status = pNPIMsg->pData[0];
            }
            break;

          case SNP_ADD_CHAR_VAL_DECL_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->addCharValueDecRsp.status = pNPIMsg->pData[0];
              npiRetMsg.pMsg->addCharValueDecRsp.attrHandle =
                BUILD_UINT16(pNPIMsg->pData[1],pNPIMsg->pData[2]);
            }
            break;

          case SNP_ADD_CHAR_DESC_DECL_RSP:
            if ( npiRetMsg.pMsg )
            {
              uint8_t i = 0;
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->addCharDescDeclRsp.status = pNPIMsg->pData[0];
              npiRetMsg.pMsg->addCharDescDeclRsp.header = pNPIMsg->pData[1];

              // Remaining Msg contents are uint16 handles
              while(i < (msgLen - 2))
              {
                npiRetMsg.pMsg->addCharDescDeclRsp.handles[(i/2)] =
                  BUILD_UINT16(pNPIMsg->pData[2 + i],pNPIMsg->pData[3 + i]);
                i += 2;
              }
            }
            break;

          case SNP_REGISTER_SERVICE_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->addCharValueDecRsp.status = pNPIMsg->pData[0];
              npiRetMsg.pMsg->addCharValueDecRsp.attrHandle =
                BUILD_UINT16(pNPIMsg->pData[1],pNPIMsg->pData[2]);
            }
            break;

          case SNP_SET_GATT_PARAM_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->setGattParamRsp.status = pNPIMsg->pData[0];
            }
            break;

          // Get GATT parameter of predefined NP service response.
          case SNP_GET_GATT_PARAM_RSP:
            if ( npiRetMsg.pMsg )
            {
              npiRetMsg.len = msgLen;

              // Explicitly copy response
              npiRetMsg.pMsg->getGattParamRsp.serviceID = pNPIMsg->pData[0];
              npiRetMsg.pMsg->getGattParamRsp.paramID = pNPIMsg->pData[1];
              npiRetMsg.pMsg->getGattParamRsp.pData =
                (uint8_t *)&pNPIMsg->pData[2];
            }
            break;

          // Get attribute value from NP response.
          case SNP_GET_ATTR_VALUE_RSP:
            //RFU
            break;

          // Set attribute value on NP response.
          case SNP_SET_ATTR_VALUE_RSP:
            //RFU
            break;
#endif //!SNP_LOCAL

          case SNP_CHAR_READ_IND:
            if (SNP_asyncCB)
            {
              snpCharReadInd_t readInd;

              readInd.connHandle =
                BUILD_UINT16(pNPIMsg->pData[0], pNPIMsg->pData[1]);
              readInd.attrHandle =
                BUILD_UINT16(pNPIMsg->pData[2], pNPIMsg->pData[3]);
              readInd.offset =
                BUILD_UINT16(pNPIMsg->pData[4], pNPIMsg->pData[5]);
              readInd.maxSize =
                BUILD_UINT16(pNPIMsg->pData[6], pNPIMsg->pData[7]);

              SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&readInd, msgLen);
            }
            break;

          // Characteristic write indication.
          case SNP_CHAR_WRITE_IND:
            {
              snpCharWriteInd_t writeInd;

              writeInd.connHandle =
                BUILD_UINT16(pNPIMsg->pData[0], pNPIMsg->pData[1]);
              writeInd.attrHandle =
                BUILD_UINT16(pNPIMsg->pData[2], pNPIMsg->pData[3]);
              writeInd.rspNeeded = pNPIMsg->pData[4];
              writeInd.offset =
                BUILD_UINT16(pNPIMsg->pData[5], pNPIMsg->pData[6]);
              writeInd.pData = (uint8_t *)&pNPIMsg->pData[7];

              if (SNP_asyncCB)
              {
                SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&writeInd, msgLen);
              }
            }
            break;

          case SNP_SEND_NOTIF_IND_CNF:
            if (SNP_asyncCB)
            {
              snpNotifIndCnf_t notifCnf;

              notifCnf.status = pNPIMsg->pData[0];
              notifCnf.connHandle =
                BUILD_UINT16(pNPIMsg->pData[1], pNPIMsg->pData[2]);

              SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&notifCnf, msgLen);
            }
            break;

          case SNP_CCCD_UPDATED_IND:
            if (SNP_asyncCB)
            {
              snpCharCfgUpdatedInd_t cccdInd;

              cccdInd.connHandle =
                BUILD_UINT16(pNPIMsg->pData[0], pNPIMsg->pData[1]);
              cccdInd.cccdHandle =
                BUILD_UINT16(pNPIMsg->pData[2], pNPIMsg->pData[3]);
              cccdInd.rspNeeded = pNPIMsg->pData[4];
              cccdInd.value =
                BUILD_UINT16(pNPIMsg->pData[5], pNPIMsg->pData[6]);

              SNP_asyncCB(pNPIMsg->cmd1, (snp_msg_t *)&cccdInd, msgLen);
            }
            break;

          default:
            // Unknown command
            break;
        }
      }
      break;

    default:
      // Unknown.
      break;
  }

#ifndef SNP_LOCAL
  if (pNPIMsg->cmd0 == SNP_NPI_SYNC_RSP_TYPE)
  {
    // This is a synchronous response, signal the application who requested this.
    SNP_responseReceived();
  }
#endif //SNP_LOCAL

  // Ok to deallocate
  SNP_free(pNPIMsg);
}
