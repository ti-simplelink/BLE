/*
 * Copyright (c) 2014-2016, Texas Instruments Incorporated
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

#include <xdc/std.h>
#include <string.h>

#include "sbl.h"
#include "sbl_tl.h"

#include <ti/drivers/UART.h>

/*********************************************************************
 * CONSTANTS
 */

enum {
  SBL_HDR_LEN_IDX = 0,
  SBL_HDR_CKS_IDX,
  SBL_HDR_SIZE,
};

#define SBL_ACK_SIZE            2

#define DEVICE_ACK              0xCC
#define DEVICE_NACK             0x33

#define SBL_UART_BR             115200

const uint8_t NACK[] =          { 0x00, 0x33 };
const uint8_t ACK[] =           { 0x00, 0xcc };
const uint8_t BAUD[] =          { 0x55, 0x55 };

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

//! \brief UART Handle for UART Driver
UART_Handle sblUartHandle;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static uint8_t SBL_TL_CKS(uint8_t cmd, uint8_t *pData, uint16_t len);
static uint8_t SBL_TL_sendACK(uint8_t ack);
static uint8_t SBL_TL_getRspACK(void);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @fn      SBL_TL_open
 *
 * @brief   Open device port for communication with the target device. Currently
 *          only supports UART
 *
 * @param   pType - SBL_DEV_INTERFACE_[UART,SPI]
 * @param   pID - local serial interface ID (i.e. CC2650_UART0)
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_TL_open(uint8_t pType, uint8_t pID)
{
  UART_Params params;

  // Currently only support over UART
  if (pType == SBL_DEV_INTERFACE_UART)
  {
    // Configure UART parameters.
    // No parity bit, One stop bit set by default
    UART_Params_init(&params);
    params.baudRate = SBL_UART_BR;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;
    params.readEcho = UART_ECHO_OFF;

    // Open UART port for sending SBL commands
    sblUartHandle = UART_open(pID, &params);
    if (sblUartHandle == NULL)
    {
      return SBL_FAILURE;
    }

    // Work around for UART driver re-open bug. Sometimes there is a mystery byte already
    // read prior to sending any commands to the target device
    SBL_TL_flushRxBuffer();

    return SBL_SUCCESS;
  }

  return SBL_FAILURE;
}

/**
 * @fn      SBL_TL_getRspACK
 *
 * @brief   Get ACK/NACK response from the target device
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_DEV_ACK, SBL_DEV_NACK, or SBL_FAILURE if neither ACK/NACK
 */
static uint8_t SBL_TL_getRspACK(void)
{
  uint8_t rsp[SBL_ACK_SIZE] = {0xFF, 0xFF};

  UART_read(sblUartHandle, rsp, sizeof(rsp));

  if (0 == memcmp(rsp, ACK, sizeof(rsp)))
  {
    return SBL_DEV_ACK;
  }
  else if (0 == memcmp(rsp, NACK, sizeof(rsp)))
  {
    return SBL_DEV_NACK;
  }
  else
  {
    return SBL_FAILURE;
  }
}

/**
 * @fn      SBL_TL_getRsp
 *
 * @brief   Get response message from the target device
 *
 * @param   pData - pointer to byte array to store data
 * @param   maxSize - size of byte array pointed to by pData
 * @param   len - will be set to the length of data written to pData
 *
 * @return  uint8_t - SBL_SUCCESS or SBL_FAILURE
 */
uint8_t SBL_TL_getRsp(uint8_t *pData, uint16_t maxSize, uint16_t *len)
{
  uint8_t hdr[SBL_HDR_SIZE];

  // Read Response header
  UART_read(sblUartHandle, hdr, sizeof(hdr));

  // Check if length of incoming response is too long
  if (maxSize < (hdr[SBL_HDR_LEN_IDX] - sizeof(hdr)))
  {
    return SBL_FAILURE;
  }

  // Read Response Payload
  UART_read(sblUartHandle, pData,hdr[SBL_HDR_LEN_IDX] - sizeof(hdr));

  // Verify Checksum
  if (hdr[SBL_HDR_CKS_IDX] != SBL_TL_CKS(0, pData, hdr[SBL_HDR_LEN_IDX] - sizeof(hdr)))
  {
    return SBL_FAILURE;
  }

  // Set length parameter to length of payload data
  *len = hdr[SBL_HDR_LEN_IDX];

  // Respond with ACK
  return SBL_TL_sendACK(DEVICE_ACK);
}

/**
 * @fn      SBL_TL_sendCmd
 *
 * @brief   Sends a SBL command to target device
 *
 * @param   cmd - command ID
 * @param   pData - pointer to command payload
 * @param   len - length of command payload
 *
 * @return  uint8_t - SBL_SUCCESS
 */
uint8_t SBL_TL_sendCmd(uint8_t cmd, uint8_t *pData, uint16_t len)
{
  uint8_t hdr[SBL_HDR_SIZE + 1]; // Header + CMD byte

  // Initalize Header
  hdr[SBL_HDR_LEN_IDX] = len + sizeof(hdr);             // Length
  hdr[SBL_HDR_CKS_IDX] = SBL_TL_CKS(cmd, pData, len);   // Checksum
  hdr[2] = cmd;                                         // Command

  // Send Header
  UART_write(sblUartHandle, hdr, sizeof(hdr));

  // Send Packet
  if (len)
  {
    UART_write(sblUartHandle, pData, len);
  }

  return SBL_TL_getRspACK();
}

/**
 * @fn      SBL_TL_sendACK
 *
 * @brief   Send ACK/NACK response to the target device
 *
 * @param   ack - ACK/NACK value
 *
 * @return  uint8_t - SBL_SUCCESS
 */
static uint8_t SBL_TL_sendACK(uint8_t ack)
{
  uint8_t rsp[2];

  // Initalize response
  rsp[0] = 0x00; // Must write zero byte first for response
  rsp[1] = ack;

  UART_write(sblUartHandle, rsp, sizeof(rsp));

  return SBL_SUCCESS;
}

/**
 * @fn      SBL_TL_uartAutoBaud
 *
 * @brief   Send baud packet to allow target to auto detect
 *          baud rate of SBL transmissions
 *
 * @param   ack - ACK/NACK value
 *
 * @return  uint8_t - SBL_SUCCESS
 */
uint8_t SBL_TL_uartAutoBaud(void)
{
  UART_write(sblUartHandle, BAUD, sizeof(BAUD));

  uint8_t autoBaudStatus = SBL_FAILURE;
  while(SBL_FAILURE == autoBaudStatus)
  {
    autoBaudStatus = SBL_TL_getRspACK();
  }

  return autoBaudStatus;
}

/**
 * @fn      SBL_TL_close
 *
 * @brief   Close interface to target device
 *
 * @param   None.
 *
 * @return  None.
 */
void SBL_TL_close(void)
{
  UART_close(sblUartHandle);
}

/**
 * @fn      SBL_TL_CKS
 *
 * @brief   Calculates checksum over a byte array
 *
 * @param   cmd - optional command byte from sending messages
 * @param   pData - pointer to byte array to store data
 * @param   len - length of byte array pointed to by pData
 *
 * @return  uint8_t - checksum
 */
static uint8_t SBL_TL_CKS(uint8_t cmd, uint8_t *pData, uint16_t len)
{
  uint8_t checksum = cmd;
  uint16_t i;

  for(i = 0; i < len; i++)
  {
    checksum += pData[i];
  }

  return checksum;
}
