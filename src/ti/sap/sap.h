/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
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

#ifndef SAP_H
#define SAP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "snp.h"

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

#define SAP_GAP_ADTYPE_FLAGS                        0x01 //!< Discovery Mode: @ref GAP_ADTYPE_FLAGS_MODES
#define SAP_GAP_ADTYPE_FLAGS_GENERAL                0x02 //!< Discovery Mode: LE General Discoverable Mode
#define SAP_GAP_ADTYPE_16BIT_MORE                   0x02 //!< Service: More 16-bit UUIDs available
#define SAP_GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED    0x04 //!< Discovery Mode: BR/EDR Not Supported
#define SAP_GAP_ADTYPE_LOCAL_NAME_COMPLETE          0x09 //!< Complete local name
#define SAP_GAP_ADTYPE_SERVICE_DATA                 0x16 //!< Service Data - 16-bit UUID

/* @defgroup SAP_DEFINES Constants and Structures
 * @{
 */

/** @defgroup SAP_PARAM_SUBSYSTEMS SAP Parameter Subsystems
 * @{
 */
#define SAP_PARAM_HCI             0x0001 //!< GAP HCI parameters: @ref SAP_GAP_HCI_PARAMS
#define SAP_PARAM_ADV             0x0002 //!< Advertising parameters: @ref SAP_ADV_PARAMS
#define SAP_PARAM_CONN            0x0003 //!< Connection parameters: @ref SAP_CONN_PARAMS
#define SAP_PARAM_SECURITY        0x0004 //!< Security parameters: @ref SAP_SECURITY_PARAMS
#define SAP_PARAM_WHITELIST       0x0005 //!< White List parameters: @ref SAP_WHITELIST_PARAMS
#define SAP_PARAM_GAP             0x0006 //!< GAP Parameter IDs: @ref SAP_GAP_PARAMS
/** @} End SAP_PARAM_SUBSYSTEMS */


/** @defgroup SAP_ADV_PARAMS SAP Advertising Parameters
 * @{
 */
#define SAP_ADV_DATA_NOTCONN      0x0001 //!< Advertising Data when not in Connection
#define SAP_ADV_DATA_CONN         0x0002 //!< Advertising Data when in a Connection
#define SAP_ADV_DATA_SCANRSP      0x0003 //!< Scan Response data
#define SAP_ADV_STATE             0x0004 //!< Advertising state
/** @} End SAP_ADV_PARAMS */

/** @defgroup SAP_ADV_STATES SAP Advertising States
 * @{
 */
#define SAP_ADV_STATE_DISABLE     0x0000 //!< Disable advertising
#define SAP_ADV_STATE_ENABLE      0x0001 //!< Enable advertising
/** @} End SAP_ADV_STATES */

/** @defgroup SAP_CONN_PARAMS SAP Connection Parameters
 * @{
 */
#define SAP_CONN_PARAM            0x0001 //!< Connection parameters
#define SAP_CONN_STATE            0x0002 //!< Connection state
/** @} End SAP_CONN_PARAMS */

/** @defgroup SAP_SECURITY_PARAMS SAP Security Parameters
 * @{
 */
#define SAP_SECURITY_IOCAPS       0x00 //!< @ref SAP_SECURITY_IO_CAPABILITIES
#define SAP_SECURITY_BEHAVIOR     0x01 //!< @ref SAP_SECURITY_BEHAVIOR
#define SAP_SECURITY_BONDING      0x02 //!< TRUE for bonding, FALSE for no bonding.
#define SAP_ERASE_ALL_BONDS       0x03 //!< Erase all bonds.
#define SAP_ERASE_LRU_BOND        0x04 //!< Erase Least recently used bond.
#define SAP_NUM_AVAILABLE_BONDS   0x05 //!< Number available bond slots remaining.
/** @} End SAP_SECURITY_PARAMS */

/** @defgroup SAP_SECURITY_IO_CAPABILITIES SAP Security IO Capabilities Parameters
 * @{
 */
#define SAP_DISPLAY_ONLY          0x00  //!< Display Only Device
#define SAP_DISPLAY_YES_NO        0x01  //!< Display and Yes and No Capable
#define SAP_KEYBOARD_ONLY         0x02  //!< Keyboard Only
#define SAP_NO_INPUT_NO_OUTPUT    0x03  //!< No Display or Input Device
#define SAP_KEYBOARD_DISPLAY      0x04  //!< Both Keyboard and Display Capable
/** @} End SAP_SECURITY_IO_CAPABILITIES */

/** @defgroup SAP_SECURITY_BEHAVIOR SAP Security Behavior Parameters
 * @{
 */
#define SAP_SECURITY_NONE                     0x00
#define SAP_SECURITY_WAIT_FOR_REQUEST         0x01
#define SAP_SECURITY_INITIATE_UPON_CONNECTION 0x02
/** @} End SAP_SECURITY_BEHAVIOR */

/** @defgroup SAP_WHITELIST_PARAMS SAP White List Parameters
 * @{
 */
#define SAP_WHITELIST_DISABLE     0x00 //!< Enable White List usage
#define SAP_WHITELIST_ENABLE      0x01 //!< Disable White List usage
/** @} End SAP_WHITELIST_PARAMS */

/** @defgroup SAP_PORT_TYPES SAP port types
 * @{
 */
#define SAP_PORT_LOCAL            0x00 //!< Locally running SAP (not supported)
#define SAP_PORT_REMOTE_UART      0x01 //!< Remote connection w/ SAP over UART
#define SAP_PORT_REMOTE_SPI       0x02 //!< Remote connection w/ SAP over SPI (not supported)
/** @} End SAP_PORT_TYPES */

/** @defgroup SAP_GAP_PARAMS SAP GAP Parameter IDs
 * @{
 */
// Timers
#define SAP_GEN_DISC_ADV_MIN          0  //!< Minimum time to remain advertising, when in Discoverable mode (mSec).  Setting this parameter to 0 turns off the timeout (default).
#define SAP_LIM_ADV_TIMEOUT           1  //!< Maximum time to remain advertising, when in Limited Discoverable mode. In seconds (default 180 seconds)
#define SAP_GEN_DISC_SCAN             2  //!< Minimum time to perform scanning, when performing General Discovery proc (mSec)
#define SAP_LIM_DISC_SCAN             3  //!< Minimum time to perform scanning, when performing Limited Discovery proc (mSec)
#define SAP_CONN_EST_ADV_TIMEOUT      4  //!< Advertising timeout, when performing Connection Establishment proc (mSec)
#define SAP_CONN_PARAM_TIMEOUT        5  //!< Link Layer connection parameter update notification timer, connection parameter update proc (mSec)

// Constants
#define SAP_LIM_DISC_ADV_INT_MIN      6  //!< Minimum advertising interval, when in limited discoverable mode (n * 0.625 mSec)
#define SAP_LIM_DISC_ADV_INT_MAX      7  //!< Maximum advertising interval, when in limited discoverable mode (n * 0.625 mSec)
#define SAP_GEN_DISC_ADV_INT_MIN      8  //!< Minimum advertising interval, when in General discoverable mode (n * 0.625 mSec)
#define SAP_GEN_DISC_ADV_INT_MAX      9  //!< Maximum advertising interval, when in General discoverable mode (n * 0.625 mSec)
#define SAP_CONN_ADV_INT_MIN         10  //!< Minimum advertising interval, when in Connectable mode (n * 0.625 mSec)
#define SAP_CONN_ADV_INT_MAX         11  //!< Maximum advertising interval, when in Connectable mode (n * 0.625 mSec)
#define SAP_CONN_SCAN_INT            12  //!< Scan interval used during Link Layer Initiating state, when in Connectable mode (n * 0.625 mSec)
#define SAP_CONN_SCAN_WIND           13  //!< Scan window used during Link Layer Initiating state, when in Connectable mode (n * 0.625 mSec)
#define SAP_CONN_HIGH_SCAN_INT       14  //!< Scan interval used during Link Layer Initiating state, when in Connectable mode, high duty scan cycle scan parameters (n * 0.625 mSec)
#define SAP_CONN_HIGH_SCAN_WIND      15  //!< Scan window used during Link Layer Initiating state, when in Connectable mode, high duty scan cycle scan parameters (n * 0.625 mSec)
#define SAP_GEN_DISC_SCAN_INT        16  //!< Scan interval used during Link Layer Scanning state, when in General Discovery proc (n * 0.625 mSec)
#define SAP_GEN_DISC_SCAN_WIND       17  //!< Scan window used during Link Layer Scanning state, when in General Discovery proc (n * 0.625 mSec)
#define SAP_LIM_DISC_SCAN_INT        18  //!< Scan interval used during Link Layer Scanning state, when in Limited Discovery proc (n * 0.625 mSec)
#define SAP_LIM_DISC_SCAN_WIND       19  //!< Scan window used during Link Layer Scanning state, when in Limited Discovery proc (n * 0.625 mSec)
#define SAP_CONN_EST_ADV             20  //!< Advertising interval, when using Connection Establishment proc (n * 0.625 mSec). Obsolete - Do not use.
#define SAP_CONN_EST_INT_MIN         21  //!< Minimum Link Layer connection interval, when using Connection Establishment proc (n * 1.25 mSec)
#define SAP_CONN_EST_INT_MAX         22  //!< Maximum Link Layer connection interval, when using Connection Establishment proc (n * 1.25 mSec)
#define SAP_CONN_EST_SCAN_INT        23  //!< Scan interval used during Link Layer Initiating state, when using Connection Establishment proc (n * 0.625 mSec)
#define SAP_CONN_EST_SCAN_WIND       24  //!< Scan window used during Link Layer Initiating state, when using Connection Establishment proc (n * 0.625 mSec)
#define SAP_CONN_EST_SUPERV_TIMEOUT  25  //!< Link Layer connection supervision timeout, when using Connection Establishment proc (n * 10 mSec)
#define SAP_CONN_EST_LATENCY         26  //!< Link Layer connection slave latency, when using Connection Establishment proc (in number of connection events)
#define SAP_CONN_EST_MIN_CE_LEN      27  //!< Local informational parameter about min len of connection needed, when using Connection Establishment proc (n * 0.625 mSec)
#define SAP_CONN_EST_MAX_CE_LEN      28  //!< Local informational parameter about max len of connection needed, when using Connection Establishment proc (n * 0.625 mSec)
#define SAP_PRIVATE_ADDR_INT         29  //!< Minimum Time Interval between private (resolvable) address changes. In minutes (default 15 minutes)
#define SAP_CONN_PAUSE_CENTRAL       30  //!< Central idle timer. In seconds (default 1 second)
#define SAP_CONN_PAUSE_PERIPHERAL    31  //!< Minimum time upon connection establishment before the peripheral starts a connection update procedure. In seconds (default 5 seconds)

// Proprietary
#define SAP_SM_TIMEOUT               32  //!< SM Message Timeout (milliseconds). Default 30 seconds.
#define SAP_SM_MIN_KEY_LEN           33  //!< SM Minimum Key Length supported. Default 7.
#define SAP_SM_MAX_KEY_LEN           34  //!< SM Maximum Key Length supported. Default 16.
#define SAP_FILTER_ADV_REPORTS       35  //!< Filter duplicate advertising reports. Default TRUE.
#define SAP_SCAN_RSP_RSSI_MIN        36  //!< Minimum RSSI required for scan responses to be reported to the app. Default -127.
#define SAP_REJECT_CONN_PARAMS       37  //!< Whether or not to reject Connection Parameter Update Request received on Central device. Default FALSE.
/** @} End SAP_GAP_PARAMS */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/** @defgroup SAP_GAP_EVENT_CB SAP GAP Event Callback.
 * @{
 */
typedef void (*pfnEventCB_t)(uint16_t event, snpEventParam_t *param);

typedef void (*pfnAsyncCB_t)(uint8_t cmd1, void *pParams);

/** @} End SAP_GAP_EVENT_CB */

/** @defgroup SAP_GATT_READ_ATTR_CB SAP Write Attribue Callback.
 * @{
 */
typedef uint8_t (*pfnGATTReadAttrCB_t)(void *context,
                                       uint16_t connectionHandle,
                                       uint16_t charHdl, uint16_t offset,
                                       uint16_t size, uint16_t * len,
                                       uint8_t *pData);
/** @} End SAP_GATT_READ_ATTR_CB */

/** @defgroup SAP_GATT_WRITE_ATTR_CB SAP Write Attribute Callback.
 * @{
 */
typedef uint8_t (*pfnGATTWriteAttrCB_t)(void *context,
                                        uint16_t connectionHandle,
                                        uint16_t charHdl, uint16_t len,
                                        uint8_t *pData);
/** @} End SAP_GATT_WRITE_ATTR_CB */

/** @defgroup SAP_CCCD_Req_CB SAP CCCB Request Call back.
 * @{
 */
typedef uint8_t (*pfnCCCDIndCB_t)(void *context,
                                  uint16_t connectionHandle,
                                  uint16_t cccdHdl, uint8_t rspNeeded,
                                  uint16_t value);
/** @} End SAP_CCCD_REQ_CB */

//!< UUID type
typedef struct
{
  uint8_t len;    //!< Length of UUID buffer
  uint8_t *pUUID; //!< UUID buffer
} UUIDType_t;

//!< User Description Attribute type
typedef snpAddAttrCccd_t                SAP_UserCCCDAttr_t;
typedef snpAddAttrFormat_t              SAP_FormatAttr_t;
typedef snpAddAttrUserDesc_t            SAP_UserDescAttr_t;
typedef snpAddAttrGenShortUUID_t        SAP_ShortUUID_t;
typedef snpAddAttrGenLongUUID_t         SAP_LongUUID_t;

// Characteristic handle type
typedef struct
{
  uint16_t valueHandle;    //!< Handle of characteristic value
  uint16_t userDescHandle; //!< Handle of characteristic user description
  uint16_t cccdHandle;     //!< Handle of characteristic CCCD
  uint16_t formatHandle;   //!< Handle of characteristic format
  uint16_t sUUIDHandle;    //!< Handle of Generic Characteristic Short UUID
  uint16_t lUUIDHandle;    //!< Handle of Generic Characteristic Long UUID
} SAP_CharHandle_t;

//!< Characteristic Value Declaration type.
typedef struct
{
  UUIDType_t         UUID;        //!< Identity of the characteristic
  uint8_t            properties;  //!< Characteristic value properties
  uint8_t            permissions; //!< Characteristic value permissions
  SAP_UserDescAttr_t *pUserDesc;  //!< User descriptor characteristic
  SAP_UserCCCDAttr_t *pCccd;      //!< User CCCD
  SAP_FormatAttr_t   *pFormat;    //!< User format.
  SAP_ShortUUID_t    *pShortUUID; //!< Generic Attribute Descriptor (Short UUID)
  SAP_LongUUID_t     *pLongUUID;  //!< Generic Attribute Descriptor (Long UUID)
} SAP_Char_t;

/** @defgroup SAP_GATT_SERV_REG SAP GATT service registration parameter
 * @{
 */
typedef struct
{
  UUIDType_t           serviceUUID;       //!< Identity of the service
  uint8_t              serviceType;       //!< Primary, Secondary, Included.
  uint16_t             charTableLen;      //!< Length of custom characteristic array
  SAP_Char_t           *charTable;        //!< Array of custom characters to add to the service
  void*                context;           //!< The context is passed by the callbacks
  pfnGATTReadAttrCB_t  charReadCallback;  //!< Read callback function pointer: @ref SAP_GATT_WRITE_ATTR_CB
  pfnGATTWriteAttrCB_t charWriteCallback; //!< Write callback function pointer: @ref SAP_GATT_WRITE_ATTR_CB
  pfnCCCDIndCB_t       cccdIndCallback;   //!< Write callback function pointer: @ref SAP_CCCB_REQ_CB
  uint16_t             serviceHandle;     //!< Handle of the service characteristic
  SAP_CharHandle_t     *charAttrHandles;  //!< Array of handles for the characteristics
} SAP_Service_t;
/** @} End SAP_GATT_SERV_REG */

//!< Port structure when running a local version of SAP (not currently supported)
typedef struct
{
  void *syncHandle; //  SNP will initialize.  SNP_LOCAL only.
} SAP_LocalPort_t;

//!< Port structure when using a remote SNP
typedef struct
{
  uint16_t             stackSize;      //!< Configurable size of stack for SAP Port
  uint16_t             bufSize;        //!< Size of Tx/Rx Transport layer buffers
  uint32_t             mrdyPinID;      //!< Pin ID Mrdy (only with Power Saving enabled)
  uint32_t             srdyPinID;      //!< Pin ID Srdy (only with Power Saving enabled)
  uint8_t              boardID;        //!< Board ID for physical port, i.e. CC2650_UART0
  uint32_t             bitRate;        //!< Baud/Bit Rate for physical port
} SAP_RemotePort_t;

//!< SAP Parameters
typedef struct
{
  uint8_t               portType;
  union
  {
    SAP_LocalPort_t      local;
    SAP_RemotePort_t     remote;
  } port;
} SAP_Params;

/** @} End SAP_DEFINES */

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @defgroup SAP_API High Level SAP API Functions
 *
 * @{
 */

/**
 * @fn      SAP_initParams
 *
 * @brief   Initializes network processor port parameters
 *
 * @param   portType - if port is NPI UART or SPI or local
 * @param   params   - parameter data to initialize.
 *
 * @return  SNP_SUCCESS if port supported by build, else SNP_INVALID_PARAMS.
 */
extern uint8_t SAP_initParams(uint8_t portType, SAP_Params *params);

/**
 * @fn      SAP_open
 *
 * @brief   Opens serial port with the network processor
 *
 * @param   params - list of parameters needed to initialize serial port used
 *                   with network processor
 *
 * @return  uint8_t - SNP_SUCCESS if NPI is open
 */
extern uint8_t SAP_open(SAP_Params *params);

/**
 * @fn      SAP_close
 *
 * @brief   Tears down serial port with the network processor
 *
 * @param   None.
 *
 * @return  uint8_t - SNP_SUCCESS if able to close
 */
extern uint8_t SAP_close(void);

/**
 * @fn      SAP_setAsyncCB
 *
 * @brief   setup Applications' asynchronous callbacks.  This must be called before
 *          using any other calls to SAP.  This function may be called multiple times
 *          to register multiple Callbacks.
 *
 * @param   asyncCB - the asynchronous callback.
 *
 * @return  None.
 */
extern uint8_t SAP_setAsyncCB(pfnAsyncCB_t asyncCB);

/**
 * @brief       Initialize the SAP module
 *
 * @param       None.
 *
 * @return      SNP_SUCCESS: the device correctly initialized.<BR>
 *              SAP_FAILURE: the device failed to initialize.<BR>
 */
extern uint8_t SAP_initDevice(void);

/**
 * @brief       Reset the SAP and return only when it has restarted.
 *
 * @param       none
 *
 * @return      SNP_SUCCESS: the device correctly reset.<BR>
 */
extern uint8_t SAP_reset(void);

/**
 * @brief       Register a GATT service with the GATT Server.
 *
 * @param       serviceParams - a pointer to service data: @ref SAP_GATT_SERV_REG
 *                              It is the callers responsibility to allocate
 *                              this structure.
 *
 * @return      SNP_SUCCESS: the service is registered.<BR>
 *              SAP_FAILURE: service registration failed.<BR>
 */
extern uint8_t SAP_registerService(SAP_Service_t *serviceParams);

/**
 * @brief       Register a callback to receive a GAP event.  This shall be
 *              called once for each callback to be registered.
 *
 * @param       eventCB   - a Callback function to register: @ref SAP_GAP_EVENT_CB
 * @param       eventMask - the mask of events which trigger this
 *                          callback. Events types: @ref SAP_GAP_EVENTS
 *
 * @return      SNP_SUCCESS: the device correctly reset.<BR>
 *              SAP_FAILURE: callback registration failed.<BR>
 */
extern uint8_t SAP_registerEventCB(pfnEventCB_t eventCB, uint16_t eventMask);

/**
 * @brief       Read a characteristic value of a service.
 *
 * @param       serviceID - the UUID of the service
 * @param       charID    - the unique handle of the characteristic
 * @param       len       - length of the data read
 * @param       pData     - pointer to buffer to write to
 *
 * @return      SNP_SUCCESS: the read completed successfully.<BR>
 *              SAP_FAILURE: service param request failed.<BR>
 */
extern uint8_t SAP_getServiceParam(uint8_t serviceID, uint8_t charID,
                                   uint16_t *len, uint8_t *pData);

/**
 * @brief       Write a characteristic value of a service.
 *
 * @param       serviceID - the UUID of the service
 * @param       charID    - the unique handle of the characteristic
 * @param       len       - length of the data to write
 * @param       pData     - pointer to buffer of data to write
 *
 * @return      SNP_SUCCESS: the write completed successfully.<BR>
 *              SAP_FAILURE: service param write request failed.<BR>
 */
extern uint8_t SAP_setServiceParam(uint8_t serviceID, uint8_t charID,
                                   uint16_t len, uint8_t *pData);

/**
 * @brief       Write to a stack parameter on the SAP. Some responses will
 *              Return immediately, others will generate an event for which
 *              a callback must be registered with the correct event mask.
 *
 * @param       subsystemID  - the subsystem ID: @ref SAP_PARAM_SUBSYSTEMS
 * @param       paramID     - the parameter within the subsystem to write
 * @param       len         - length of the data to write
 * @param       pData       - pointer to buffer of data to write
 *
 * @return      SNP_SUCCESS: the write completed successfully.<BR>
 *              SAP_FAILURE: stack parameter write failed.<BR>
 */
extern uint8_t SAP_setParam(uint8_t subsystemID, uint16_t paramID,
                            uint16_t len, uint8_t *pData);

/**
 * @brief       Read a stack parameter on the SAP. Some responses will
 *              Return immediately, others will generate an event for which
 *              a callback must be registered with the correct event mask.
 *
 * @param       subsystemID - the subsystem ID: @ref SAP_PARAM_SUBSYSTEMS
 * @param       paramID    - the parameter within the subsystem to read
 * @param       len        - length of the data to read
 * @param       pData      - pointer to buffer to write to
 *
 * @return      SNP_SUCCESS: the read completed successfully.<BR>
 *              SAP_FAILURE: stack param read failed.<BR>
 */
extern uint8_t SAP_getParam(uint8_t subsystemID, uint8_t paramID,
                            uint16_t len, uint8_t *pData);

/**
 * @brief       Request Security
 *
 * @param       None
 *
 * @return      SNP_SUCCESS: security was requested.<BR>
 *              SNP_FAILURE: security request failed.<BR>
 */
extern uint8_t SAP_sendSecurityRequest(void);

/**
 * @brief       Send authentication data to SNP.
 *
 * @param       authData - authentication data.  This is the passkey used for
 *                         for the passkey entry protocol.  For numeric
 *                         comparisons, this is TRUE if equivalent or FALSE if
 *                         not.
 *
 * @return      SNP_SUCCESS: authentication data sent.<BR>
 *              SNP_FAILURE: authentication data failed to be sent.<BR>
 */
extern uint8_t SAP_setAuthenticationRsp(uint32_t authData);

/**
 * @brief       Set event mask of SNP events
 *
 * @param       eventMask - mask of event flags for SNP to send. event types
 *                          not included in this mask will not be sent.
 *
 * @return      SNP_SUCCESS: mask set.<BR>
 */
extern uint8_t SAP_setSNPEventMask(uint16_t eventMask);

/*********************************************************************
 * @fn      SAP_getRevision
 *
 * @brief   Get SNP Image revision and Stack revision numbers
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  None
 */
extern void SAP_getRevision(snpGetRevisionRsp_t *pRsp);

/*********************************************************************
 * @fn      SAP_getRand
 *
 * @brief   Get a random number generated by the SNP TRNG
 *
 * @param   None
 *
 * @return  32 bit random number
 */
extern uint32_t SAP_getRand(void);

/*********************************************************************
 * @fn      SAP_testCommand
 *
 * @brief   Get SNP memory statistics, returned as an asynchronous event.
 *
 * @return  None
 */
extern void SAP_testCommand(void);

/*********************************************************************
 * @fn      SAP_getStatus
 *
 * @brief   Get SNP status
 *
 * @param   pRsp - pointer to SNP response message
 *
 * @return  Nonde
 */
extern void SAP_getStatus(snpGetStatusCmdRsp_t *pRsp);


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SAP_H */
