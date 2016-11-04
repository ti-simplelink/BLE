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

#ifndef SNP_RPC_H
#define SNP_RPC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "snp.h"

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

  /*********************************************************************
 * TYPEDEFS
 */

typedef void (* SNP_RPC_eventCBRouter_t)(snpEvt_t *pEvt);
typedef void (* SNP_RPC_asyncCB_t)(uint8_t cmd1, snp_msg_t *pMsg, uint16_t msgLen);

typedef struct
{
  uint16_t   len;     // Length of the received message
  snp_msg_t  *pMsg;   // SNP message format.  This is a pointer to the application's buffer.
} snpSyncRspData_t;

/*********************************************************************
 * HELPER FUNCTIONS
 */

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
extern void SNP_RPC_registerSAPCBs(SNP_RPC_asyncCB_t asyncCB,
                                   SNP_RPC_eventCBRouter_t eventCB);

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      SNP_RPC_maskEvent
 *
 * @brief   mask of events to to receive from the Network Processor
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_maskEvent(snpMaskEventReq_t *pReq,
                                 snpMaskEventRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_getRevision
 *
 * @brief   Get SNP Image revision and Stack revision numbers
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_getRevision(snpGetRevisionRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_getRand
 *
 * @brief   Get random number generated using TRNG on SNP
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_SUCCESS
 */
extern uint8_t SNP_RPC_getRand(snpGetRandRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_sendHCICommand
 *
 * @brief   Send an HCI command over SNP message
 *
 * @param   pReq - pointer to SNP request message
 * @param   paramLen - length of data field of SNP request
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_sendHCICommand(snpHciCmdReq_t *pHciCmd,
                                      uint8_t paramLen);

/*********************************************************************
 * @fn      SNP_RPC_testCommand
 *
 * @brief   Get SNP memory statistics
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_testCommand(void);

/*********************************************************************
 * @fn      SNP_RPC_getStatus
 *
 * @brief   Get SNP status
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_getStatus(snpGetStatusCmdRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_startAdvertising
 *
 * @brief   Start advertising.  Advertising data must be set with
 *          SNP_RPC_updateAdvData first. Command sets the following:
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
 * @return  uint8_t - SNP_RPC_SUCCESS, SNP_RPC_ADV_DATA_NOT_READY,
 *                    or SNP_RPC_ADV_NOT_AVAILABLE
 */
extern uint8_t SNP_RPC_startAdvertising(snpStartAdvReq_t *pReq);

/*********************************************************************
 * @fn      SNP_RPC_stopAdvertising
 *
 * @brief   Stop advertising
 *
 * @param   none
 *
 * @return  uint8_t - SNP_RPC_SUCCESS, SNP_RPC_NOT_ADVERTISING
 */
extern uint8_t SNP_RPC_stopAdvertising(void);

/*********************************************************************
 * @fn      SNP_RPC_setAdvertisementData
 *
 * @brief   Configure Advertising or Scan Response Data.
 *
 * @param   pReq - pointer to SNP request message
 * @param   dataLen - length of data field of SNP request
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_setAdvertisementData(snpSetAdvDataReq_t *pReq,
                                            uint8_t datalen);

/*********************************************************************
 * @fn      SNP_RPC_terminateConnection
 *
 * @brief   Terminate the existing connection.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_terminateConnection(snpTermConnReq_t *pReq);

/*********************************************************************
 * @fn      SNP_RPC_updateConnectionParams
 *
 * @brief   Update the parameters of an existing connection.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_updateConnectionParams(snpUpdateConnParamReq_t *pReq);

/*********************************************************************
 * @fn      SNP_RPC_setGAPparam
 *
 * @brief   Set GAP parameter on SNP
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_setGAPparam(snpSetGapParamReq_t *pReq,
                                   snpSetGapParamRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_getGAPparam
 *
 * @brief   Get GAP parameter from SNP
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_getGAPparam(snpGetGapParamReq_t *pReq,
                                   snpGetGapParamRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_setSecurityParam
 *
 * @brief   Set the Security Requirements for SNP.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_setSecurityParam(snpSetSecParamReq_t *pReq,
                                        snpSetSecParamRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_sendSecurityRequest
 *
 * @brief   Set the Security Requirements for SNP.
 *
 * @param   none
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_sendSecurityRequest(void);

/*********************************************************************
 * @fn      SNP_RPC_setAuthenticationData
 *
 * @brief   Set the Authentication data for a pairing procedure.
 *
 * @param   pReq - pointer to SPN request message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_setAuthenticationData(snpSetAuthDataReq_t *pReq);

/*********************************************************************
 * @fn      SNP_RPC_setWhiteListParam
 *
 * @brief   Set the White List policy.
 *
 * @param   pReq - pointer to SPN request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_setWhiteListParam(snpSetWhiteListReq_t *pReq);

/*********************************************************************
 * @fn      SNP_RPC_addService
 *
 * @brief   Add a service to the GATT server on the SNP.
 *
 * @param   pReq - pointer to SNP request message
 * @param   uuidLen - Length of UUID field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_addService(snpAddServiceReq_t *pReq, uint8_t uuidLen,
                                  snpAddServiceRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_addCharValueDec
 *
 * @brief   Add a characteristic value to a service table. Used after
 *          calling SNP_RPC_addService to include characteristics within the
 *          service.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pReq - Length of UUID field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_addCharValueDec(snpAddCharValueDeclReq_t *pReq,
                                       uint8_t uuidLen,
                                       snpAddCharValueDeclRsp_t *pRsp);

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
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_addCharDescDec(snpAddCharDescDeclReq_t *pReq,
                                      snpAddCharDescDeclRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_registerService
 *
 * @brief   Registers the service within the GATT Server.  This is called
 *          after a service is added with SNP_RPC_addService and all service
 *          characteristics for that service are added with
 *          SNP_RPC_addCharacteristic.
 *
 * @param   pReq - pointer to SNP request message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_registerService(snpRegisterServiceRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_readCharCnf
 *
 * @brief   Respond to a characteristic read request from a GATT Client.
 *
 * @param   pCnf - pointer to SNP confirmation message
 * @param   size - length of the confirmation data payload
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_readCharCnf(snpCharReadCnf_t *pCnf, uint16_t size);

/*********************************************************************
 * @fn      SNP_RPC_writeCharCnf
 *
 * @brief   Respond to a characteristic write request from GATT Client
 *
 * @param   pCnf - pointer to SNP confirmation message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_writeCharCnf(snpCharWriteCnf_t *pCnf);

/*********************************************************************
 * @fn      SNP_RPC_sendNotifInd
 *
 * @brief   Send a notification for a characteristic value to GATT Client
 *
 * @param   pReq - pointer to SNP request message
 * @param   dataLen - length of the notification data payload
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_sendNotifInd(snpNotifIndReq_t *pReq, uint8_t dataLen);

/*********************************************************************
 * @fn      SNP_RPC_charConfigUpdatedRsp
 *
 * @brief   A GATT Client has requested to updated the CCCD value. the AP
 *          has a choice of validating the request or rejecting.
 *
 * @param   pReq - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_charConfigUpdatedRsp(snpCharCfgUpdatedRsp_t *pReq);

/*********************************************************************
 * @fn      SNP_RPC_setGATTParam
 *
 * @brief   Overwrite characteristic values of the Device Info Service.
 *
 * @param   pReq - pointer to SNP request message
 * @param   dataLen - Length of data field in request message
 * @param   pRsp - pointer to SNP response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_setGATTParam(snpSetGattParamReq_t *pReq, uint16_t dataLen,
                                    snpSetGattParamRsp_t *pRsp);

/*********************************************************************
 * @fn      SNP_RPC_getGATTParam
 *
 * @brief   Get characteristic values of the Device Info Service.
 *
 * @param   pReq - pointer to SNP request message
 * @param   pRsp - pointer to SNP response message
 * @param   dataLen - Length of data field in response message
 *
 * @return  uint8_t - SNP_RPC_SUCCESS
 */
extern uint8_t SNP_RPC_getGATTParam(snpGetGattParamReq_t *pReq,
                                    snpGetGattParamRsp_t *pRsp,
                                    uint16_t *dataLen);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SNP_RPC_H */
