
#include <string.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/drivers/UART.h>

#include "ti/sap/sap.h"
#include "ti/sap/snp.h"
#include "ti/sap/snp_rpc.h"

#include <BLE.h>
#include "BLEBoard.h"
#include "BLEEventHandling.h"
#include "BLELog.h"
#include "BLESerial.h"
#include "BLEServiceList.h"

/* So the user doesn't have to call the BLE constructor. */
BLE ble = BLE();

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8_t defNotConnAD[] =
{
  0x02,   // length of this data
  SAP_GAP_ADTYPE_FLAGS,
  SAP_GAP_ADTYPE_FLAGS_GENERAL | SAP_GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // Manufacturer specific advertising data
  0x06,
  0xFF, //GAP_ADTYPE_MANUFACTURER_SPECIFIC,
  LO_UINT16(TI_COMPANY_ID),
  HI_UINT16(TI_COMPANY_ID),
  TI_ST_DEVICE_ID,
  TI_ST_KEY_DATA_ID,
  0x00                                    // Key state
};

/*
 * The SNP will automatically use the non-connectable advertisement data
 * if the connectable advertisement data is not set.
 */
static uint8_t defConnAD[0] = {};

static uint8_t defScanRspData[] = {
  // complete name
  0xc,// length of this data
  SAP_GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'E', 'n', 'e', 'r', 'g', 'i', 'a', ' ',
  'B', 'L', 'E',

  // connection interval range
  0x05,   // length of this data
  0x12, //GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( BLE_DEF_DESIRED_MIN_CONN_INT ),
  HI_UINT16( BLE_DEF_DESIRED_MIN_CONN_INT ),
  LO_UINT16( BLE_DEF_DESIRED_MAX_CONN_INT ),
  HI_UINT16( BLE_DEF_DESIRED_MAX_CONN_INT ),

  // Tx power level
  0x02,   // length of this data
  0x0A, //GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

/*
 * These three arrays and advertIndex() are used to set default
 * advertisement data.
 */
static uint8_t *defADArr[] =
{
  defNotConnAD,
  defConnAD,
  defScanRspData
};

static size_t defADSizes[] =
{
  sizeof(defNotConnAD),
  sizeof(defConnAD),
  sizeof(defScanRspData)
};

static uint8_t aDIdxToType[] =
{
  BLE_ADV_DATA_NOTCONN,
  BLE_ADV_DATA_CONN,
  BLE_ADV_DATA_SCANRSP
};

static uint8_t advertIndex(uint8_t advertType);

/* Constructor. portType defaults to UART. */
BLE::BLE(byte portType)
{
  _portType = portType;
  for (uint8_t idx = 0; idx < MAX_ADVERT_IDX; idx++) {advertDataArr[idx] = NULL;}
  resetPublicMembers();
}

int BLE::begin(void)
{
  /* Do board specific initializations */
  initBoard();

  apEvent = Event_create(NULL, NULL);
  logSetAPTask(Task_self());

  SAP_Params sapParams;
  SAP_initParams(_portType, &sapParams);
  sapParams.port.remote.boardID = BLE_UART_ID;
  sapParams.port.remote.mrdyPinID = BLE_Board_MRDY;
  sapParams.port.remote.srdyPinID = BLE_Board_SRDY;
  logRPC("Opening SAP");
  logRelease();
  if (isError(SAP_open(&sapParams)))
  {
    SAP_close();
    return BLE_CHECK_ERROR;
  }

  /*
   * Register callback to receive asynchronous requests from the NP.
   * This must be called before using any other calls to SAP, except
   * those above. This function may be called multiple times to
   * register multiple callbacks. Runs in NPI task.
   */
  if (isError(SAP_setAsyncCB(AP_asyncCB)))
  {
    SAP_close();
    return BLE_CHECK_ERROR;
  }

  /*
   * Register async SNP event handler. Includes connection establisment,
   * termination, advertisement enabling, security events.
   */
  if (isError(SAP_registerEventCB(processSNPEventCB, 0xFFFF)))
  {
    SAP_close();
    return BLE_CHECK_ERROR;
  }

  /*
   * Give the NP time to send a power up indicator, if we receive one then
   * we can assume the NP has just started and we don't need to send a reset
   * otherwise, we can assume the NP was running previously and needs to be
   * reset to a known state
   */
  if (!apEventPend(AP_EVT_PUI)) {
    // Assuming that at SAP start up that SNP is already running
    logRPC("Reseting SNP");
    logRelease();
    if (isError(SAP_reset()))
    {
      SAP_close();
      return BLE_CHECK_ERROR;
    }
    if (!apEventPend(AP_EVT_PUI))
    {
      return BLE_CHECK_ERROR;
    }
  }

  return BLE_SUCCESS;
}

void BLE::end(void)
{
  /* Reset private members of BLE.h */
  for (uint8_t idx = 0; idx < MAX_ADVERT_IDX; idx++) {advertDataArr[idx] = NULL;}

  resetPublicMembers();

  BLE_clearServices();
  flush();
  logReset();
  Event_delete(&apEvent);
  free(asyncRspData);
  asyncRspData = NULL;
  memset(&eventHandlerData, 0, sizeof(eventHandlerData));
  _connHandle = -1;
  connected = false;
  advertising = false;
  SAP_close();
}

int BLE::resetPublicMembers(void)
{
  error = BLE_SUCCESS;
  opcode = 0;
  memset(&usedConnParams, 0, sizeof(usedConnParams));
  memset(&bleAddr, 0, sizeof(bleAddr));
  authKey = 0;
  mtu = 20;
  displayStringFxn = NULL;
  displayUIntFxn = NULL;
  return BLE_SUCCESS;
}

int BLE::terminateConn(void)
{
  logRPC("Terminate connection");
  logRelease();
  if ((!connected && isError(BLE_NOT_CONNECTED)) ||
      isError(SAP_setParam(SAP_PARAM_CONN, SAP_CONN_STATE,
                           sizeof(_connHandle), (uint8_t *) &_connHandle)) ||
      !apEventPend(AP_EVT_CONN_TERM))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

bool BLE::isConnected(void)
{
  return connected;
}

bool BLE::isAdvertising(void)
{
  return advertising;
}

int BLE::addService(BLE_Service *bleService)
{
  if (isError(BLE_registerService(bleService)))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

/* Converts advertisement type defines to indices for the array of data. */
static uint8_t advertIndex(uint8_t advertType)
{
  switch (advertType)
  {
    case BLE_ADV_DATA_NOTCONN:
      return 0;
    case BLE_ADV_DATA_CONN:
      return 1;
    case BLE_ADV_DATA_SCANRSP:
      return 2;
  }
  return BLE_INVALID_PARAMETERS;
}

/* Initialize advertisement data not set by user to defaults. */
uint8_t BLE::advertDataInit(void)
{
  for (uint8_t idx = 0; idx < MAX_ADVERT_IDX; idx++)
  {
    if (advertDataArr[idx] == NULL && defADArr[idx] != NULL && defADSizes[idx])
    {
      if (isError(setAdvertData(aDIdxToType[idx], defADSizes[idx],
                                defADArr[idx])))
      {
        return BLE_CHECK_ERROR;
      }
    }
  }
  return BLE_SUCCESS;
}

/* Initialize default advertisement data and start advertising. */
int BLE::startAdvert(BLE_Advert_Settings *advertSettings)
{
  logRPC("Start adv");
  logRelease();
  if ((advertising && isError(BLE_ALREADY_ADVERTISING)) ||
      isError(advertDataInit()))
  {
    return BLE_CHECK_ERROR;
  }

  uint16_t reqSize;
  uint8_t *pData;
  /* Declare these outside the if statements so they're in-scope
     for SAP_setParam. */
  uint8_t enableAdv = SAP_ADV_STATE_ENABLE;
  snpStartAdvReq_t lReq;
  if (advertSettings == NULL)
  {
    reqSize = 1;
    pData = &enableAdv;
  }
  else
  {
    lReq.type = advertSettings->advertMode;
    lReq.timeout = advertSettings->timeout;
    lReq.interval = advertSettings->interval;
    lReq.behavior = advertSettings->connectedBehavior;
    reqSize = (uint16_t) sizeof(lReq);
    pData = (uint8_t *) &lReq;
  }
  if (isError(SAP_setParam(SAP_PARAM_ADV, SAP_ADV_STATE, reqSize, pData)) ||
      !apEventPend(AP_EVT_ADV_ENB))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

int BLE::stopAdvert(void)
{
  uint8_t disableAdv = SAP_ADV_STATE_DISABLE;
  logRPC("End adv");
  logRelease();
  if ((!advertising && isError(BLE_NOT_ADVERTISING)) ||
      isError(SAP_setParam(SAP_PARAM_ADV, SAP_ADV_STATE, 1, &disableAdv)) ||
      !apEventPend(AP_EVT_ADV_END))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

int BLE::setAdvertData(uint8_t advertType, uint8_t len, uint8_t *advertData)
{
  logRPC("Set adv data");
  logParam("Type", advertType);
  logRelease();
  if (isError(SAP_setParam(SAP_PARAM_ADV, advertType, len, advertData)) ||
      !apEventPend(AP_EVT_ADV_DATA_RSP))
  {
    return BLE_CHECK_ERROR;
  }
  // advertType validated by SAP_setParam
  uint8_t idx = advertIndex(advertType);
  advertDataArr[idx] = advertData;
  return BLE_SUCCESS;
}

/* Used to free advertisement data malloced by the user. */
uint8_t* BLE::getAdvertData(uint8_t advertType)
{
  ble.error = BLE_SUCCESS;
  uint8_t idx = advertIndex(advertType);
  if (idx < MAX_ADVERT_IDX && advertDataArr[idx] != defADArr[idx])
  {
    return advertDataArr[idx];
  }
  else
  {
    ble.error = idx;
    return NULL;
  }
}

/*
 * Uses the default scan response data defScanRspData. The first
 * byte is one plus the length of the name. The second should be
 * SAP_GAP_ADTYPE_LOCAL_NAME_COMPLETE, and the third and so on are the
 * characters of the name.
 */
int BLE::setAdvertName(uint8_t advertNameLen, const char advertName[])
{
  uint8_t newSize = sizeof(defScanRspData) - defScanRspData[0]
                  + 1 + advertNameLen;
  uint8_t *newData = (uint8_t *) malloc(newSize * sizeof(*newData));
  newData[0] = 1 + advertNameLen;
  newData[1] = defScanRspData[1];
  strcpy((char *) &newData[2], advertName);
  uint8_t *destAfterStr = newData + 2 + advertNameLen;
  uint8_t *srcAfterStr = defScanRspData + 1 + defScanRspData[0];
  uint8_t afterStrLen = sizeof(defScanRspData) - 1 - defScanRspData[0];
  memcpy(destAfterStr, srcAfterStr, afterStrLen);


  /* Set Device Name, this improves user experience with iOS devices */
  SAP_setServiceParam(SNP_GGS_SERV_ID, SNP_GGS_DEVICE_NAME_ATT,
                         advertNameLen, (uint8_t *)advertName);
  logRPC("Set adv name");
  logParam(advertName);
  logRelease();
  return setAdvertData(BLE_ADV_DATA_SCANRSP, newSize, newData);
}

int BLE::setAdvertName(const char advertName[])
{
  return setAdvertName(strlen(advertName), advertName);
}

int BLE::setAdvertName(String *advertName)
{
  uint8_t len = (*advertName).length();
  return setAdvertName(len, (*advertName).c_str());
}

/* Uses macros from sap.h and snp.h. */
int BLE::setGattParam(uint8_t serviceId, uint8_t charId,
                      uint16_t len, uint8_t *pData)
{
  logRPC("Set GATT param");
  logRelease();
  return SAP_setServiceParam(serviceId, charId, len, pData);
}

/* Uses macros from sap.h and snp.h. */
int BLE::getGattParam(uint8_t serviceId, uint8_t charId,
                      uint16_t *len, uint8_t *pData)
{
  logRPC("Get GATT param");
  logRelease();
  return SAP_getServiceParam(serviceId, charId, len, pData);
}

/* Uses macros from sap.h and snp.h. */
int BLE::setGapParam(uint16_t paramId, uint16_t value)
{
  logRPC("Set GAP param");
  logParam("Param ID", paramId);
  logParam("Value", value);
  logRelease();
  return SAP_setParam(SAP_PARAM_GAP, paramId,
                      sizeof(value), (uint8_t *) &value);
}

/* Uses macros from sap.h and snp.h. */
int BLE::getGapParam(uint16_t paramId, uint16_t *value)
{
  logRPC("Get GAP param");
  logParam("Param ID", paramId);
  logRelease();
  return SAP_setParam(SAP_PARAM_GAP, paramId,
                      sizeof(*value), (uint8_t *) value);
}

/* Uses macros from sap.h and snp.h. */
uint8_t *BLE::hciCommand(uint16_t opcode, uint16_t len, uint8_t *pData)
{
  logRPC("HCI cmd");
  logParam("Opcode", opcode);
  logRelease();
  if (isError(SAP_getParam(SAP_PARAM_HCI, opcode, len, pData)) ||
      !apEventPend(AP_EVT_HCI_RSP))
  {
    return NULL;
  }
  snpHciCmdRsp_t hciCmdRsp;
  memcpy(&hciCmdRsp, asyncRspData, sizeof(hciCmdRsp));
  Event_post(apEvent, AP_EVT_COPIED_ASYNC_DATA);
  return hciCmdRsp.pData; // TODO: does this get deallocated in NPI task?
}

/*
 * Must be currently in a connection.
 */
int BLE::setConnParams(BLE_Conn_Params_Update_Req *connParams)
{
  logRPC("Conn params req");
  logParam("intervalMin", connParams->intervalMin);
  logParam("intervalMax", connParams->intervalMax);
  logParam("slaveLatency", connParams->slaveLatency);
  logParam("supervisionTimeout", connParams->supervisionTimeout);
  logRelease();
  if ((!connected && isError(BLE_NOT_CONNECTED)) ||
      isError(SAP_setParam(SAP_PARAM_CONN, SAP_CONN_PARAM,
                           sizeof(*connParams), (uint8_t *) connParams)) ||
      !apEventPend(AP_EVT_CONN_PARAMS_CNF))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

int BLE::setSingleConnParam(size_t offset, uint16_t value)
{
  BLE_Conn_Params_Update_Req paramsReq;
  paramsReq.connHandle         = _connHandle;
  paramsReq.intervalMin        = SNP_CONN_INT_MIN;
  paramsReq.intervalMax        = SNP_CONN_INT_MAX;
  paramsReq.slaveLatency       = usedConnParams.slaveLatency;
  paramsReq.supervisionTimeout = usedConnParams.supervisionTimeout;
  *(uint16_t *)(((char *) &paramsReq) + offset) = value;
  return setConnParams(&paramsReq);
}

int BLE::setMinConnInt(uint16_t intervalMin)
{
  return setSingleConnParam(offsetof(BLE_Conn_Params_Update_Req, intervalMin),
                            intervalMin);
}

int BLE::setMaxConnInt(uint16_t intervalMax)
{
  return setSingleConnParam(offsetof(BLE_Conn_Params_Update_Req, intervalMax),
                            intervalMax);
}

int BLE::setRespLatency(uint16_t slaveLatency)
{
  return setSingleConnParam(offsetof(BLE_Conn_Params_Update_Req, slaveLatency),
                            slaveLatency);
}

int BLE::setBleTimeout(uint16_t supervisionTimeout)
{
  return setSingleConnParam(offsetof(BLE_Conn_Params_Update_Req, supervisionTimeout),
                            supervisionTimeout);
}

/* Helper function for AP characteristic writes. */
int BLE::apCharWriteValue(BLE_Char *bleChar, void *pData,
                          size_t size, bool isBigEnd=true)
{
  logChar("App writing");
  BLE_charWriteValue(bleChar, pData, size, isBigEnd);
  logRelease();
  return writeNotifInd(bleChar);
}

/* Helper function to handle notifications and indications. */
uint8_t BLE::writeNotifInd(BLE_Char *bleChar)
{
  uint8_t status = BLE_SUCCESS;
  if (bleChar->_CCCD)
  {
    snpNotifIndReq_t localReq;
    localReq.connHandle = _connHandle;
    localReq.attrHandle = bleChar->_handle;
    localReq.authenticate = 0;
    if (bleChar->_CCCD & SNP_GATT_CLIENT_CFG_NOTIFY)
    {
      localReq.type = SNP_SEND_NOTIFICATION;
      logRPC("Sending notif");
    }
    else if (bleChar->_CCCD & SNP_GATT_CLIENT_CFG_INDICATE)
    {
      localReq.type = SNP_SEND_INDICATION;
      logRPC("Sending ind");
    }
    logParam("Total bytes", bleChar->_valueLen);
    uint16_t sent = 0;
    /* Send at least one notification, in case data is 0 length. */
    do
    {
      /* Send at most ble.mtu per packet. */
      uint16_t size = MIN(bleChar->_valueLen - sent, mtu);
      localReq.pData = ((uint8_t *) bleChar->_value) + sent;
      logParam("Sending", size);
      if (isError(SNP_RPC_sendNotifInd(&localReq, size)))
      {
        status = BLE_CHECK_ERROR;
        break;
      }
      // Only pend for confirmation of indication
      if ((bleChar->_CCCD & SNP_GATT_CLIENT_CFG_INDICATE) &&
               !apEventPend(AP_EVT_NOTIF_IND_RSP))
      {
        status = BLE_CHECK_ERROR;
        break;
      }
      sent += size;
    } while (sent < bleChar->_valueLen);
  }
  logRelease();
  return status;
}

int BLE::writeValue(BLE_Char *bleChar, bool value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, char value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, unsigned char value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, int value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, unsigned int value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, long value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, unsigned long value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), false);
}

int BLE::writeValue(BLE_Char *bleChar, float value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), true);
}

int BLE::writeValue(BLE_Char *bleChar, double value)
{
  return apCharWriteValue(bleChar, (uint8_t *) &value, sizeof(value), true);
}

int BLE::writeValue(BLE_Char *bleChar, const uint8_t buf[], int len)
{
  return apCharWriteValue(bleChar, (uint8_t *) buf, (len)*sizeof(*buf), true);
}

/*
 * Use buffer of size len+1 so the null-termination is stored. This way the
 * stored strings match the functionality of strcpy, which copies it.
 */
int BLE::writeValue(BLE_Char *bleChar, const char str[], int len)
{
  int ret = apCharWriteValue(bleChar, (uint8_t *) str, (len+1)*sizeof(*str), true);
  logChar("  String");
  logParam(str);
  logRelease();
  return ret;
}

int BLE::writeValue(BLE_Char *bleChar, const char str[])
{
  return writeValue(bleChar, str, strlen(str));
}

int BLE::writeValue(BLE_Char *bleChar, String *str)
{
  int len = (*str).length();
  return writeValue(bleChar, (*str).c_str(), len);
}

/* Helper function to validate the size of the read data. */
uint8_t BLE::readValueValidateSize(BLE_Char *bleChar, size_t size)
{
  uint8_t status = BLE_SUCCESS;
  logChar("App reading");
  logParam("Handle", bleChar->_handle);
  if (bleChar->_valueLen != size)
  {
    logParam("Invalid size");
    logParam("Have", bleChar->_valueLen);
    logParam("Want", size);
    error = BLE_UNDEFINED_VALUE;
    status = BLE_CHECK_ERROR;
  }
  else
  {
    logParam("Size in bytes", size);
    logParam("Value", (const uint8_t *) bleChar->_value, size, bleChar->_isBigEnd);
  }
  logRelease();
  return status;
}

bool BLE::readValue_bool(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(bool));
  if (error == BLE_SUCCESS)
  {
    return *(bool *) bleChar->_value;
  }
  return 0;
}

char BLE::readValue_char(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(char));
  if (error == BLE_SUCCESS)
  {
    return *(char *) bleChar->_value;
  }
  return 0;
}

unsigned char BLE::readValue_uchar(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(unsigned char));
  if (error == BLE_SUCCESS)
  {
    return *(unsigned char *) bleChar->_value;
  }
  return 0;
}

int BLE::readValue_int(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(int));
  if (error == BLE_SUCCESS)
  {
    return *(int *) bleChar->_value;
  }
  return 0;
}

unsigned int BLE::readValue_uint(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(unsigned int));
  if (error == BLE_SUCCESS)
  {
    return *(unsigned int *) bleChar->_value;
  }
  return 0;
}

long BLE::readValue_long(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(long));
  if (error == BLE_SUCCESS)
  {
    return *(long *) bleChar->_value;
  }
  return 0;
}

unsigned long BLE::readValue_ulong(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(unsigned long));
  if (error == BLE_SUCCESS)
  {
    return *(unsigned long *) bleChar->_value;
  }
  return 0;
}

float BLE::readValue_float(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(float));
  if (error == BLE_SUCCESS)
  {
    return *(float *) bleChar->_value;
  }
  return 0;
}

double BLE::readValue_double(BLE_Char *bleChar)
{
  error = readValueValidateSize(bleChar, sizeof(double));
  if (error == BLE_SUCCESS)
  {
    return *(double *) bleChar->_value;
  }
  return 0;
}

uint8_t* BLE::readValue_uint8_t(BLE_Char *bleChar, int *len)
{
  *len = bleChar->_valueLen;
  logChar("App reading");
  logParam("Handle", bleChar->_handle);
  logParam("Buffer length", *len);
  logParam("Buffer contents", (const uint8_t *) bleChar->_value, *len, true);
  logRelease();
  return (uint8_t *) bleChar->_value;
}

char* BLE::readValue_charArr(BLE_Char *bleChar)
{
  int len = bleChar->_valueLen;
  logChar("App reading");
  logParam("Handle", bleChar->_handle);
  logParam("String length", len);
  /* Convert value to null-termiated string, if not already */
  if (((char *) bleChar->_value)[len-1] != '\0')
  {
    bleChar->_value = realloc(bleChar->_value, (len+1)*sizeof(char));
    ((char *) bleChar->_value)[len] = '\0';
  }
  logParam("As string", (const char *) bleChar->_value);
  logRelease();
  return (char *) bleChar->_value;
}

/* Returns object by value instead of reference so the Energia user
   doesn't have to care about deallocating the object. */
String BLE::readValue_String(BLE_Char *bleChar)
{
  char *buf = readValue_charArr(bleChar);
  String str;
  if (buf)
  {
    str = String(buf);
  }
  else
  {
    str = String();
  }
  return str;
}

void BLE::setValueFormat(BLE_Char *bleChar, uint8_t valueFormat,
                         int8_t valueExponent)
{
  bleChar->_valueFormat = valueFormat;
  bleChar->_valueExponent = valueExponent;
}

/* Uses macros from sap.h and snp.h. */
int BLE::setSecurityParam(uint16_t paramId, uint16_t len, uint8_t *pData)
{
  logRPC("Set sec param");
  logParam("ParamId", paramId, len);
  logRelease();
  if (isError(SAP_setParam(SAP_PARAM_SECURITY, paramId, len, pData)))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

int BLE::setPairingMode(uint8_t pairingMode)
{
  return setSecurityParam(SAP_SECURITY_BEHAVIOR, 1, &pairingMode);
}

int BLE::setIoCapabilities(uint8_t param)
{
  return setSecurityParam(SAP_SECURITY_IOCAPS, 1, &param);
}

int BLE::useBonding(bool param)
{
  return setSecurityParam(SAP_SECURITY_BONDING, 1, (uint8_t*) &param);
}

int BLE::eraseAllBonds(void)
{
  return setSecurityParam(SAP_ERASE_ALL_BONDS, 0, NULL);
}

int BLE::replaceLruBond(bool param)
{
  return setSecurityParam(SAP_ERASE_LRU_BOND, 1, (uint8_t*) &param);
}

int BLE::sendSecurityRequest(void)
{
  logRPC("Send sec req");
  logRelease();
  return SAP_sendSecurityRequest();
}

int BLE::useWhiteListPolicy(bool useWhiteList)
{
  logRPC("Use whitelist policy");
  logParam("policy", useWhiteList);
  uint8_t useWhiteListInt = (uint8_t)useWhiteList;
  logRelease();
  if (isError(SAP_setParam(SAP_PARAM_WHITELIST, 0, 0, &useWhiteListInt)) ||
      !apEventPend(AP_EVT_WHITE_LIST_RSP))
  {
    return BLE_CHECK_ERROR;
  }
  return BLE_SUCCESS;
}

unsigned int BLE::getRand(void)
{
  return SAP_getRand();
}

/*
 * Rarely used and advanced-use calls so we won't provide a framework for this.
 */
void BLE::getRevision(BLE_Get_Revision_Rsp *getRevisionRsp)
{
  logRPC("Get revision");
  logRelease();
  SAP_getRevision(getRevisionRsp);
}

void BLE::getStatus(BLE_Get_Status_Rsp *getStatusRsp)
{
  logRPC("Get status");
  logRelease();
  SAP_getStatus(getStatusRsp);
}

int BLE::testCommand(BLE_Test_Command_Rsp *testRsp)
{
  logRPC("Test cmd");
  logRelease();
  SAP_testCommand(); // void function
  if (!apEventPend(AP_EVT_TEST_RSP))
  {
    return BLE_CHECK_ERROR;
  }
  memcpy(testRsp, asyncRspData, sizeof(*testRsp));
  Event_post(apEvent, AP_EVT_COPIED_ASYNC_DATA);
  return BLE_SUCCESS;
}


int BLE::serial(void)
{
  if (isError(addService(&serialService)))
  {
    return BLE_CHECK_ERROR;
  }
  uint8_t advertData[] = {
    0x02,                 // Length of field
    0x01,                 //GAP_ADTYPE_FLAGS
    0x06,                 //GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    0x11,                 // length of this data
    0x06,                 // GAP_ADTYPE_128BIT_MORE
    0x00,0x00,0x00,0x00,  // Serial SVC UUID placeholder (128 bit UUID)
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00
  };

  // Copy the UUID into advert data
  memcpy(&advertData[5], &(serialService.UUID), serialService._UUIDlen);
  // Set advert data, we are setting this so we advertise the UART service
  // This is done for compatibility with the Adafruit Bluefruit app
  if (isError(setAdvertData(SAP_ADV_DATA_NOTCONN, sizeof(advertData), advertData)))
  {
    return BLE_CHECK_ERROR;
  }

  rxChar._isBigEnd = true;
  txChar._isBigEnd = true;
  return BLE_SUCCESS;
}

void BLE::setLogLevel(uint8_t newLogLevel)
{
  logLevel = newLogLevel;
}
