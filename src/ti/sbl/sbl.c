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
 * *  Redistributions in binary form must reproduce bthe above copyright
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
#include <stdbool.h>

#include "sbl_internal.h"
#include "sbl.h"
#include "sbl_image.h"
#include "sbl_cmd.h"
#include "sbl_tl.h"

#include <ti/drivers/UART.h>

#include "Board.h"

/*********************************************************************
 * CONSTANTS
 */

// Imperically found delay count for 1 ms
#define SBL_DELAY_1_MS          4800

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static uint8_t SBL_lastSectorProtected( void );
static void SBL_utilDelay(uint32_t ms);
static uint8_t SBL_eraseFlash(uint32_t addr, uint32_t size);
static void SBL_utilDelay(uint32_t ms);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @fn      SBL_initParams
 *
 * @brief   Initializes SBL parameters
 *
 * @param   params - SBL parameter structure to be set to SBL default values
 *
 * @return  None.
 */
void SBL_initParams(SBL_Params *params)
{
  // Make invalid.
  memset(params, 0xFF, sizeof(SBL_Params));

  // Set target interface to undefined.
  params->targetInterface = SBL_DEV_INTERFACE_UNDEFINED;
}

/**
 * @fn      SBL_open
 *
 * @brief   Opens the SBL port for writing images
 *
 * @param   params - SBL parameters to initialize the port with
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_open(SBL_Params *params)
{
  // Initialize SBL Pins
  if (SBL_initPins(params))
  {
    return SBL_FAILURE;
  }

  // Open SBL Transport Layer
  return SBL_TL_open(params->targetInterface, params->localInterfaceID);
}

/**
 * @fn      SBL_writeImage
 *
 * @brief   Writes image to the target device using SBL and resets device
 *
 * @param   image - parameters that describe the image to be written to target
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_writeImage(SBL_Image *image)
{
  uint32_t imgSize = 0;
  uint32_t bytesWritten = 0;
  uint32_t transLen = 0;
  uint32_t imageAddr = 0;
  uint8_t  imgBuf[SBL_MAX_TRANSFER];

  if (SBL_IMG_open() == SBL_SUCCESS)
  {
    // Verify the image is valid
    if (SBL_IMG_isValid(image->imgInfoLocAddr))
    {
      // Get image size
      imgSize = SBL_IMG_getSize(image->imgInfoLocAddr);

      // Align size on 4B boundary
      if (imgSize &  0x03)
      {
        imgSize += (4 - (imgSize % 4));
      }

      // Image size must be non-zero
      if (imgSize)
      {
        // Check if the last sector is protected
        if (!SBL_lastSectorProtected())
        {
          // if the last sector is not protected, do not touch the last sector
          imgSize -= SBL_PAGE_SIZE;
        }

        // Erase Flash
        SBL_eraseFlash(image->imgTargetAddr, imgSize);

        // Send Download command to target
        SBL_CMD_download(image->imgTargetAddr, imgSize);

        // Verify Device Status after download command
        if (SBL_CMD_getStatus() != SBL_CMD_RET_SUCCESS)
        {
          SBL_IMG_close();

          return SBL_FAILURE;
        }

        // Update addr to point to image instead of image header
        imageAddr = image->imgLocAddr;

        // Write image to target device
        while(bytesWritten < imgSize)
        {
          // Determine number of bytes in this transfer
          transLen = (SBL_MAX_TRANSFER < (imgSize - bytesWritten))?
                             SBL_MAX_TRANSFER : (imgSize - bytesWritten);

          // Read bytes of image
          SBL_IMG_read(imageAddr, imgBuf, transLen);

          // Send data to target
          SBL_CMD_sendData(imgBuf, transLen);

          // Verify Device Status after download command
          if (SBL_CMD_getStatus() != SBL_CMD_RET_SUCCESS)
          {
            SBL_IMG_close();

            return SBL_FAILURE;
          }

          // Update bytes written and image address of next read
          bytesWritten += transLen;
          imageAddr += transLen;
        }

        // Write complete
        SBL_IMG_close();

        // Send reset command
        SBL_CMD_reset();

        return SBL_SUCCESS;
      }
    }

    SBL_IMG_close();
  }

  return SBL_FAILURE;
}

/**
 * @fn      SBL_eraseFlash
 *
 * @brief   Erases specific flash sectors of target device
 *
 * @param   addr - address of image to be erased
 * @param   size - total amount of bytes to erase starting from addr
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
static uint8_t SBL_eraseFlash(uint32_t addr, uint32_t size)
{
  uint16_t i;
  uint16_t numPages = 0;
  uint32_t numBytes = size;
  uint32_t firstPageAddr = addr;


  // Calculate number of flash pages that need to be erased
  if (addr % SBL_PAGE_SIZE)
  {
    // Starting address is not page aligned, add page to erase total and
    // remove bytes in the starting page from total
    numPages++;
    numBytes -= (SBL_PAGE_SIZE - (addr % SBL_PAGE_SIZE));

    // Reset first page address to beginning of page that contains addr
    firstPageAddr -= (addr % SBL_PAGE_SIZE);
  }

  // Calculate pages from remaining byte total
  numPages += (numBytes / SBL_PAGE_SIZE);
  numPages += (numBytes % SBL_PAGE_SIZE) ? 1 : 0;

  for(i = 0; i < numPages; i++)
  {
    // Erase page
    SBL_CMD_sectorErase(firstPageAddr);

    // Verify Device Status after sector erase command
    if (SBL_CMD_getStatus() != SBL_CMD_RET_SUCCESS)
    {
      return SBL_FAILURE;
    }

    // Update page address to next page
    firstPageAddr += SBL_PAGE_SIZE;
  }

  return SBL_SUCCESS;
}

/**
 * @fn      SBL_close
 *
 * @brief   Closes SBL port
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_close(void)
{
  SBL_closePins();

  // Close SBL Transport Layer
  SBL_TL_close();

  return SBL_SUCCESS;
}

/**
 * @fn      SBL_closeTarget
 *
 * @brief   Exit SBL
 *
 * @param   None.
 *
 * @return  None.
 */
void SBL_closeTarget( void )
{
  // Set BL PIN high and then set Reset PIN low to exit SBL
  WRITE_BL(1);
  WRITE_RST(0);

  SBL_utilDelay(15);

  // Release Reset PIN while keeping BL pin low
  WRITE_RST(1);

  // Delay to be tuned
  SBL_utilDelay(150);

  SBL_utilDelay(150);
}

/**
 * @fn      SBL_openTarget
 *
 * @brief   Forces target device into SBL
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_openTarget( void )
{
  // Set BL PIN low and then set Reset PIN low to enter SBL
  WRITE_BL(0);
  WRITE_RST(0);

  SBL_utilDelay(15);

  // Release Reset PIN while keeping BL pin low
  WRITE_RST(1);

  // Delay to be tuned
  SBL_utilDelay(150);

  // Release BL Pin now that target should be in SBL mode
  WRITE_BL(1);

  SBL_utilDelay(150);

  // Send initial packet so target can detect baud rate
  if (SBL_TL_uartAutoBaud() == SBL_DEV_ACK)
  {
    return SBL_SUCCESS;
  }

  return SBL_FAILURE;
}

/**
 * @fn      SBL_resetTarget
 *
 * @brief   Forces target device to boot from flash image instead of SBL. This
 *          function can be called before SBL_open() or after SBL_open()
 *
 * @param   rstPinID - Board Pin ID of reset PIN
 * @param   blPinID  - Board Pin ID of boot loader PIN
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_resetTarget(uint32_t rstPinID, uint32_t blPinID)
{
  // Guarantee that Boot Loader Pin is high during reset toggle
  WRITE_BL(1);

  // Set reset PIN low
  WRITE_RST(0);

  SBL_utilDelay(15);

  // Release Reset PIN
  WRITE_RST(1);

  return SBL_SUCCESS;
}

/**
 * @fn      SBL_lastSectorProtected
 *
 * @brief   Verifies if the last sector is protected. Last sectors holds
 *          the CCFG configuration
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
static uint8_t SBL_lastSectorProtected( void )
{
  uint8_t rsp[4];
  SBL_CMD_memoryRead(0x1FFF0,4,rsp);

  if (rsp[3] == 0x7F)
  {
      // Protected
      return SBL_SUCCESS;
  }
  else
  {
      // Not Protected
      return SBL_FAILURE;
  }
}

/**
 * @fn      SBL_utilDelay
 *
 * @brief   Simple for loop to burn cycles while target device is resetting
 *
 * @param   ms - Milliseconds to delay
 *
 * @return  None.
 */
static void SBL_utilDelay(uint32_t ms)
{
  volatile uint32_t i,j; // Force compiler to not optimize loops

  for (i = 0; i < ms; i++)
  {
    // Delay one millisecond
    for (j = SBL_DELAY_1_MS; j > 0; j--);
  }
}
