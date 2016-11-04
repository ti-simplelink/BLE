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

#ifndef EXT_FLASH_LAYOUT_H
#define EXT_FLASH_LAYOUT_H

// Page size
#define EFL_PAGE_SIZE               0x1000
#define EFL_FLASH_SIZE              0x80000

// Application Image
#define EFL_ADDR_IMAGE_APP          0x00000
#define EFL_SIZE_IMAGE_APP          0x20000

// Stack or Network Processor image.
#define EFL_ADDR_IMAGE_BLE          0x20000
#define EFL_SIZE_IMAGE_BLE          0x20000

// Recovery region (factory reset)
#define EFL_ADDR_RECOVERY           0x40000
#define EFL_SIZE_RECOVERY           0x20000

// Image information (meta-data)
#define EFL_ADDR_META               0x78000
#define EFL_SIZE_META               0x08000

#define EFL_IMAGE_INFO_ADDR_APP     ( EFL_ADDR_META + 0x0000 )
#define EFL_IMAGE_INFO_ADDR_BLE     ( EFL_ADDR_META + EFL_PAGE_SIZE )
#define EFL_IMAGE_INFO_ADDR_FACTORY ( EFL_ADDR_META + EFL_PAGE_SIZE*2 )

// Image types
#define EFL_OAD_IMG_TYPE_APP        1
#define EFL_OAD_IMG_TYPE_STACK      2
#define EFL_OAD_IMG_TYPE_NP         3
#define EFL_OAD_IMG_TYPE_FACTORY    4

// Address/length resolution
#define EFL_OAD_ADDR_RESOLUTION     4

typedef struct
{
  uint16_t crc[2];     // crc[0] calculated by OAD client before transfer
                       // crc[1] calculated from flash after transfer
  uint16_t ver;        // Version number
  uint16_t len;        // Image length in 4-byte blocks (OAD_ADDR_RESOLUTION)
  uint8_t uid[4];      // User-defined Image Identification bytes
  uint16_t addr;       // Address offset in 4-byte blocks (OAD_ADDR_RESOLUTION)
  uint8_t imgType;     // BIM, APP, STACK
  uint8_t status;      // This field is not used until after the data is stored.
                       // A bootloader or other may use this field to check if the
                       // Image has already been loaded into internal flash.
} ExtImageInfo_t;

#endif /* EXT_FLASH_LAYOUT_H */
