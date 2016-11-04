
#ifndef BLE_TYPES_H
#define BLE_TYPES_H

#include <Energia.h>

#include "ti/sap/sap.h"
#include "ti/npi/hal_defs.h"

/* Energia BLE status codes */
#define BLE_SUCCESS                    SNP_SUCCESS // 0x00
#define BLE_FAILURE                    SNP_FAILURE // 0x83
#define BLE_INVALID_PARAMETERS         SNP_INVALID_PARAMS // 0x84
#define BLE_ALREADY_ADVERTISING        SNP_ALREADY_ADVERTISING // 0x8A
#define BLE_NOT_ADVERTISING            SNP_NOT_ADVERTISING // 0x8B
#define BLE_NOT_CONNECTED              SNP_NOT_CONNECTED // 0x92
#define BLE_INVALID_HANDLE             0x50
#define BLE_UNDEFINED_VALUE            0x51
#define BLE_NOT_IMPLEMENTED            0x52
#define BLE_TIMEOUT                    0x53
#define BLE_CHECK_ERROR                0x54

/* Log Levels */
#define BLE_LOG_NONE                   0x00
#define BLE_LOG_ERRORS                 0x01
#define BLE_LOG_RPCS                   0x02
#define BLE_LOG_REC_MSGS               0x04
#define BLE_LOG_MSGS (BLE_LOG_RPCS | BLE_LOG_REC_MSGS)
#define BLE_LOG_CHARACTERISTICS        0x08
#define BLE_LOG_ALL                    0xFF

/* Type of connection between the user application's
   processor and the bluetooth chip */
#define BLE_PORT_LOCAL                 SAP_PORT_LOCAL // unsupported
#define BLE_PORT_UART                  SAP_PORT_REMOTE_UART
#define BLE_PORT_SPI                   SAP_PORT_REMOTE_SPI // unsupported

/*
 * For setAdvertData.
 * Data to advertise when not connected, connected, and when scanned.
 * The first two are passively seen by a client, while scanning is an
 * active action to request more data.
 */
#define BLE_ADV_DATA_NOTCONN           SAP_ADV_DATA_NOTCONN
#define BLE_ADV_DATA_CONN              SAP_ADV_DATA_CONN
#define BLE_ADV_DATA_SCANRSP           SAP_ADV_DATA_SCANRSP
/* The number of kinds of advertising data */
#define MAX_ADVERT_IDX                 3

/*
 * For startAdvert.
 * Non-connectable undirected advertisement.
 * Connectable undirected adverisement.
 * Scannable undirected advertisement.
 */
#define BLE_ADV_MODE_NONCONN           SNP_ADV_TYPE_NONCONN
#define BLE_ADV_MODE_CONN              SNP_ADV_TYPE_CONN
#define BLE_ADV_MODE_SCANNABLE         SNP_ADV_TYPE_SCANABLE

/* startAdvert connectedBehavior */
#define BLE_ADV_STOP_ON_CONN           SNP_ADV_STOP_ON_CONN
#define BLE_ADV_RESTART_ON_CONN_EST    SNP_ADV_RESTART_ON_CONN_EST
#define BLE_ADV_RESTART_ON_CONN_TERM   SNP_ADV_RESTART_ON_CONN_TERM

/* Characteristic value properties */
#define BLE_READABLE                   SNP_GATT_PROP_READ
#define BLE_WRITABLE_NORSP             SNP_GATT_PROP_WRITE_NORSP
#define BLE_WRITABLE                   SNP_GATT_PROP_WRITE
#define BLE_NOTIFIABLE                 SNP_GATT_PROP_NOTIFICATION
#define BLE_INDICATABLE                SNP_GATT_PROP_INDICATION
/* Note that the below defines depend SNP_GATT_* bitfields not changing */
#define BLE_AUTHEN                     0x40
#define BLE_ENCRYPT                    0x80

/* Upper two bits are reserved to set Authen and Encrypt */
#define BLE_PROPERTIES_MASK            0x3F

/*
 * Serial Buffer Length
 */
#define BLE_SERIAL_BUFFER_SIZE 128

/*
 * Security Parameters
 */
/* Security Pairing Modes */
#define BLE_SECURITY_NONE                      SAP_SECURITY_NONE                     // 0x00
#define BLE_SECURITY_WAIT_FOR_REQUEST          SAP_SECURITY_WAIT_FOR_REQUEST         // 0x01
#define BLE_SECURITY_INITIATE_UPON_CONNECTION  SAP_SECURITY_INITIATE_UPON_CONNECTION // 0x02

/* Security IO Capabilities */
#define BLE_DISPLAY_ONLY               SAP_DISPLAY_ONLY       // Display Only Device
#define BLE_DISPLAY_YES_NO             SAP_DISPLAY_YES_NO     // Display and Yes and No Capable
#define BLE_KEYBOARD_ONLY              SAP_KEYBOARD_ONLY      // Keyboard Only
#define BLE_NO_INPUT_NO_OUTPUT         SAP_NO_INPUT_NO_OUTPUT // No Display or Input Device
#define BLE_KEYBOARD_DISPLAY           SAP_KEYBOARD_DISPLAY   // Both Keyboard and Display Capable

/* Security States */
#define BLE_SECURITY_STATE_COMPLETE    SNP_GAPBOND_PAIRING_STATE_COMPLETE   // Pairing complete
#define BLE_SECURITY_STATE_BONDED      SNP_GAPBOND_PAIRING_STATE_BONDED     // Devices bonded
#define BLE_SECURITY_STATE_BOND_SAVED  SNP_GAPBOND_PAIRING_STATE_BOND_SAVED // Bonding record saved in NV

// Minimum connection interval (units of 1.25ms, 6=7.5ms) if automatic
// parameter update request is enabled
#define BLE_DEF_DESIRED_MIN_CONN_INT   6

// Maximum connection interval (units of 1.25ms, 80=100ms) if automatic
// parameter update request is enabled
#define BLE_DEF_DESIRED_MAX_CONN_INT   80

 // Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                  0x000D
#define TI_ST_DEVICE_ID                0x03
#define TI_ST_KEY_DATA_ID              0x00

typedef struct
{
  unsigned char     UUID[16]; // array of UUID bytes, little-endian
  unsigned char     properties; // bitwise OR of macros: e.g. BLE_READABLE | BLE_WRITABLE
  // Null terminated; internally set permissions to read only so we don't have to worry about the length changing
  const char        *charDesc;
  /* Energia user should never need to touch these. */
  unsigned char     _valueFormat;
  unsigned char     _valueExponent; // only used with integer formats, e.g. value = storedValue*10^valueExponent
  uint16_t          _handle;
  void              *_value;
  bool              _isBigEnd;
  uint16_t          _valueLen;
  uint8_t           _CCCD;
  uint16_t          _CCCDHandle;
  uint8_t           _UUIDlen;
} BLE_Char;

typedef struct
{
  unsigned char     UUID[16]; // array of UUID bytes
  unsigned int      numChars;
  BLE_Char          **chars;
  /* Energia user should never need to touch these. */
  uint16_t          _handle;
  uint8_t           _UUIDlen;
} BLE_Service;

typedef struct
{
  unsigned char     advertMode;
  unsigned int      timeout; // How long to advertise for (in ms), 0 for indefinitely
  unsigned int      interval; // Advertising Interval (n * 0.625 ms), 0 for 100ms default
  /* 0x00   Advertising is disabled during connection and will not start after.
   * 0x01   Advertising will continue with non-connectable advertising when connection is established
        There are separate GAP parameters for setting connected advertising interval.
   * 0x02   Advertising will restart with connectable advertising when a connection is terminated.
   */
  unsigned char      connectedBehavior;
} BLE_Advert_Settings;

typedef void (*displayStringFxn_t)(const char string[]);
typedef void (*displayUIntFxn_t)(uint32_t num);

/*******************************************************************************
 * See the SNP API guide for documentation on these typedefs.
 ******************************************************************************/
typedef snpGetRevisionRsp_t BLE_Get_Revision_Rsp;
typedef snpGetStatusCmdRsp_t BLE_Get_Status_Rsp;
typedef snpTestCmdRsp_t BLE_Test_Command_Rsp;
typedef snpUpdateConnParamReq_t BLE_Conn_Params_Update_Req;
typedef snpConnEstEvt_t BLE_Conn_Params;

#endif
