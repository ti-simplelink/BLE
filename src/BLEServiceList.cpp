
#include "ti/sap/sap.h"
#include "ti/sap/snp.h"
#include "ti/sap/snp_rpc.h"

#include "BLELog.h"
#include "BLESerial.h"
#include "BLEServiceList.h"

typedef struct BLE_Service_Node
{
  BLE_Service_Node *next;
  BLE_Service *service;
} BLE_Service_Node;

BLE_Service_Node *bleServiceListHead = NULL;
BLE_Service_Node *bleServiceListTail = NULL;

static void addServiceNode(BLE_Service *service);
static BLE_Char* getChar(uint16_t handle);
static BLE_Char* getCCCD(uint16_t handle);
static BLE_Service* getServiceWithChar(uint16_t handle);
static void constructService(SAP_Service_t *service, BLE_Service *bleService);
static void constructChar(SAP_Char_t *sapChar, BLE_Char *bleChar);
static uint8_t setPermissions(uint8_t props);
static uint8_t getUUIDLen(const uint8_t UUID[]);
static uint8_t serviceReadAttrCB(void *context,
                                 uint16_t connectionHandle,
                                 uint16_t charHdl, uint16_t offset,
                                 uint16_t maxSize, uint16_t *len,
                                 uint8_t *pData);
static uint8_t serviceWriteAttrCB(void *context,
                                  uint16_t connectionHandle,
                                  uint16_t charHdl, uint16_t len,
                                  uint8_t *pData);
static uint8_t serviceCCCDIndCB(void *context,
                                uint16_t connectionHandle,
                                uint16_t cccdHdl, uint8_t type,
                                uint16_t value);

int BLE_registerService(BLE_Service *bleService)
{
  SAP_Service_t *service = (SAP_Service_t *) malloc(sizeof(*service));
  constructService(service, bleService);
  logAcquire();
  logRPC("Register service");
  logUUID(bleService->UUID, bleService->_UUIDlen);
  int status = SAP_registerService(service);
  if (status != SNP_FAILURE) {
    bleService->_handle = service->serviceHandle;
    logParam("Handle", bleService->_handle);
    for (uint8_t i = 0; i < bleService->numChars; i++)
    {
      bleService->chars[i]->_handle = service->charAttrHandles[i].valueHandle;
      logParam("With characteristic");
      logUUID(bleService->chars[i]->UUID, bleService->chars[i]->_UUIDlen);
      logParam("Handle", bleService->chars[i]->_handle);
      bleService->chars[i]->_CCCDHandle = service->charAttrHandles[i].cccdHandle;
      if (service->charTable[i].pUserDesc)
      {
        free(service->charTable[i].pUserDesc);
      }
      if (service->charTable[i].pCccd)
      {
        free(service->charTable[i].pCccd);
      }
      if (service->charTable[i].pFormat)
      {
        free(service->charTable[i].pFormat);
      }
    }
    addServiceNode(bleService);
  }
  logRelease();
  free(service->charTable);
  free(service->charAttrHandles);
  free(service);
  return status;
}

void BLE_resetCCCD(void)
{
  BLE_Service_Node *curr = bleServiceListHead;
  while (curr)
  {
    for (uint8_t i = 0; i < curr->service->numChars; i++)
    {
      curr->service->chars[i]->_CCCD = 0;
    }
    curr = curr->next;
  }
}

void BLE_charWriteValue(BLE_Char *bleChar, void *pData, size_t size, bool isBigEnd)
{
  logParam("Handle", bleChar->_handle);
  logParam("Size in bytes", bleChar->_valueLen);
  if (bleChar->_valueLen != size && bleChar->_value)
  {
    free(bleChar->_value);
    bleChar->_value = NULL;
  }
  if (bleChar->_value == NULL)
  {
    logParam("New size in bytes", size);
    bleChar->_value = (void *) malloc(size);
    bleChar->_isBigEnd = isBigEnd;
    bleChar->_valueLen = size;
  }
  memcpy((uint8_t *) bleChar->_value, pData, size);
  logParam("Value", (const uint8_t *) bleChar->_value, size, isBigEnd);
}

static void addServiceNode(BLE_Service *service)
{
  BLE_Service_Node *newNode = (BLE_Service_Node *) malloc(sizeof(*newNode));
  newNode->next = NULL;
  newNode->service = service;
  if (bleServiceListHead == NULL)
  {
    bleServiceListHead = newNode;
    bleServiceListTail = newNode;
  }
  else
  {
    bleServiceListTail->next = newNode;
    bleServiceListTail = newNode;
  }
}

static BLE_Char* getChar(uint16_t handle)
{
  BLE_Service *service = getServiceWithChar(handle);
  for (uint8_t i = 0; i < service->numChars; i++)
  {
    if (service->chars[i]->_handle == handle)
    {
      return service->chars[i];
    }
  }
  return NULL;
}

static BLE_Char* getCCCD(uint16_t handle)
{
  BLE_Service *service = getServiceWithChar(handle);
  for (uint8_t i = 0; i < service->numChars; i++)
  {
    if (service->chars[i]->_CCCDHandle == handle)
    {
      return service->chars[i];
    }
  }
  return NULL;
}

static BLE_Service* getServiceWithChar(uint16_t handle)
{
  BLE_Service_Node *curr = bleServiceListHead;
  while (curr->next && curr->next->service->_handle <= handle)
  {
    curr = curr->next;
  } // At end of loop, curr->service->_handle <= handle < curr-next->service->_handle
  return curr->service;
}

static void constructService(SAP_Service_t *service, BLE_Service *bleService)
{
  bleService->_handle         = 0;
  bleService->_UUIDlen        = getUUIDLen(bleService->UUID);
  service->serviceUUID.len    = bleService->_UUIDlen;
  service->serviceUUID.pUUID  = bleService->UUID;
  service->serviceType        = SNP_PRIMARY_SERVICE;
  service->charTableLen       = bleService->numChars; // sizeof with static array?
  service->charTable          = (SAP_Char_t *) malloc(service->charTableLen *
                                                    sizeof(*service->charTable));
  service->context            = NULL;
  service->charReadCallback   = serviceReadAttrCB;
  service->charWriteCallback  = serviceWriteAttrCB;
  service->cccdIndCallback    = serviceCCCDIndCB;
  service->charAttrHandles    = (SAP_CharHandle_t *) malloc(service->charTableLen *
                                                            sizeof(*service->charAttrHandles));
  for (uint8_t i = 0; i < bleService->numChars; i++)
  {
    constructChar(&service->charTable[i], bleService->chars[i]);
  }
}

static uint8_t setPermissions(uint8_t props)
{

  uint8_t permissions = 0;
  if(props & BLE_ENCRYPT)
  {
    permissions =   ((props & BLE_READABLE)
                        ? SNP_GATT_PERMIT_ENCRYPT_READ : 0)
                    | ((props & (BLE_WRITABLE_NORSP | BLE_WRITABLE))
                        ? SNP_GATT_PERMIT_ENCRYPT_WRITE : 0);
  }
  else if(props & BLE_AUTHEN)
  {
    permissions =   ((props & BLE_READABLE)
                        ? SNP_GATT_PERMIT_AUTHEN_READ : 0)
                    | ((props & (BLE_WRITABLE_NORSP | BLE_WRITABLE))
                        ? SNP_GATT_PERMIT_AUTHEN_WRITE : 0);
  }
  else
  {
    permissions =   ((props & BLE_READABLE)
                        ? SNP_GATT_PERMIT_READ : 0)
                    | ((props & (BLE_WRITABLE_NORSP | BLE_WRITABLE))
                        ? SNP_GATT_PERMIT_WRITE : 0);
  }
  return permissions;
}

static void constructChar(SAP_Char_t *sapChar, BLE_Char *bleChar)
{
  /* TODO remove this. Bug in BLE stack makes this fail otherwise. */
  bleChar->_valueFormat = 0;

  /* Initialize characteristic to have one byte with a value of 0.
     Override by calling writeValue in the main sketch. */
  bleChar->_value = calloc(1, 1);
  bleChar->_isBigEnd = true;
  bleChar->_valueLen = 1;

  /* Default to no notifications or indications. */
  bleChar->_CCCD = 0;
  bleChar->_CCCDHandle = 0;

  bleChar->_UUIDlen    = getUUIDLen(bleChar->UUID);
  sapChar->UUID.len    = bleChar->_UUIDlen;
  sapChar->UUID.pUUID  = bleChar->UUID;

  // Need to mask out the AUTHEN and ENCRYPT bits
  sapChar->properties  = (bleChar->properties & BLE_PROPERTIES_MASK);
  sapChar->permissions = setPermissions(bleChar->properties);

  if (bleChar->charDesc)
  {
    sapChar->pUserDesc = (SAP_UserDescAttr_t *) malloc(sizeof(*sapChar->pUserDesc));
    sapChar->pUserDesc->perms    = SNP_GATT_PERMIT_READ;
    uint16_t charStrLen = strlen(bleChar->charDesc) + 1; // +1 for null term
    sapChar->pUserDesc->maxLen   = charStrLen;
    sapChar->pUserDesc->initLen  = charStrLen;
    sapChar->pUserDesc->pDesc    = (uint8_t *) bleChar->charDesc;
  }
  else
  {
    sapChar->pUserDesc = NULL;
  }
  sapChar->pCccd = (SAP_UserCCCDAttr_t *) malloc(sizeof(*sapChar->pCccd));
  sapChar->pCccd->perms          = SNP_GATT_PERMIT_READ | SNP_GATT_PERMIT_WRITE;
  if (bleChar->_valueFormat)
  {
    sapChar->pFormat = (SAP_FormatAttr_t *) malloc(sizeof(*sapChar->pFormat));
    sapChar->pFormat->format     = bleChar->_valueFormat;
    sapChar->pFormat->exponent   = bleChar->_valueExponent;
    sapChar->pFormat->unit       = 0;
    sapChar->pFormat->name_space = 0;
    sapChar->pFormat->desc       = 0;
  }
  else
  {
    sapChar->pFormat   = NULL;
  }
  sapChar->pShortUUID  = NULL;
  sapChar->pLongUUID   = NULL;
}

/* Array guaranteed to have 16 bytes. */
static uint8_t getUUIDLen(const uint8_t UUID[])
{
  for (uint8_t i = SNP_16BIT_UUID_SIZE; i < SNP_128BIT_UUID_SIZE; i++)
  {
    if (UUID[i] != 0)
    {
      return SNP_128BIT_UUID_SIZE;
    }
  }
  return SNP_16BIT_UUID_SIZE;
}

static uint8_t serviceReadAttrCB(void *context,
                                 uint16_t connectionHandle,
                                 uint16_t charHdl, uint16_t offset,
                                 uint16_t maxSize, uint16_t *len,
                                 uint8_t *pData)
{
  (void) context;
  (void) connectionHandle;
  uint8_t status = SNP_SUCCESS;
  logAcquire();
  logChar("Client reading");
  BLE_Char *bleChar = getChar(charHdl);
  if (bleChar == NULL)
  {
    *len = 0;
    status = SNP_UNKNOWN_ATTRIBUTE;
    logError("Unknown handle", connectionHandle);
  }
  else if (bleChar->_valueLen <= offset)
  {
    logParam("Handle", bleChar->_handle);
    logParam("Offset too big");
    logParam("Value len", bleChar->_valueLen);
    logParam("Offset", offset);
    *len = 0;
  }
  else
  {
    logParam("Handle", bleChar->_handle);
    logParam("Offset", offset);
    uint8_t *src = (uint8_t *) bleChar->_value + offset;
    uint16_t remaining = bleChar->_valueLen - offset;
    *len = MIN(remaining, maxSize);
    logParam("Read length", *len);
    logParam("Data", pData, *len, bleChar->_isBigEnd);
    memcpy(pData, src, *len);
  }
  logRelease();
  return status;
}

static uint8_t serviceWriteAttrCB(void *context,
                                  uint16_t connectionHandle,
                                  uint16_t charHdl, uint16_t len,
                                  uint8_t *pData)
{
  (void) context;
  (void) connectionHandle;
  BLE_Char *bleChar = getChar(charHdl);
  uint8_t status = SNP_SUCCESS;
  if (bleChar == NULL)
  {
    status = SNP_UNKNOWN_ATTRIBUTE;
  }
  logAcquire();
  logChar("Client writing");
  BLE_charWriteValue(bleChar, pData, len, bleChar->_isBigEnd);
  logRelease();
  if (bleChar == &rxChar)
  {
    BLESerial_clientWrite(len, pData);
  }
  return status;
}

static uint8_t serviceCCCDIndCB(void *context,
                                uint16_t connectionHandle,
                                uint16_t cccdHdl, uint8_t type,
                                uint16_t value)
{
  (void) context;
  (void) type;
  (void) connectionHandle;
  uint8_t status = SNP_SUCCESS;
  bool notify = (value == SNP_GATT_CLIENT_CFG_NOTIFY);
  bool indicate = (value == SNP_GATT_CLIENT_CFG_INDICATE);
  BLE_Char *bleChar = getCCCD(cccdHdl);
  logAcquire();
  logChar("Client writing CCCD");
  if (bleChar == NULL)
  {
    status = SNP_UNKNOWN_ATTRIBUTE;
    logError("Unknown handle", connectionHandle);
  }
  // Only 0, or either notify/indicate but not both, is valid.
  else if ((value != 0) && (notify == indicate))
  {
    logParam("Handle", bleChar->_handle);
    logParam("Invalid CCCD", value);
    logParam("Must be 0, 1, 2");
    status = SNP_INVALID_PARAMS;
  }
  // Attempting to set to mode not allowed by char properties
  else if ((notify && !(bleChar->properties & BLE_NOTIFIABLE))
        || (indicate && !(bleChar->properties & BLE_INDICATABLE)))
  {
    logParam("Handle", bleChar->_handle);
    logParam("CCCD not permitted", value);
    status = SNP_NOTIF_IND_NOT_ALLOWED;
  }
  else
  {
    logParam("Handle", bleChar->_handle);
    logParam("Setting to", value);
    bleChar->_CCCD = (uint8_t) value;
  }
  logRelease();
  return status;
}

/*
 * Iterate through list and free malloced data. Note that the BLE_Char's
 * and BLE_Service's are statically allocated and are not freed.
 */
void BLE_clearServices(void)
{
  while (bleServiceListHead != NULL)
  {
    BLE_Service *service = bleServiceListHead->service;
    for (uint8_t i = 0; i < service->numChars; i++)
    {
      free(service->chars[i]->_value);
      service->chars[i]->_value = NULL;
    }
    bleServiceListTail = bleServiceListHead;
    bleServiceListHead = bleServiceListHead->next;
    free(bleServiceListTail);
  }
  bleServiceListHead = NULL;
  bleServiceListTail = NULL;
}
