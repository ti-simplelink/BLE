
#include "BLELog.h"

#define ACQUIRE_TIMEOUT 50 // ms

/* Bit mask that determines what is logged. */
uint8_t logLevel = BLE_LOG_NONE;

/* Used to determine if caller is the Energia sketch's task. */
Task_Handle apTask = NULL;

/*
 * Prevents the NPI task from logging while the AP task is logging or has
 * locked logging (e.g. when the Energia user is able to use Serial calls).
 * Prevents the AP task from logging while the NPI task is logging.
 */
uint8_t apIsLogging = false;
uint8_t npiIsLogging = false;

/*
 * Indicates when the NPI task wants to log. If set when release is called,
 * yields to another task.
 */
volatile bool logLockReq = false;

/* The last log mode called; used for followup calls. */
uint8_t apLogLast = 0x00;
uint8_t npiLogLast = 0x00;

/* Determines whether  secondary log call matches log level of  main call. */
#define LOG_CHECK_MODE (logLevel & ((apTask == Task_self()) ? apLogLast : npiLogLast))

/*
 * Local Functions Declarations
 */
static void hexPrint(int num);
static void hexPrintBigEnd(const uint8_t buf[], uint16_t len);
static void hexPrintLitEnd(const uint8_t buf[], uint16_t len);
static bool logSetCheckMode(uint8_t mode);

/*
 * Global Functions
 */
void logSetAPTask(Task_Handle _apTask)
{
  apTask = _apTask;
}

/*
 * Log hex data accounting for endianness.
 * Example usages:
 * Print int in byte order with  logParam(..., &val, sizeof(val), true)
 * Print int with MSB first with logParam(..., &val, sizeof(val), false)
 */
void logParam(const char name[], const uint8_t buf[],
              uint16_t len, bool isBigEnd)
{
  if (LOG_CHECK_MODE)
  {
    Serial.print("  ");
    Serial.print(name);
    Serial.print(":0x");
    if (isBigEnd)
    {
      hexPrintBigEnd(buf, len);
    }
    else
    {
      hexPrintLitEnd(buf, len);
    }
    Serial.println();
  }
}

/* Log a number in decimal, hex, or binary. */
void logParam(const char name[], int value, int base)
{
  if (LOG_CHECK_MODE)
  {
    Serial.print("  ");
    Serial.print(name);
    Serial.print(":");
    if (base == HEX)
    {
      hexPrint(value);
      Serial.println();
    }
    else if (base == BIN)
    {
      Serial.print("0b");
      Serial.println(value, base);
    }
    else
    {
      Serial.println(value);
    }
  }
}

/* Log two strings. Primary use is a fixed description and a variable string. */
void logParam(const char name[], const char value[])
{
  if (LOG_CHECK_MODE)
  {
    Serial.print("  ");
    Serial.print(name);
    Serial.print(":");
    Serial.println(value);
  }
}

/* Log a string. */
void logParam(const char value[])
{
  if (LOG_CHECK_MODE)
  {
    Serial.print("  ");
    Serial.println(value);
  }
}

/*
 * Log a UUID in the standard format.
 * Array must be guaranteed to have 16 bytes.
 */
void logUUID(const uint8_t UUID[], uint8_t UUIDlen)
{
  if (LOG_CHECK_MODE)
  {
    Serial.print("  UUID:0x");
    if (UUIDlen == SNP_128BIT_UUID_SIZE)
    {
      hexPrintLitEnd(&UUID[12], 4);
      Serial.print("-");
      hexPrintLitEnd(&UUID[10], 2);
      Serial.print("-");
      hexPrintLitEnd(&UUID[8], 2);
      Serial.print("-");
      hexPrintLitEnd(&UUID[6], 2);
      Serial.print("-");
      hexPrintLitEnd(&UUID[0], 6);
    }
    else if (UUIDlen == SNP_16BIT_UUID_SIZE)
    {
      hexPrintLitEnd(&UUID[0], 2);
    }
    Serial.println();
  }
}

/* Log an error status code. */
void logError(uint8_t status)
{
  if (logSetCheckMode(BLE_LOG_ERRORS))
  {
    logAcquire();
    Serial.print("ERR ");
    hexPrint(status);
    Serial.println();
  }
}

/* Log an error message with a status code. */
void logError(const char msg[], uint8_t status)
{
  if (logSetCheckMode(BLE_LOG_ERRORS))
  {
    logAcquire();
    Serial.print("ERR ");
    hexPrint(status);
    Serial.print(":");
    Serial.println(msg);
  }
}

/* Log a message to indicate a remote procedure call. */
void logRPC(const char msg[])
{
  if (logSetCheckMode(BLE_LOG_RPCS))
  {
    logAcquire();
    Serial.print("RPC:");
    Serial.println(msg);
  }
}

/* TODO */
/* The first half of this function is a workaround for the double async in NPI. */
/* Log a message to indicate that an asynchronous message was received. */
bool advDataCnfPendedWorkaround = false;
void logAsync(const char name[], uint8_t cmd1)
{
  if (cmd1 == SNP_SET_ADV_DATA_CNF)
  {
    advDataCnfPendedWorkaround = !advDataCnfPendedWorkaround;
    // Log every other one of this type because it gets posted twice
    if (!advDataCnfPendedWorkaround)
    {
      return;
    }
  }
  if (logSetCheckMode(BLE_LOG_REC_MSGS))
  {
    logAcquire();
    Serial.print("Rec msg ");
    hexPrint(cmd1);
    Serial.print(":");
    Serial.println(name);
  }
}

/*
 * Log what action happened to a characteristic.
 * e.g. "Client writing char value"
 */
void logChar(const char action[])
{
  if (logSetCheckMode(BLE_LOG_CHARACTERISTICS))
  {
    logAcquire();
    Serial.print(action);
    Serial.println(" char value");
  }
}

/* Reset all the logging state variables. */
void logReset(void)
{
  logLevel = BLE_LOG_NONE;
  apIsLogging = 0;
  npiIsLogging = 0;
  apTask = NULL;
  logLockReq = false;
  apLogLast = 0x00;
  npiLogLast = 0x00;
}

/*
 * Local Functions
 */
/* Log a hex number with zero padding to make it two digits. */
static void hexPrint(int num)
{
  Serial.print("0x");
  if (num < 0x10)
  {
    Serial.print("0");
  }
  Serial.print(num, HEX);
}

/* Print bytes big endian by printing start to end. Pad first byte. */
static void hexPrintBigEnd(const uint8_t buf[], uint16_t len)
{
  for (uint16_t i = 0; i < len; i++)
  {
    if (buf[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(buf[i], HEX);
  }
}

/* Print bytes little endian by printing end to start. Pad first byte. */
static void hexPrintLitEnd(const uint8_t buf[], uint16_t len)
{
  /* int16_t instead of uint16_t because overflow to 65535 screws up bounds. */
  for (int16_t i = len-1; i >= 0; i--)
  {
    if (buf[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(buf[i], HEX);
  }
}

/*
 * Set the log type of the most recent main logging function so that
 * secondary functions can work with any logLevel.
 * Check the logLevel bit mask to see if it should log.
 */
static bool logSetCheckMode(uint8_t mode)
{
  if (Task_self() == apTask)
  {
    apLogLast = mode;
  }
  else
  {
    npiLogLast = mode;
  }
  return logLevel & mode;
}

/*
 * Attempt to take ownership of logging for a set of log calls.
 */
void logAcquire(void)
{
  uint32_t startTime = millis();
  if (Task_self() == apTask)
  {
    /* Wait for NPI task to not be logging, or timeout. */
    while ((npiIsLogging) &&
           (millis() - startTime < ACQUIRE_TIMEOUT))
    {
      Task_yield();
    }
    /* Acquire AP log lock. */
    apIsLogging++;
  }
  else
  {
    /* Indicate to AP that the NPI task wants to log. */
    logLockReq = true;
    /* Wait for AP task to not be have ownership of logging, or timeout. */
    while ((apIsLogging) &&
           (millis() - startTime < ACQUIRE_TIMEOUT))
    {
      Task_yield();
    }
    /* Acquire NPI log lock. */
    npiIsLogging++;
  }
}

void logRelease(void)
{
  if (Task_self() == apTask)
  {
    /* In case a logRelease is called without a matching logAcquire. */
    if (apIsLogging)
    {
      apIsLogging--;
    }
    /* Continually yeild to NPI task if it wants to log. */
    if (logLockReq)
    {
      uint32_t startTime = millis();
      bool apIsLoggingSaved = apIsLogging;
      while (logLockReq && (millis() - startTime < ACQUIRE_TIMEOUT))
      {
        apIsLogging = 0;
        Task_yield();
        apIsLogging = apIsLoggingSaved;
      }
    }
  }
  else
  {
    /* In case a logRelease is called without a matching logAcquire. */
    if (npiIsLogging)
    {
      npiIsLogging--;
    }
    /* Indicate to AP that the NPI task has finished logging. */
    logLockReq = false;
  }
}
