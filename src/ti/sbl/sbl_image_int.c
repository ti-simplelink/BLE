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
#include <stdbool.h>

#include "sbl.h"
#include "sbl_image.h"

/*********************************************************************
 * CONSTANTS
 */

#define BL_CONFIG_REG_H   0xFFED
#define BL_CONFIG_REG_L   0xFFEC
#define BL_ENABLE         0xC5

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern unsigned short SNP_Code[];
extern unsigned long SNP_Code_length;
extern unsigned long SNP_Code_sections;

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8_t flashIsOpen = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void flashRead(uint32_t addr, uint16_t len, uint8_t *pBuf);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

static void flashRead(uint32_t addr, uint16_t len, uint8_t *pBuf)
{
   uint8_t  *flashPtr;
   uint16_t flashIndex;

   flashPtr = (uint8_t *)addr;

   for(flashIndex = 0; flashIndex <= len; flashIndex++)
   {
     /* data fit into buffer */
     pBuf[flashIndex] = *flashPtr++;
   }
}

uint8_t SBL_IMG_open( void )
{
  if (( SNP_Code_sections != 0 ) &&
      ( SNP_Code_length != 0 ) )
  {
    flashIsOpen = 1;
    return SBL_SUCCESS;
  }

  return SBL_FAILURE;
}

bool SBL_IMG_isValid( uint32_t hdrAddr )
{
  // Verify the flash has been opened
  if ( flashIsOpen )
  {
    // Check for a valid BL_BACKDOOR_CONFIG
    // A valid config is considered when
    // 1. BOOTLOADER_ENABLE is set AND
    // 2. BL_ENABLE is set to enable backdoor
    // See section 9.1.1.13 of swcu117 for more information
    if (( (SNP_Code[BL_CONFIG_REG_L] & 0x00FF) == BL_ENABLE ) &&
        ( (SNP_Code[BL_CONFIG_REG_H] & 0xFF00) == (BL_ENABLE << 8) ))
    {
        return true;
    }
  }
  return false;
}

uint32_t SBL_IMG_getSize( uint32_t hdrAddr )
{
  // Verify the flash has been opened
  if ( flashIsOpen )
  {
    // Length in NP_Code_length number of bytes
    return (SNP_Code_length);
  }
  return 0;
}

uint32_t SBL_IMG_getOffset( uint32_t hdrAddr )
{
  return 0;
}

uint8_t SBL_IMG_read( uint32_t addr, uint8_t *pBuf, uint16_t len)
{
  flashRead(addr, len, pBuf);
  return SBL_SUCCESS;
}

void SBL_IMG_close( void )
{
  flashIsOpen = 0;
}
