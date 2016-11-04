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

#ifndef SBL_INTERNAL_H
#define SBL_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "sbl.h"

/*********************************************************************
 * CONSTANTS
 */

// Indexes for pin configurations in PIN_Config array
#define RST_PIN_IDX             0
#define BL_PIN_IDX              1

#ifdef CC26XX
#include <ti/drivers/PIN.h>

extern PIN_Handle hsblPins;

#define WRITE_BL(value) PIN_setOutputValue(hsblPins, blPIN, value);
#define WRITE_RST(value) PIN_setOutputValue(hsblPins, rstPIN, value);
#else
#include <ti/drivers/GPIO.h>

#define WRITE_BL(value) GPIO_write(blPIN, value);
#define WRITE_RST(value) GPIO_write(rstPIN, value);
#endif //CC26xx

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
*  EXTERNAL VARIABLES
*/

extern uint32_t rstPIN;
extern uint32_t blPIN;

/*********************************************************************
 * FUNCTIONS
 */

extern uint8_t SBL_initPins(SBL_Params *params);
extern void SBL_closePins(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SBL_INTERNAL_H */
