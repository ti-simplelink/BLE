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

#ifndef SNP_SYNCHRO_H
#define SNP_SYNCHRO_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#ifdef SNP_LOCAL
#include <ti/sysbios/knl/Event.h>
#endif //SNP_LOCAL

#include <ti/npi/npi_data.h>

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * MACROS
 */

// SAP message to the SNP
#define SAP_SendMessage(msg) SNP_RPC_sendToHost(SAP_TASK_ID, msg)

// SNP message to the SAP
#define SNP_SendMessage(msg) SNP_RPC_sendToHost(SNP_TASK_ID, msg)

// Register the SAP callback function
#define SAP_RegisterCBack(cb) SNP_RPC_registerCBack(SAP_TASK_ID, cb)

// Register the SNP callback function
#define SNP_RegisterCback(cb) SNP_RPC_registerCBack(SNP_TASK_ID, cb)

/*********************************************************************
 * CONSTANTS
 */

#define SAP_TASK_ID 0x00  // This is the SAP task
#define SNP_TASK_ID 0x01  // This is the SNP task

/*********************************************************************
 * TYPEDEFS
 */

typedef void (*snpMessageCBack_t)(_npiFrame_t *pNPIMsg);

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * HELPER FUNCTIONS
 */

/*********************************************************************
 * FUNCTIONS
 */

// Call before using any synchronization.
extern void SNP_open(void *syncHandle);

// Call when closing connection with SNP
extern void SNP_close(void);

#ifndef SNP_LOCAL
// enter critical section.
extern void SNP_enterCS(void);

// exist critical section.
extern void SNP_exitCS(void);

// Block an application until a response is received.
extern void SNP_waitForResponse(void);

// Unblock application from running.
extern void SNP_responseReceived(void);
#endif //SNP_LOCAL

// Pass in size get a pointer to dynamically allocated buffer.
extern void *SNP_malloc(uint16_t size);

extern _npiFrame_t *SNP_mallocNPIFrame(uint16_t len);

// Pass in the pointer to a buffer allocated by SNP_malloc to free the buffer.
extern void SNP_free(void* pointer);

// Register SNP
extern void SNP_RPC_registerCBack(uint8_t snpID, snpMessageCBack_t pCB);

// Send message directly to other task
extern void SNP_RPC_sendToHost(uint8_t senderSnpID, _npiFrame_t *pMsg);

#ifdef SNP_LOCAL
// Task pends in function until all pend conditions are met.  Allows execution
// of SNP and ICall flags while waiting. Only used when SNP_LOCAL is defined.
extern uint32_t SNP_pend(Event_Handle event, uint32_t andFlags,
                         uint32_t orFlags, uint32_t timeout);
#endif //SNP_LOCAL
/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* SNP_SYNCHRO_H */
