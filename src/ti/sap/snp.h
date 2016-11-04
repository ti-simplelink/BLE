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

#ifndef SNP_H
#define SNP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include <ti/npi/hal_types.h>

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

//! \brief Location in memory where the SNP image is located
#define SNP_IMAGE_START           0x00000000

//! \brief list of command type (include SNP subsystem)
/** @defgroup SNP_NPI_TYPE NPI frame type for SNP (NPI cmd 0)
 * @{
 */
//! (0x55) Asynchronous Command type for the BLE simple network processor (cmd0 field of npi frame)
#define SNP_NPI_ASYNC_CMD_TYPE    (RPC_SYS_BLE_SNP+(NPI_MSG_TYPE_ASYNC<<5))
//! (0x35) Synchronous request type for the BLE simple network processor (cmd0 field of npi frame)
#define SNP_NPI_SYNC_REQ_TYPE     (RPC_SYS_BLE_SNP+(NPI_MSG_TYPE_SYNCREQ<<5))
//! (0x75) Synchronous response type for the BLE simple network processor (cmd0 field of npi frame)
#define SNP_NPI_SYNC_RSP_TYPE     (RPC_SYS_BLE_SNP+(NPI_MSG_TYPE_SYNCRSP<<5))
/**
  * @}
  */

//! \brief list of subgroup
// Fixed size, must not exceed 2 bits!
// the upper two bits of NPI frame cmd1 field is reserved for subgroup value.
enum SNP_CMD1_HDR_TYPE {
    SNP_DEVICE_GRP = 0,
    SNP_GAP_GRP,
    SNP_GATT_GRP,
    SNP_RFU_GRP
};

//! \brief list of request for the device subgroup
#define SNP_MASK_EVT_REQ                       0x02
#define SNP_GET_REVISION_REQ                   0x03
#define SNP_HCI_CMD_REQ                        0x04
#define SNP_GET_STATUS_REQ                     0x06
#define SNP_GET_RAND_REQ                       0x07
#define SNP_TEST_REQ                           0x10

//! \brief list of indication/response for the device subgroup
#define SNP_POWER_UP_IND                       0x01
#define SNP_MASK_EVENT_RSP                     0x02
#define SNP_GET_REVISION_RSP                   0x03
#define SNP_HCI_CMD_RSP                        0x04
#define SNP_EVENT_IND                          0x05
#define SNP_GET_STATUS_RSP                     0x06
#define SNP_SYNC_ERROR_CMD_IND                 0x07
#define SNP_GET_RAND_RSP                       0x08
#define SNP_TEST_RSP                           0x10

//! \brief list of command for the GAP subgroup
#define SNP_INIT_DEVICE_REQ                    0x41
#define SNP_START_ADV_REQ                      0x42
#define SNP_SET_ADV_DATA_REQ                   0x43
#define SNP_STOP_ADV_REQ                       0x44
#define SNP_UPDATE_CONN_PARAM_REQ              0x45
#define SNP_TERMINATE_CONN_REQ                 0x46
#define SNP_SET_GAP_PARAM_REQ                  0x48
#define SNP_GET_GAP_PARAM_REQ                  0x49
#define SNP_SET_SECURITY_PARAM_REQ             0x4A
#define SNP_SEND_SECURITY_REQUEST_REQ          0x4B
#define SNP_SET_AUTHENTICATION_DATA_REQ        0x4C
#define SNP_SET_WHITE_LIST_POLICY_REQ          0x4D

//! \brief list of response/indication for the GAP subgroup
#define SNP_INIT_DEVICE_CNF                    0x41
#define SNP_SET_ADV_DATA_CNF                   0x43
#define SNP_UPDATE_CONN_PARAM_CNF              0x45
#define SNP_SET_GAP_PARAM_RSP                  0x48
#define SNP_GET_GAP_PARAM_RSP                  0x49
#define SNP_SET_SECURITY_PARAM_RSP             0x4A
#define SNP_SEND_AUTHENTICATION_DATA_RSP       0x4C
#define SNP_SET_WHITE_LIST_POLICY_RSP          0x4D

//! \brief list of command for the GATT subgroup
#define SNP_ADD_SERVICE_REQ                    0x81
#define SNP_ADD_CHAR_VAL_DECL_REQ              0x82
#define SNP_ADD_CHAR_DESC_DECL_REQ             0x83
#define SNP_REGISTER_SERVICE_REQ               0x84
#define SNP_GET_ATTR_VALUE_REQ                 0x85
#define SNP_SET_ATTR_VALUE_REQ                 0x86
#define SNP_CHAR_READ_CNF                      0x87
#define SNP_CHAR_WRITE_CNF                     0x88
#define SNP_SEND_NOTIF_IND_REQ                 0x89
#define SNP_CCCD_UPDATED_CNF                   0x8B
#define SNP_SET_GATT_PARAM_REQ                 0x8C
#define SNP_GET_GATT_PARAM_REQ                 0x8D
#define SNP_REG_PREDEF_SRV_REQ                 0x8E

//! \brief list of response/indication for the GATT subgroup
#define SNP_ADD_SERVICE_RSP                    0x81
#define SNP_ADD_CHAR_VAL_DECL_RSP              0x82
#define SNP_ADD_CHAR_DESC_DECL_RSP             0x83
#define SNP_REGISTER_SERVICE_RSP               0x84
#define SNP_GET_ATTR_VALUE_RSP                 0x85
#define SNP_SET_ATTR_VALUE_RSP                 0x86
#define SNP_CHAR_READ_IND                      0x87
#define SNP_CHAR_WRITE_IND                     0x88
#define SNP_SEND_NOTIF_IND_CNF                 0x89
#define SNP_CCCD_UPDATED_IND                   0x8B
#define SNP_SET_GATT_PARAM_RSP                 0x8C
#define SNP_GET_GATT_PARAM_RSP                 0x8D
#define SNP_REG_PREDEF_SRV_RSP                 0x8E

/** @defgroup SNP_ERRORS list of status generated by the SNP.
 * @{
 */
//! \brief list of possible status return by the SNP
#define SNP_SUCCESS                            0x00
#define SNP_FAILURE                            0x83
#define SNP_INVALID_PARAMS                     0x84
#define SNP_CMD_ALREADY_IN_PROGRESS            0x85
#define SNP_CMD_REJECTED                       0x86
#define SNP_OUT_OF_RESOURCES                   0x87
#define SNP_UNKNOWN_ATTRIBUTE                  0x88
#define SNP_UNKNOWN_SERVICE                    0x89
#define SNP_ALREADY_ADVERTISING                0x8A
#define SNP_NOT_ADVERTISING                    0x8B
#define SNP_HCI_RSP_COLLISION_RSP              0x8C
#define SNP_HCI_CMD_UNKNOWN                    0x8D
#define SNP_GATT_COLLISION                     0x8E
#define SNP_NOTIF_IND_NOT_ENABLE_BY_CLIENT     0x8F
#define SNP_NOTIF_IND_NOT_ALLOWED              0x90
#define SNP_NOTIF_IND_NO_CCCD_ATTRIBUTE        0x91
#define SNP_NOT_CONNECTED                      0x92
#define SNP_FAILURE_NO_ATT_RSP                 0x93
/**
 * @}
 */

/** @defgroup SNP_EVENT list of event generated by the SNP.
 * @{
 */
//! \brief list of possible events generated by the SNP
#define  SNP_CONN_EST_EVT                      0x0001
#define  SNP_CONN_TERM_EVT                     0x0002
#define  SNP_CONN_PARAM_UPDATED_EVT            0x0004
#define  SNP_ADV_STARTED_EVT                   0x0008
#define  SNP_ADV_ENDED_EVT                     0x0010
#define  SNP_ATT_MTU_EVT                       0x0020
#define  SNP_SECURITY_EVT                      0x0040
#define  SNP_AUTHENTICATION_EVT                0x0080
#define  SNP_ERROR_EVT                         0x8000
/**
 * @}
 */
//! \brief this mask check that only meaningful event can be masked/send.
#define SNP_DEV_ALL_EVENT_MASK (SNP_CONN_EST_EVT | SNP_CONN_TERM_EVT | \
SNP_CONN_PARAM_UPDATED_EVT | SNP_ADV_STARTED_EVT | SNP_ADV_ENDED_EVT | \
  SNP_ATT_MTU_EVT | SNP_ERROR_EVT)
/* HCI Opcodes */
/** @defgroup SNP_ALLOWED_HCI list of all HCI commands allowed by the SNP.
 * @{
 */
#define SNP_HCI_OPCODE_EXT_SET_TX_POWER                 0xFC01 //!< HCI Extension Set Tx Power
#define SNP_HCI_OPCODE_EXT_MODEM_TEST_TX                0xFC08 //!< HCI Extension Modem Test Tx
#define SNP_HCI_OPCODE_EXT_MODEM_HOP_TEST_TX            0xFC09 //!< HCI Extension Modem Hop Test Tx
#define SNP_HCI_OPCODE_EXT_MODEM_TEST_RX                0xFC0A //!< HCI Extension Test Rx
#define SNP_HCI_OPCODE_EXT_END_MODEM_TEST               0xFC0B //!< HCI Extension End Modem Test
#define SNP_HCI_OPCODE_EXT_SET_BDADDR                   0xFC0C //!< HCI Extension Set BD Address
#define SNP_HCI_OPCODE_EXT_SET_SCA                      0xFC0D //!< HCI Extension Set SCA
#define SNP_HCI_OPCODE_EXT_ENABLE_PTM                   0xFC0E //!< HCI Extension Enable PTM
#define SNP_HCI_OPCODE_EXT_SET_MAX_DTM_TX_POWER         0xFC11 //!< HCI Extension Set Max DTM Tx Power
#define SNP_HCI_OPCODE_EXT_RESET_SYSTEM                 0xFC1D //!< HCI Extension Reset System
#define SNP_HCI_OPCODE_READ_BDADDR                      0x1009 //!< HCI Extension Read BD Address
#define SNP_HCI_OPCODE_READ_RSSI                        0x1405 //!< HCI Extension Read RSSI
#define SNP_HCI_OPCODE_LE_RECEIVER_TEST                 0x201D //!< HCI Extension LE Receiver Test
#define SNP_HCI_OPCODE_LE_TRANSMITTER_TEST              0x201E //!< HCI Extension LE Transmitter Test
#define SNP_HCI_OPCODE_LE_TEST_END                      0x201F //!< HCI Extension LE Test End Command
#define SNP_HCI_OPCODE_EXT_PER                          0xFC14 //!< HCI Extension PER Command
#define SNP_HCI_OPCODE_EXT_DECRYPT                      0xFC05 //!< HCI Extension decrypt encrypted data using AES128
#define SNP_HCI_OPCODE_LE_ENCRYPT                       0x2017 //!< HCI Extension encrypt data using AES128
#define SNP_HCI_OPCODE_EXT_OVERRIDE_SL                  0xFC1A //!< HCI Extension enable or disable suspending slave latency
#define SNP_HCI_OPCODE_EXT_SET_FAST_TX_RESP_TIME        0xFC07 //!< HCI Extension set whether transmit data is sent as soon as possible even when slave latency is used
#define SNP_HCI_OPCODE_EXT_ONE_PKT_PER_EVT              0xFC02 //!< HCI Extension set whether a connection will be limited to one packet per event
#define SNP_HCI_OPCODE_EXT_GET_CONNECTION_INFO          0xFC20 //!< HCI Extension get connection related information
/**
 * @}
 */
#define SNP_HCI_OPCODE_SIZE              0x02


/* Advertising */
/** @defgroup SNP_AdvType Advertisement Type
 * @{
 */
#define SNP_ADV_TYPE_CONN                0x00 //!< Connectable undirected advertisement
#define SNP_ADV_TYPE_SCANABLE            0x02 //!< Scannable undirected advertisement
#define SNP_ADV_TYPE_NONCONN             0x03 //!< Non-Connectable undirected advertisement
 /**
 * @}
 */

/** @defgroup SNP_AdvFilterPolicyType Advertisement Filter Policy Type
 * @{
 */
#define SNP_FILTER_POLICY_ALL             0x00 //!< Allow Scan Request from Any, Allow Connect Request from Any (default).
#define SNP_FILTER_POLICY_WHITE           0x03 //!< Allow Scan Request and Connect from White List Only
/**
 * @}
 */

/** @defgroup SNP_AdvBehavior Adv. behavior when connection are established or terminated.
 * @{
 */
//! Advertising is disabled during connection and will not start after.
#define SNP_ADV_STOP_ON_CONN             0x00
//! @brief Advertising will continue with non-connectable advertising when
//! connection is established. period and interval of advertising are different when the device is in a connection. see @ref SNP_startAdv
#define SNP_ADV_RESTART_ON_CONN_EST      0x01
//! @brief Advertising will start with connectable advertising when a
//! connection is terminated.
#define SNP_ADV_RESTART_ON_CONN_TERM     0x02
 /**
 * @}
 */

/** @defgroup SNP_Advbuffer Adv. data buffer
 * @{
 */
//!  data buffer holding the scanning response
#define SNP_SCANRSP_DATA                 0x00 // Scan response data
//!  data buffer holding advertisement data that will be advertise if the device is NOT in a connection
#define SNP_NONCONN_ADV_DATA             0x01 // Non-connectable advertising data.
//!  data buffer holding advertisement data that will be advertise if the device is in a connection
#define SNP_CONN_ADV_DATA                0x02 // Connectable advertising data.
 /**
 * @}
 */

//!< This is the maximum size of a scan response or advertising data buffer.
#define SNP_MAX_ADVDATA_LENGTH           31

//! @brief Connection Terminated (CT)
/** @defgroup SNP_connTerm connection termination reason.
 * @{
 */
#define SNP_CT_SUPERVISOR_TO             0x08 //!< Supervisor timeout
#define SNP_CT_PEER_REQ                  0x13 //!< Peer Requested
#define SNP_CT_HOST_REQ                  0x16 //!< Host Requested
#define SNP_CT_CTRL_PKT_TO               0x22 //!< Control Packet Timeout
#define SNP_CT_CTRL_PKT_INST_PASS        0x28 //!< Control Packet Instance Passed
#define SNP_CT_LSTO_VIOLATION            0x3B //!< LSTO Violation
#define SNP_CT_MIC_FAILURE               0x3D //!< MIC Failure
 /**
 * @}
 */

/* Connection Parameters and Updates */
//! \brief Connection Interval Range (in 1.25ms units, ranging from 7.5ms to 4s)
#define SNP_CONN_INT_MIN                 0x0006  //!< minimal connection interval range: 7.5ms
#define SNP_CONN_INT_MAX                 0x0C80  //!< maximal connection interval range: 4a

//! \brief Slave Latency Ranges
#define SNP_CONN_SL_MIN                  0x0000 //!< minimal slave latency value
#define SNP_CONN_SL_MAX                  0x01F3 //!< maximum slave latency value

//! \brief Supervisor Timeout Range (in 10ms units, ranging from 100ms to 32 seconds)
#define SNP_CONN_SUPERVISOR_TIMEOUT_MIN  0x000A //!< minimum supervision timeout, multiple of 10ms
#define SNP_CONN_SUPERVISOR_TIMEOUT_MAX  0x0C80 //!< maximum supervision timeout, multiple of 10ms

/** @defgroup SNP_SERVICE_TYPE Service Type
 * @{
 */
//! Primary service
#define SNP_PRIMARY_SERVICE                   1
//! Secondary service
#define SNP_SECONDARY_SERVICE                 2
/** @} End SNP_SERVICE_TYPE  */

//! \brief characteristic value management Management option (RFU)
#define SNP_CHAR_MANAGED_BY_AP             0x00
#define SNP_CHAR_MANAGED_BY_NP             0x01

//! \brief UUID Sizes
#define SNP_16BIT_UUID_SIZE                   2
#define SNP_128BIT_UUID_SIZE                 16

/** @defgroup SNP_PERMIT_BITMAPS_DEFINES GATT Attribute Access Permissions Bit Fields
 * @{
 */
//! @brief For now, Only read and write permissions are allowed.
//! permission to read the attribute value
#define SNP_GATT_PERMIT_READ                0x01
//! permission to write the attribute value
#define SNP_GATT_PERMIT_WRITE               0x02
//! permission to authenticated read the attribute value
#define SNP_GATT_PERMIT_AUTHEN_READ         0x04
//! permission to authenticated write the attribute value
#define SNP_GATT_PERMIT_AUTHEN_WRITE        0x08
//! permission to encrypted read the attribute value
#define SNP_GATT_PERMIT_ENCRYPT_READ        0x40
//! permission to encrypted write the attribute value
#define SNP_GATT_PERMIT_ENCRYPT_WRITE       0x80

/** @} End SNP_PERMIT_BITMAPS_DEFINES */

//! \brief mask for clearing RFU bits on the attribute permissions field
#define SNP_GATT_PERM_RFU_MASK   (SNP_GATT_PERMIT_READ         | \
                                  SNP_GATT_PERMIT_WRITE        | \
                                  SNP_GATT_PERMIT_AUTHEN_READ  | \
                                  SNP_GATT_PERMIT_AUTHEN_WRITE | \
                                  SNP_GATT_PERMIT_ENCRYPT_READ | \
                                  SNP_GATT_PERMIT_ENCRYPT_WRITE)

/** @defgroup SNP_PROP_BITMAPS_DEFINES GATT Characteristic Properties Bit Fields
 * @{
 */
#define SNP_GATT_PROP_READ               0x0002 // Read
#define SNP_GATT_PROP_WRITE_NORSP        0x0004 // Write without response
#define SNP_GATT_PROP_WRITE              0x0008 // Writes
#define SNP_GATT_PROP_NOTIFICATION       0x0010 // Notifications
#define SNP_GATT_PROP_INDICATION         0x0020 // Indications
/** @} End SNP_PROP_BITMAPS_DEFINES */

//! \brief Maximum Length of the Characteristic Value Attribute
#define SNP_GATT_CHAR_MAX_LENGTH            512

/** @defgroup SNP_IND_TYPE SNP indication type
 * @{
 */
//! \brief Notification type
#define SNP_SEND_NOTIFICATION               0x01
//! \brief Indication type
#define SNP_SEND_INDICATION                 0x02
/** @} End SNP_IND_TYPE */


/** @defgroup SNP_PREDEF_SERVICE Predefined services
 * @{
 */
//! \brief GATT Param parameters
#define SNP_GGS_SERV_ID                     0x01 // Generic Access Service
#define SNP_DIS_SERV_ID                     0x02 // Device Info Service
/** @} End SNP_PREDEF_SERVICE */

/** @defgroup SNP_DEV_INFO_SERVICE Device Information Service Parameters
 * @{
 */
#define SNP_DEVINFO_SYSTEM_ID                  0 //!< RW  uint8_t[#SNP_DEVINFO_SYSTEM_ID_LEN]
#define SNP_DEVINFO_MODEL_NUMBER               1 //!< RW  uint8_t[#SNP_DEVINFO_STRING_LEN]
#define SNP_DEVINFO_SERIAL_NUMBER              2 //!< RW  uint8_t[#SNP_DEVINFO_STRING_LEN]
#define SNP_DEVINFO_FIRMWARE_REV               3 //!< RW  uint8_t[#SNP_DEVINFO_STRING_LEN]
#define SNP_DEVINFO_HARDWARE_REV               4 //!< RW  uint8_t[#SNP_DEVINFO_STRING_LEN]
#define SNP_DEVINFO_SOFTWARE_REV               5 //!< RW  uint8_t[#SNP_DEVINFO_STRING_LEN]
#define SNP_DEVINFO_MANUFACTURER_NAME          6 //!< RW  uint8_t[#SNP_DEVINFO_STRING_LEN]
#define SNP_DEVINFO_11073_CERT_DATA            7 //!< RW variable length
#define SNP_DEVINFO_PNP_ID                     8 //!< RW  uint8_t[#SNP_DEVINFO_PNP_ID_LEN]

// System ID length
#define SNP_DEVINFO_SYSTEM_ID_LEN              8

// PnP ID length
#define SNP_DEVINFO_PNP_ID_LEN                 7

//! \brief default string length, including the null terminated char.
// this is for information only, do not change it here.
#define SNP_DEVINFO_STRING_LEN                 21
/** @} End SNP_DEV_INFO_SERVICE */

/** @defgroup SNP_GAP_SERVICE GAP GATT Server Parameters
 * @{
 */
#define SNP_GGS_DEVICE_NAME_ATT                0   //!< RW  uint8_t[#SNP_GAP_DEVICE_NAME_LEN]
#define SNP_GGS_APPEARANCE_ATT                 1   //!< RW  uint16_t
#define SNP_GGS_PERI_PRIVACY_FLAG_ATT          2   //!< RESERVED, DO NOT USED, need the stack to be build with GAP_PRIVACY and GAP_PRIVACY_RECONNECT RW  uint8_t
#define SNP_GGS_RECONNCT_ADDR_ATT              3   //!< RESERVED, DO NOT USED, need the stack to be build with GAP_PRIVACY_RECONNECT  RW  uint8_t[B_ADDR_LEN]
#define SNP_GGS_PERI_CONN_PARAM_ATT            4   //!< RW  sizeof(#snpGapPeriConnectParams_t)
#define SNP_GGS_PERI_PRIVACY_FLAG_PROPS        5   //!< RESERVED, DO NOT USED, need the stack to be build with GAP_PRIVACY and GAP_PRIVACY_RECONNECT RW  uint8_t
#define SNP_GGS_W_PERMIT_DEVICE_NAME_ATT       6   //!< SET ONLY (W   uint8_t)
#define SNP_GGS_W_PERMIT_APPEARANCE_ATT        7   //!< SET ONLY (W   uint8_t)
#define SNP_GGS_W_PERMIT_PRIVACY_FLAG_ATT      8   //!< RESERVED, DO NOT USED, need the stack to be build with GAP_PRIVACY and GAP_PRIVACY_RECONNECT, SET ONLY W   uint8_t

#define SNP_GAP_DEVICE_NAME_LEN                     21 // Excluding null-terminate char

// Privacy Flag States
#define SNP_GAP_PRIVACY_DISABLED            0x00
#define SNP_GAP_PRIVACY_ENABLED             0x01

/** @} End SNP_GAP_SERVICE */

//! \brief bitfield for the presence of the corresponding attribute in the
//  characteristic declaration message.
#define SNP_DESC_HEADER_GEN_SHORT_UUID      0x01
#define SNP_DESC_HEADER_GEN_LONG_UUID       0x02
#define SNP_DESC_HEADER_CCCD                0x04
#define SNP_DESC_HEADER_FORMAT              0x08
#define SNP_DESC_HEADER_USER_DESC           0x80
#define SNP_DESC_HEADER_UNSUPPORTED_MASK    0x70


#define SNP_GATT_CLIENT_CFG_NOTIFY          0x0001 //!< The Characteristic Value shall be notified
#define SNP_GATT_CLIENT_CFG_INDICATE        0x0002 //!< The Characteristic Value shall be indicated

#define SNP_RESPONSE_NOT_NEEDED             0x00
#define SNP_RESPONSE_NEEDED                 0x01

#define SNP_INVALID_HANDLE                  0x0000


/** @defgroup SNP_GAP_TERM_CONN_OPTION GAP GATT Terminate connection request option field
 * @{
 */
#define SNP_GAP_TERM_CONN_DEFAULT           0x00  //!< Default disconnection, send a packet OAD to tell the remote side to disconnect.
#define SNP_GAP_TERM_CONN_IMMEDIATLY        0x01  //!< abrupt disconnection: stop sending RF packet. The other side will timeout.
/** @} End SNP_GAP_TERM_CONN_OPTION */

/** @defgroup SNP_GAPBOND_PARAMS GAP Bond Manager parameters
 * @{
 */
#define SNP_GAPBOND_PAIRING_MODE            0x00  //!< Behavior on connections
#define SNP_GAPBOND_IO_CAPABILITIES         0x01  //!< IO Capabilities
#define SNP_GAPBOND_BONDING_ENABLED         0x02  //!< Toggle Bonding
#define SNP_GAPBOND_ERASE_ALLBONDS          0x03  //!< Erase all Bonds
#define SNP_GAPBOND_LRU_BOND_REPLACEMENT    0x04  //!< Overwrite Least Recently Used Bond when record is full.
/** @} End SNP_GAPBOND_PARAMS */

/** @defgroup SNP_SECURITY_STATE Security States
 * @{
 */
#define SNP_GAPBOND_PAIRING_STATE_COMPLETE   0x01  //!< Pairing complete
#define SNP_GAPBOND_PAIRING_STATE_BONDED     0x02  //!< Devices bonded
#define SNP_GAPBOND_PAIRING_STATE_BOND_SAVED 0x03  //!< Bonding record saved in NV
/** @} End SNP_SECURITY_STATE */

/*-------------------------------------------------------------------
* TYPEDEFS - Initialization and Configuration
*/
/*** Device Data Types ***/

//! @brief parameter structure for the Mask Event request
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t eventMask; //!< bitmask of Event to mask
                      //!
                      //! list of events available are listed here: @ref SNP_EVENT
} snpMaskEventReq_t;

//! @brief parameter structure for the Mask Event confirmation
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t maskedEvent; //!< Event that has been masked.
                        //!
                        //! list of events available are listed here: @ref SNP_EVENT
} snpMaskEventRsp_t;

//! @brief parameter structure for the test request confirmation
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t  memAlo;  //!< HEAP allocated memory
  uint16_t  memMax;  //!< maximum HEAP ever allocated since startup
  uint16_t  memSize; //!< total HEAP size
} snpTestCmdRsp_t;

//! @brief parameter structure for the HCI command. \n
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t  opcode; //!< HCI opcode to execute
                    //!
                    //! The list of available HCI opcode is listed here: @ref SNP_ALLOWED_HCI
  uint8_t   *pData; //!< parameters of the HCI opcode.
                    //!
                    //! Parameter depends of the HCI command being used. Those parameter are absolutely identical to the one define in TI HCI vendor guide.
} snpHciCmdReq_t;

//! @brief parameter structure for the HCI response
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status; //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t opcode; //!< HCI opcode that was executed in the request
  uint8_t  *pData; //!< @brief result parameters for this opcode.
                   //!
                   //! the expected result is exactly the same as the one describe in the HCI vendor guide
                   //! For the exception of the event Code not being sent code. Only the opcode reference the request is send back. (in the snpHciCmdRsp_t::opcode field)
} snpHciCmdRsp_t;

//! @brief parameter structure for the revision query response
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;            //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t snpVer;            //!< Version of the SNP (major, minor)
  uint8_t  stackBuildVer[10]; //!< Stack Revision see TI HCI vendor guide.
} snpGetRevisionRsp_t;

//! @brief parameter structure for the random number query response
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint32_t  rand;            //!< 32-bit Random Number generated by TRNG
} snpGetRandRsp_t;

//! @brief parameter structure for the status query response
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  gapRoleStatus;  //!< @brief current state of the GAP Role ( gaprole_States_t)
                           //!
                           //! Currently, the different possible state are: \n
                           //! \snippet peripheral.h GAP Peripheral Role State
  uint8_t  advStatus;      //!< @brief current state of advertising :
                           //!
                           //! TRUE= advertisement enable,  \n
                           //! FALSE = advertisement disable \n
  uint8_t  ATTstatus;      //!< @brief current state of the GATT server
                           //!
                           //! TRUE= operation ongoing, snpGetStatusCmdRsp_t::ATTmethod will indicate the currently running operation \n
                           //! FALSE = no operation \n
  uint8_t  ATTmethod;      //!< current GATT operation in progress if snpGetStatusCmdRsp_t::ATTstatus is set to TRUE
} snpGetStatusCmdRsp_t;

/*** GAP Data Types ***/

//! @brief parameter structure for the Start Advertising Request .
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  type;                //!< Type of Advertisement, see @ref SNP_AdvType
  uint16_t timeout;             //!< @brief Timeout of the advertisement.
                                //!
                                //! unit is in 625us period. value of 0 mean infinite advertisement.
  uint16_t interval;            //!< @brief Interval between advertisement event in 625us period.
                                //!
                                //! unit is in 625us period.
  uint8_t  filterPolicy;        //!< @brief RFU (Advertising filter policy)
  uint8_t  initiatorAddrType;   //!< @brief RFU (initiator address Type)
  uint8_t  initiatorAddress[6]; //!< @brief RFU (Initiator Address)
  uint8_t  behavior;            //!< behavior of Advertisement on connection event. see \ref SNP_AdvBehavior
} snpStartAdvReq_t;


//! @brief parameter structure for the Advertising Data Update Request.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t type;         //!< Type of Advertisement data to update, see \ref SNP_Advbuffer
  uint8_t *pData;       //!< Raw data for the advertisement data buffer to update.
} snpSetAdvDataReq_t;

//! @brief parameter structure for the Connection termination Request .
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle; //!< handle of the connection to terminate
  uint8_t  option;     //!< type of disconnection wanted: see @ref SNP_GAP_TERM_CONN_DEFAULT.
} snpTermConnReq_t;

//! @brief parameter structure for the Update Connection Parameter Request .
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;  //!< handle of the connection to update parameters to.
  uint16_t intervalMin; //!< @brief Minimum value for the connection event interval.
                        //!
                        //!< This shall be less than or equal to intervalMax.\n
                        //! Range: 0x0006 to 0x0C80 \n
                        //! Time = intervalMin * 1.25 msec \n
                        //! Time Range: 7.5 msec to 4 seconds.
  uint16_t intervalMax; ///< @brief  Maximum value for the connection event interval.
                        //!
                        /// This shall be greater than or equal to intervalMin.\n
                        //! Range: 0x0006 to 0x0C80 \n
                        //! Time = intervalMax * 1.25 msec \n
                        //! Time Range: 7.5 msec to 4 seconds.
  uint16_t slaveLatency; //!< @brief Slave latency for the connection in number of connection events.
                         //!
                         //! Range: 0x0000 to 0x01F3
  uint16_t supervisionTimeout; //!<  @brief Supervision timeout for the LE Link.
                               //!
                               //! Range: 0x000A to 0x0C80 \n
                               //! Time = connTimeout * 10 msec \n
                               //! Time Range: 100 msec to 32 seconds

} snpUpdateConnParamReq_t;

//! @brief parameter structure for the Update Connection Parameter Confirmation .
//! This does not mean parameters have been changed, only that the NP has successfully sent the request.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;      //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t connHandle;  //!< handle of the connection the parameter update was intended for..
} snpUpdateConnParamCnf_t;

//! @brief parameter structure for the Set and Get GAP Parameter Value Request.
//! This is also for the Set Security Parameter Request.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t paramId;  //!< GAP Parameter Id,
  uint16_t value;    //!< Parameter Value to get or set.
} snpSetGapParamReq_t, snpGetGapParamReq_t, snpSetSecParamReq_t;

//! @brief parameter structure for the Get GAP Parameter Value Response.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;      //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t paramId;     //!< GAP Parameter Id,
  uint16_t value;       //!< Parameter Value retrieve.
} snpGetGapParamRsp_t;

//!< @brief parameter structure for the Authentication data response.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint32_t authData;
} snpSetAuthDataReq_t;

/*** GATT Data Types ***/

//! @brief parameter structure for the Add new GATT Service Request .
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t type;         //!< Type of the service to add. see @ref SNP_SERVICE_TYPE
  uint8_t UUID[16];     //!< UUID of the service.
                        //!
                        //! UUID can be 2 bytes only.
                        //! at reception of this request, the length of the UUID can be deduce from the NPI frame length.
                        //! at transmission of this request, the length of UUId will need to be indicated outside of this structure.
} snpAddServiceReq_t;

//! @brief parameter structure used to add a new Characteristic Value Declaration.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  charValPerms;  //!< permissions of the value attribute, see @ref SNP_PERMIT_BITMAPS_DEFINES
  uint16_t charValProps;  //!< property of the value attribute, see @ref SNP_PROP_BITMAPS_DEFINES
  uint8_t  mgmtOption;    //!< Reserve for future used
  uint16_t charValMaxLen; //!< Reserve for future used Range: 0-512
                          //!
                          //! Since for now the characteristic value is managed only by the AP, this length field is not useful. Reserve for future use. \n
  uint8_t  UUID[16];      //! UUID of the characteristic. see @ref UUID_desc
                          //!
                          //! UUID can be 2 bytes only.
                          //! at reception of this request, the length of the UUID can be deduce from the NPI frame length.
                          //! at transmission of this request, the length of UUId will need to be indicated outside of this structure.
} snpAddCharValueDeclReq_t;

//! @brief parameter structure for the Add new Characteristic Value Declaration Response.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;       //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t attrHandle;  //!< attribute handle of the characteristic value.
} snpAddCharValueDeclRsp_t;

//! @brief parameter structure used to add a Client Characteristic Configuration Description Attribute.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t perms;        //!< permissions of the attribute, see @ref SNP_PERMIT_BITMAPS_DEFINES
} snpAddAttrCccd_t;

//! @brief parameter structure used to add a Format attribute.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  format;      //!<  Format, as describe by BLE spec, vol3, part G, chapter 3.3
  uint8_t  exponent;    //!<  exponent, as describe by BLE spec, vol3, part G, chapter 3.3
  uint16_t unit;        //!<  unit, as describe by BLE spec, vol3, part G, chapter 3.3
  uint8_t  name_space;   //!<  namespace, as describe by BLE spec, vol3, part G, chapter 3.3
  uint16_t desc;        //!<  desc, as describe by BLE spec, vol3, part G, chapter 3.3
} snpAddAttrFormat_t;

//! @brief parameter structure used to add a User Descriptor attribute.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  perms;       //!<  permissions of the attribute, see @ref SNP_PERMIT_BITMAPS_DEFINES, force to Read only
  uint16_t maxLen;      //!<  Maximum possible length of the string, plus one byte for null terminator (range from 1 to 512)
  uint16_t initLen;     //!<  initial length of the string, plus one byte for null terminator (must be <= snpAddAttrUserDesc_t::maxLen)
  uint8_t  *pDesc;      //!<  initial string (must be null terminated)
} snpAddAttrUserDesc_t;

//! @brief parameter structure used to add a generic attribute descriptor (short UUID)
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  perms;       //!< permissions of the attribute, see @ref SNP_PERMIT_BITMAPS_DEFINES, force to Read only
  uint16_t maxLen;      //!< Reserve for future used Range: 0-512
                        //!
                        //!  Since for now the characteristic value is managed only by the AP, this length field is not useful. Reserve for future use. \n
  uint8_t  UUID[2];     //!< UUID of the attribute. see @ref UUID_desc
} snpAddAttrGenShortUUID_t;

//! @brief parameter structure used to add a generic attribute descriptor (long UUID)
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  perms;       //!< permissions of the attribute, see @ref SNP_PERMIT_BITMAPS_DEFINES, force to Read only
  uint16_t maxLen;      //!< Reserve for future used Range: 0-512
                        //!
                        //!  Since for now the characteristic value is managed only by the AP, this length field is not useful. Reserve for future use. \n
  uint8_t  UUID[16];    //!< UUID of the attribute. see @ref UUID_desc
} snpAddAttrGenLongUUID_t;

//! @brief parameter structure for the Add new Characteristic Description Request.
//! 'This packet structure is not used to create a NPI frame'
PACKED_TYPEDEF_STRUCT
{
  uint8_t                       header; //!< Header bit field of request. Defines which attributes are to be added
  snpAddAttrGenShortUUID_t      *pShortUUID;
  snpAddAttrGenLongUUID_t       *pLongUUID;
  snpAddAttrCccd_t              *pCCCD;
  snpAddAttrFormat_t            *pFormat;
  snpAddAttrUserDesc_t          *pUserDesc;
} snpAddCharDescDeclReq_t;

// Add new Characteristic Description Response.
//! 'This packet structure is not used to create a NPI frame'
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;       //!< Header bit field of request. Defines which attributes are to be added
  uint8_t  header;       //!< Status of response
  uint16_t handles[6];   //!< Maximum handles returned is 6. One for each possible attribute added. Header will determine the number of actual handles returned
} snpAddCharDescDeclRsp_t;

//! @brief parameter structure for the Register Service Response.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;      //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t startHandle; //!< first attribute handle of the registered service
  uint16_t endHandle;   //!< last attribute handle of the registered service
} snpRegisterServiceRsp_t;

//! @brief parameter structure for the Characteristic Read Request Indication data.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;  //!< handle of the connection
  uint16_t attrHandle;  //!< handle of the characteristic value attribute being read
  uint16_t offset;      //!< Offset of the characteristic to start reading from
  uint16_t maxSize;     //!< Max Size of the data to read
} snpCharReadInd_t;

//! @brief parameter structure for the Characteristic Read Request Confirmation and
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;      //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t connHandle;  //!< handle of the connection
  uint16_t attrHandle;  //!< handle of the characteristic value attribute being read
  uint16_t offset;      //!< Offset of the characteristic to start reading from
  uint8_t *pData;       //!< Characteristic value data, from offset to offset+size-1.
} snpCharReadCnf_t;

//! @brief parameter structure for the Characteristic Write Request Indication data.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle; //!< handle of the connection
  uint16_t attrHandle; //!< handle of the characteristic value attribute being written
  uint8_t  rspNeeded;  //!< indicate if the AP must answer this indication or not
  uint16_t offset;     //!< Offset of the characteristic to start writing to
  uint8_t *pData;      //!< data to write to the characteristic value, from offset to size of data (deduce from NPI frame length)
} snpCharWriteInd_t;

//! @brief parameter structure for the response to Char right indication, Char config update, or Notification indication.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;      //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t connHandle;  //!< handle of the connection
} snpCharWriteCnf_t, snpCharCfgUpdatedRsp_t,  snpNotifIndCnf_t ;

//! @brief parameter structure for the Characteristic Notification and Indication Request data.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;  //!< handle of the connection
  uint16_t attrHandle;  //!< handle of the characteristic value attribute to notify/indicate
  uint8_t authenticate; //!< RFU
  uint8_t type;         //!< Type of the request, see @ref SNP_IND_TYPE
  uint8_t *pData;
} snpNotifIndReq_t;


//! @brief parameter structure for the :
//! Add Service response
//! Write responses
//! notifInd responses
//! Set Security Param response
//! Set Authentication Data response
//! Set White List Filter Policy response
//! setGATTParam responses.Characteristic Notification and Indication Request data.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t status;       //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
}snpSetGattParamRsp_t, snpSetAdvDataCnf_t, snpSetGapParamRsp_t,
snpAddServiceRsp_t, snpSetSecParamRsp_t, snpSetAuthDataRsp_t,
snpSetWhiteListRsp_t;

//! @brief parameter structure for the Set GATT Service Parameter from a specified NP Service Request and
//! Get GATT Service Parameter from a specified NP Service Response data.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t serviceID;    //!< Id of the service containing the parameter to define
                        //!
                        //! existing list of service can be found here: see  @ref SNP_PREDEF_SERVICE
  uint8_t paramID;      //!< Id of the parameter of the service to update/retrieved
  uint8_t *pData;       //!< data to write/retrieved.
} snpSetGattParamReq_t, snpGetGattParamRsp_t;

//! @brief parameter structure for the Set Attribute Value request.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrHandle;  //!< attribute handle of the characteristic value.
  uint8_t *pData;      //!< data of the value to set.
} snpSetAttrValueReq_t;

//! @brief parameter structure for the Set Attribute Value
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;       //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t attrHandle;  //!< attribute handle of the characteristic value.
} snpSetAttrValueRsp_t;

//! @brief parameter structure for the Get Attribute Value request
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t attrHandle;   //!< attribute handle of the characteristic value.
} snpGetAttrValueReq_t;

//! @brief parameter structure for the Get Attribute Value request response
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t  status;        //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
  uint16_t attrHandle;   //!< attribute handle of the characteristic value.
  uint8_t  *pData;       //!< data of the value to set.
} snpGetAttrValueRsp_t;

//! @brief parameter structure for the Get GATT Service Parameter from a specified NP Service Request.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t serviceID;    //!< Id of the service containing the parameter to define
  uint8_t paramID;      //!< Id of the parameter to retrieved.
} snpGetGattParamReq_t;

//! @brief parameter structure for the Characteristic Configuration Update Indication .
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t  connHandle;   //!< handle of the connection
  uint16_t  cccdHandle;   //!< handle of the characteristic value attribute to notify/indicate
  uint8_t   rspNeeded;    //!< indicate if the AP must answer this indication or not
  uint16_t  value;        //!< value to write to the CCCD
} snpCharCfgUpdatedInd_t;

//! @brief paramteer structure for the White Listing.
//! This is a packed structure.  see @ref TL_Parmaeter for more information
PACKED_TYPEDEF_STRUCT
{
  uint8_t useWhiteList; //!< @ref SNP_AdvFilterPolicyType
} snpSetWhiteListReq_t;

// The SNP message.  A union of all SNP message types.
typedef union
{
  //Mask events
  snpMaskEventReq_t        maskEventReq;        //!< event mask request.
  snpMaskEventRsp_t        maskEventCnf;        //!< event mask confirmation.

  //Revision
  snpGetRevisionRsp_t      revisionRsp;         //!< Get revision Response.

  //Rand
  snpGetRandRsp_t          randRsp;             //!< Get Random Number Response

  //HCI
  snpHciCmdReq_t           hciCmdReq;           //!< HCI command request
  snpHciCmdRsp_t           hciCmdRsp;           //!< HCI command response (event)

  //Status
  snpGetStatusCmdRsp_t     getStatusRsp;        //!< Get status Response

  //Test
  snpTestCmdRsp_t          testCmdRsp;          //!< Test command response

  //Advertising
  snpStartAdvReq_t         startAdv;            //!< Start advertising request.
  snpSetAdvDataReq_t       setAdvDataReq;       //!< Set advertising data Request.
  snpSetAdvDataCnf_t       setAdvDataCnf;       //!< Set advertising data Response.

  //Connection
  snpTermConnReq_t         connTermReq;         //!< Connection terminated Request.
  snpUpdateConnParamReq_t  updateConnParamsReq; //!< Update connection parameters indication.
  snpUpdateConnParamCnf_t  updateConnParamsCnf; //!< Update connection parameters, request.

  //GAP Parameters
  snpSetGapParamReq_t      setGapParamReq;      //!< Set GAP parameter Request.
  snpSetGapParamRsp_t      setGapParamRsp;      //!< Set GAP parameter Response.
  snpGetGapParamReq_t      getGapParamReq;      //!< Get GAP parameter Request.
  snpGetGapParamRsp_t      getGapParamRsp;      //!< Get GAP parameter Response.

  //Security
  snpSetSecParamReq_t      setSecParamReq;      //!< Set the Security Requirements request.
  snpSetSecParamRsp_t      setSecParamRsp;      //!< Set Security Requirements response.
  snpSetAuthDataReq_t      setAuthDataReq;      //!< Set Authentication Data request.
  snpSetAuthDataRsp_t      setAuthDataRsp;      //!< Set Authentication Data response.

  //White List
  snpSetWhiteListReq_t     setWhiteListReq;    //!< Set the White List Filter Policy request.
  snpSetWhiteListRsp_t     setWhiteListRsp;    //!< Set the White List Filter Policy response.

  //Services
  snpAddServiceReq_t       addServiceReq;       //!< Add service request.
  snpAddServiceRsp_t       addServiceRsp;       //!< Add service response.
  snpAddCharValueDeclReq_t addCharValueDecReq;  //!< Add characteristic value Request.
  snpAddCharValueDeclRsp_t addCharValueDecRsp;  //!< Add characteristic value Response.
  snpAddCharDescDeclReq_t  addCharDescDeclReq;  //!< Add characteristic descriptor Request.
  snpAddCharDescDeclRsp_t  addCharDescDeclRsp;  //!< Add characteristic descriptor Response.
  snpRegisterServiceRsp_t  registerService;     //!< Register service Response.

  //Incoming GATT Requests
  snpCharReadInd_t         readInd;             //!< Characteristic read request.
  snpCharReadCnf_t         readCnf;             //!< Characteristic read confirmation.
  snpCharWriteInd_t        charWriteInd;        //!< Characteristic write request.
  snpCharWriteCnf_t        charWriteCnf;        //!< Characteristic write confirmation.
  snpNotifIndReq_t         notifIndReq;         //!< Notification or indication request
  snpNotifIndCnf_t         notifIndCnf;         //!< Indication Confirmation.

  //NP GATT Parameters of predefined services
  snpSetGattParamReq_t     setGattParamReq;     //!< Set GATT parameter request.
  snpSetGattParamRsp_t     setGattParamRsp;     //!< Set GATT parameter response.
  snpGetGattParamReq_t     getGattParamReq;     //!< Get GATT parameter response.
  snpGetGattParamRsp_t     getGattParamRsp;     //!< Get GATT parameter request.

  //CCCD Updates
  snpCharCfgUpdatedInd_t   charCfgUpdatedReq;   //!< Characteristic configuration updated indication.
  snpCharCfgUpdatedRsp_t   charcfgUpdatedRsp;   //!< Characteristic configuration update response.
} snp_msg_t;

/*** SNP EVENTS ***/


//! @brief parameter structure for the Connection Established Event
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;          //!< handle of the connection
  uint16_t connInterval;        //!< @see snpUpdateConnParamReq_t::connInterval
  uint16_t slaveLatency;        //!< @see snpUpdateConnParamReq_t::slaveLatency
  uint16_t supervisionTimeout;  //!< @see snpUpdateConnParamReq_t::supervisionTimeout
  uint8_t  addressType;         //!< Type of initiator address
  uint8_t  pAddr[6];            //!< address of the initiator
} snpConnEstEvt_t;

//! @brief parameter structure for the Connection terminated Event.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;          //!< handle of the connection
  uint8_t  reason;              //!< Reason of the termination
} snpConnTermEvt_t;

//! @brief parameter structure for the Update Connection Parameter Event.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;          //!< handle of the connection
  uint16_t connInterval;        //!< @see snpUpdateConnParamReq_t::connInterval
  uint16_t slaveLatency;        //!< @see snpUpdateConnParamReq_t::slaveLatency
  uint16_t supervisionTimeout;  //!< @see snpUpdateConnParamReq_t::supervisionTimeout
} snpUpdateConnParamEvt_t;

//! @brief parameter structure for the Advertising state change Event.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t status;
} snpAdvStatusEvt_t;

//! @brief parameter structure for the Security Event.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t state;  //!< new Security State @ref SNP_SECURITY_STATE
  uint8_t status; //!< status upon entering this state
} snpSecurityEvt_t;

//! @brief parameter structure for the Error Event.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t opcode;       //!< SNP opcode that triggered the error
  uint8_t  status;       //!< status of the request (SUCCESS or Error, @ref SNP_ERRORS)
} snpErrorEvt_t;

//! @brief parameter structure for the ATT MTU size updated Event data
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint16_t connHandle;  //!< handle of the connection
  uint16_t attMtuSize;  //!< New ATT MTU Size negotiated between GATT client and GATT server.
} snpATTMTUSizeEvt_t;

//! @brief This event is recieved when MITM Authentication data must be dislayed
//! or input by the user.
//! This is a packed structure. see @ref TL_Parameter for more information.
PACKED_TYPEDEF_STRUCT
{
  uint8_t display; //!< TRUE if device must display passcode.
  uint8_t input;   //!< TRUE if device must input passcode.
  uint32_t numCmp; //!< Numeric Comparison value to display, 0 if not used.
} snpAuthenticationEvt_t;

/* SNP Event Parameters */
typedef union
{
  snpConnEstEvt_t         connEstEvt;
  snpConnTermEvt_t        connTermEvt;
  snpUpdateConnParamEvt_t updateConnParamEvt;
  snpAdvStatusEvt_t       advStatusEvt;
  snpATTMTUSizeEvt_t      attMTUSizeEvt;
  snpErrorEvt_t           advErrorEvt;
  snpSecurityEvt_t        securityEvt;
  snpAuthenticationEvt_t  authenticationEvt;
} snpEventParam_t;

/* SNP Event structure */
PACKED_TYPEDEF_STRUCT
{
  uint16_t        event;
  snpEventParam_t *pEvtParams;
} snpEvt_t;

/**
 * Connection parameters for the peripheral device.  These numbers are used
 * to compare against connection events and request connection parameter
 * updates with the master.
 * this structure is used to update the parameter #SNP_GGS_PERI_CONN_PARAM_ATT of the GAP/GATT service.
 */
typedef struct
{
  uint16_t intervalMin; //!< @see snpUpdateConnParamReq_t::connInterval
  uint16_t intervalMax; //!< @see snpUpdateConnParamReq_t::connInterval
  uint16_t latency;     //!< @see snpUpdateConnParamReq_t::slaveLatency
  uint16_t timeout;     //!< @see snpUpdateConnParamReq_t::supervisionTimeout
} snpGapPeriConnectParams_t;

/*********************************************************************
 * MACROS
 */
//! \brief Macro to retrieve the HDR bitfield from CMD1
#define SNP_CMD1_HDR_MASK               0xC0
#define SNP_CMD1_HDR_POS                0x6
#define SNP_GET_OPCODE_HDR_CMD1(cmd)    (((cmd) & SNP_CMD1_HDR_MASK) >>  SNP_CMD1_HDR_POS)

//! \brief Macro to retrieve the ID bitfield from CMD1
#define SNP_CMD1_ID_MASK                0x3F
#define SNP_GET_OPCODE_ID_CMD1(cmd)     ((cmd) & SNP_CMD1_ID_MASK)

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SNP_H */
