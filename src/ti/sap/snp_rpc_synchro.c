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
#include <stdlib.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

#ifdef SNP_LOCAL
#include <ti/sysbios/knl/Event.h>
#include <icall.h>

#include "sap.h"
#include "simple_np.h"
#endif //SNP_LOCAL

#include "snp_rpc_synchro.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

#ifndef SNP_LOCAL
// Used to block SNP calls during a synchronous transaction.
Semaphore_Handle syncReq_sem;

// Signal application which requested the synchronous request.
Semaphore_Handle waitRsp_sem;
#endif //SNP_LOCAL

#ifdef SNP_LOCAL
snpMessageCBack_t sapCb;
snpMessageCBack_t snpCb;
#endif //SNP_LOCAL

/*********************************************************************
 * FUNCTIONS
 */

//extern ICall_Semaphore sem;

void SNP_open(void *syncHandle)
{
#ifdef SNP_LOCAL
  // Posting this semaphore is the equivalent of posting eventID for
  // eventHandle.
  SNP_init((Event_Handle *)syncHandle);

  // Process any posted events
  SNP_processEvents();
#else //!SNP_LOCAL
 // Create Synchronous message semaphore, ready to claim
 syncReq_sem = Semaphore_create(1, NULL, NULL);

 // Create waitResponse semaphore, must be signaled before progressing.
 waitRsp_sem = Semaphore_create(0, NULL, NULL);
#endif //SNP_LOCAL
}


void SNP_close(void)
{
#ifndef SNP_LOCAL
  // Delete SNP semaphores
  Semaphore_delete(&syncReq_sem);
  Semaphore_delete(&waitRsp_sem);
#endif //SNP_LOCAL
}

#ifndef SNP_LOCAL
void SNP_enterCS(void)
{
  // Wait for any ongoing requests to finish.
  Semaphore_pend(syncReq_sem, BIOS_WAIT_FOREVER);
}

void SNP_exitCS(void)
{
  // Release the synchronous request semaphore.
  Semaphore_post(syncReq_sem);
}

void SNP_waitForResponse(void)
{
  // Wait for a response.
  Semaphore_pend(waitRsp_sem, BIOS_WAIT_FOREVER);
}


void SNP_responseReceived(void)
{
  // Response received, release the semaphore.
  Semaphore_post(waitRsp_sem);
}
#endif //SNP_LOCAL

// Pass in size get a pointer to dynamically allocated buffer.
void *SNP_malloc(uint16_t size)
{
#ifdef SNP_LOCAL
  return ICall_malloc(size);
#else //!SNP_LOCAL
  return malloc(size);
#endif //SNP_LOCAL
}

// Pass in the pointer to a buffer allocated by SNP_malloc to free the buffer.
void SNP_free(void* pointer)
{
#ifdef SNP_LOCAL
  ICall_free(pointer);
#else //!SNP_LOCAL
  free(pointer);
#endif //SNP_LOCAL
}

#ifdef SNP_LOCAL
/**
 *  @fn          SNP_mallocNPIFrame
 *
 *  @brief       allocate a NPI Frame.  called in SNP_LOCAL only.
 *
 *  @param[in]   len    : length in bytes
 *
 *  @return  None.
 */
_npiFrame_t *SNP_mallocNPIFrame(uint16_t len)
{
  _npiFrame_t *pMsg;

  // Allocate memory for NPI Frame
  pMsg = (_npiFrame_t *)ICall_malloc(sizeof(_npiFrame_t) + len);

  if (pMsg != NULL)
  {
      // Assign Data Length of Frame
      pMsg->dataLen = len;

      // Assign pData to first byte of payload
      // Pointer arithmetic of + 1 is equal to sizeof(_npiFrame_t) bytes
      // then cast to unsigned char * for pData
      pMsg->pData = (unsigned char *)(pMsg + 1);
  }

  return pMsg;
}

// Register SNP
void SNP_RPC_registerCBack(uint8_t snpID, snpMessageCBack_t pCB)
{
  if (snpID == SAP_TASK_ID)
  {
    sapCb = pCB;
  }
  else if (snpID == SNP_TASK_ID)
  {
    snpCb = pCB;
  }
}

// Send message to task
void SNP_RPC_sendToHost(uint8_t senderSnpID, _npiFrame_t *pMsg)
{
  if (senderSnpID == SAP_TASK_ID && snpCb)
  {
    // SAP to SNP
    snpCb(pMsg);
  }
  else if (senderSnpID == SNP_TASK_ID && sapCb)
  {
    // SNP to SAP
    sapCb(pMsg);
  }
}
#endif //SNP_LOCAL

#ifdef SNP_LOCAL
// Task pends in function until all pend conditions are met.  Allows execution
// of SNP and ICall flags while waiting.  For use with SNP_LOCAL only.
uint32_t SNP_pend(Event_Handle event, uint32_t andFlags, uint32_t orFlags,
                  uint32_t timeout)
{
  uint32_t flags;
  uint32_t savedAndFlags = 0;
  uint32_t savedOrFlags = 0;

  while(1)
  {
    // Pend on event flags posted by the application and OR in the SNP
    // Event flags.  This includes AND flags to not block SNP when it is waiting
    // on events.
    flags = Event_pend(event, Event_Id_NONE, orFlags + andFlags + SNP_ALL_EVENTS, timeout);

    // Check if timeout
    if (!flags)
    {
      break;
    }

    if (flags & andFlags)
    {
      // Save the AND flags until we have all of them.
      savedAndFlags |= flags & andFlags;
    }

    if (flags & orFlags)
    {
      // Save the OR flags until we have all of them.
      savedOrFlags |= flags & orFlags;
    }

    if (flags & SNP_ALL_EVENTS)
    {
      // Process SNP events
      SNP_processEvents();
    }

    // set savedFlags back in.
    flags |= savedAndFlags | savedOrFlags;

    // Check if flags OR savedAndFlags matches all andFlags AND at least one
    // OR flag.
    if (((flags & andFlags) == andFlags) && (flags & orFlags))
    {
      // All conditions were met.
      break;
    }
  }

  return flags;
}
#endif //SNP_LOCAL

/*********************************************************************
*********************************************************************/
