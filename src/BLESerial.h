
#ifndef BLESERIAL_H
#define BLESERIAL_H

#include "BLETypes.h"

extern BLE_Char txChar;
extern BLE_Char rxChar;
extern BLE_Service serialService;

void BLESerial_clientWrite(uint16_t len, uint8_t *pData);

#endif
