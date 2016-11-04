
#ifndef BLE_BOARD_H
#define BLE_BOARD_H

#ifdef __MSP432P401R__
/*
 * Removes conflicts between drivers/bsp/Board.h (used in Energia.h),
 * and the Board.h included here.
  */
#ifdef Board_WATCHDOG
#undef Board_WATCHDOG
#endif //Board_WATCHDOG
#ifdef Board_WIFI
#undef Board_WIFI
#endif //Board_WIFI
#include "variants/MSP_EXP432P401R/Board.h"

/* Pin numbers used by Energia pin functions and in the GPIO tables. */
#define PIN2_5             19
#define PIN6_0             2
#define PIN6_7             35

#define CC2650_RESET_PIN   PIN6_7

/* Pins used for power savings wakeup and sleep. */
#define BLE_Board_MRDY     PIN6_0
#define BLE_Board_SRDY     PIN2_5

/* Pin used to switch the CC2650 into its bootloader for SBL. */
#define BLE_SBL_BL_PIN     PIN6_0

#define BLE_UART_ID        Board_UARTA2 // =1, USB Serial is Board_UARTA0=0
#endif //__MSP432P401R__

/* Performs all necessary board initialization in ble.begin(). */
void initBoard(void);

#endif
