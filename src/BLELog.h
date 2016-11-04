
#ifndef BLE_LOG_H
#define BLE_LOG_H

#include "BLETypes.h"
#include <ti/sysbios/knl/Task.h>

extern uint8_t logLevel;

/*
 * Set the task given ability to lock out the other. This is called in
 * ble.begin() so the Energia user's code has priority for USB serial writes.
 */
void logSetAPTask(Task_Handle apTask);

/*
 * Main logging functions that will call logAcquire(). Each should call
 * logSetCheckMode() to set the most recent attempted log type for the
 * secondary functions below.
 */
void logError(uint8_t status);
void logError(const char msg[], uint8_t status);
void logRPC(const char msg[]);
void logAsync(const char name[], uint8_t cmd1);
void logChar(const char action[]);

/*
 * Secondary logging functions. Each should use the macro LOG_CHECK_MODE so
 * they only log if the previous main logging function logged. This way these
 * functions can be agnostic of log type and the log level check can be handled
 * by only the main logging functions.
 */
void logParam(const char name[], const uint8_t buf[], uint16_t len, bool isBigEnd=true);
void logParam(const char name[], int value, int base=DEC);
void logParam(const char name[], const char value[]);
void logParam(const char value[]);
void logUUID(const uint8_t UUID[], uint8_t len);

/*
 * Wrap a set of logging calls with logAcquire and logRelease to ensure they
 * are not interleaved with other logging calls. All of the main logging
 * functions call these by default. logRelease() must be manually called once
 * for every main logging call.
 */
void logAcquire(void);
void logRelease(void);

/* Used in ble.end(). */
void logReset(void);

#endif
