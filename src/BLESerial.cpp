
#include <BLE.h>
#include "BLESerial.h"

uint8_t rxBuffer[BLE_SERIAL_BUFFER_SIZE] = {0};
volatile uint16_t rxWriteIndex = 0;
volatile uint16_t rxReadIndex = 0;

/* Nordic Semiconductor's UART Service */

BLE_Char rxChar =
{
  {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
   0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E},
  BLE_WRITABLE,
  "Client TX"
};

BLE_Char txChar =
{
  {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
   0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E},
  BLE_READABLE | BLE_NOTIFIABLE,
  "Client RX"
};

BLE_Char *serialServiceChars[] = {&rxChar, &txChar};


//6e400001-b5a3-f393-e0a9-e50e24dcca9e
BLE_Service serialService =
{
  {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
   0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E},
  2, serialServiceChars
};

int BLE::available(void)
{
  int numChars = 0;
  numChars = (rxWriteIndex >= rxReadIndex) ?
    (rxWriteIndex - rxReadIndex)
    : BLE_SERIAL_BUFFER_SIZE - (rxReadIndex - rxWriteIndex);
  return numChars;
}

/* If avaialable, return character at current read index. */
int BLE::peek(void)
{
  int iChar = -1;
  if (available())
  {
    iChar = (int) rxBuffer[rxReadIndex];
  }
  return iChar;
}

/* peek() to check for character. If there is one, increment index and return. */
int BLE::read(void)
{
  int iChar = -1;
  iChar = peek();
  if (0 <= iChar)
  {
    rxReadIndex = (rxReadIndex + 1) % BLE_SERIAL_BUFFER_SIZE;
  }
  return iChar;
}

/*
 * Serial.flush is supposed to wait for outgoing data to transmit, but since
 * a write call operates in the same task all the way through the RPC, we
 * don't need to worry.
 */
void BLE::flush(void)
{
  /*
   * Essentially fast-forward the read index to the write index.
   * When rxReadIndex == rxWriteIndex, available() returns 0.
   */
  rxReadIndex = rxWriteIndex;
  return;
}

/* Write a single byte or an arra of bytes with the appropriate writeValue(). */
size_t BLE::write(uint8_t c)
{
  if (writeValue(&txChar, c) == BLE_SUCCESS)
  {
    return 1;
  }
  return 0;
}

size_t BLE::write(const uint8_t buffer[], size_t size)
{
  if (writeValue(&txChar, buffer, size) == BLE_SUCCESS)
  {
    return size;
  }
  return 0;
}

/* Called in the NPI task when the BLE client writes data. */
void BLESerial_clientWrite(uint16_t len, uint8_t *pData)
{
  /*
   * Can only write up to BLE_SERIAL_BUFFER_SIZE-1 because if the read and
   * write indices are the same, available() returns 0.
   */
  if (BLE_SERIAL_BUFFER_SIZE - 1 < len)
  {
    pData += len - BLE_SERIAL_BUFFER_SIZE - 1;
    len = BLE_SERIAL_BUFFER_SIZE - 1;
  }

  /*
   * Determine if this write will move the write index past the read index.
   * If true then move the read index to one past the write index.
   */
  bool overwritesReadIdx = false;
  /* Fits in buffer without wrapping. */
  if (len < BLE_SERIAL_BUFFER_SIZE - rxWriteIndex)
  {
    memcpy(&rxBuffer[rxWriteIndex], pData, len);
    /* Indicate that read index was overwritten. */
    if (rxWriteIndex < rxReadIndex && rxReadIndex < rxWriteIndex + len)
    {
      overwritesReadIdx = true;
    }
    rxWriteIndex += len;
  }
  /* Wraps end of buffer */
  else
  {
    /* Amount written to reach end of buffer. */
    uint16_t firstLen = BLE_SERIAL_BUFFER_SIZE - rxWriteIndex;
    /* Amount written from beginning of buffer. */
    uint16_t lastLen = len - firstLen;
    memcpy(&rxBuffer[rxWriteIndex], pData, firstLen);
    memcpy(&rxBuffer[0], pData + firstLen, lastLen);
    /* Indicate that read index was overwritten. */
    if (rxReadIndex < lastLen || rxWriteIndex < rxReadIndex)
    {
      overwritesReadIdx = true;
    }
    rxWriteIndex = lastLen;
  }
  if (overwritesReadIdx)
  {
    rxReadIndex = (rxWriteIndex + 1) % BLE_SERIAL_BUFFER_SIZE;
  }
}
