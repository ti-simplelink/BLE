
#ifndef BLE_EVENT_HANDLING_H
#define BLE_EVENT_HANDLING_H

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>

#include "ti/sap/snp.h"

/*
 * Event_pend timeout set in units of ticks. Tick period is microseconds,
 * so this evaluates to 1 second.
 */
#define AP_EVENT_PEND_TIMEOUT                (1000000/Clock_tickPeriod)

// Event ID's are integers
#define AP_NONE                    Event_Id_NONE // No Event
#define AP_EVT_PUI                 Event_Id_00   // Power-Up Indication
#define AP_EVT_ADV_ENB             Event_Id_01   // Advertisement Enabled
#define AP_EVT_ADV_END             Event_Id_02   // Advertisement Ended
#define AP_EVT_ADV_DATA_RSP        Event_Id_03   // Advertisement Data Set Response
#define AP_EVT_CONN_EST            Event_Id_04   // Connection Established
#define AP_EVT_CONN_TERM           Event_Id_05   // Connection Terminated
#define AP_EVT_HCI_RSP             Event_Id_06   // HCI Command Response
#define AP_EVT_TEST_RSP            Event_Id_07   // Test Command Response
#define AP_EVT_CONN_PARAMS_UPDATED Event_Id_08   // Connection Parameters Updated
#define AP_EVT_CONN_PARAMS_CNF     Event_Id_09   // Connection Parameters Request Confirmation
#define AP_EVT_NOTIF_IND_RSP       Event_Id_10   // Notification/Indication Response
#define AP_EVT_HANDLE_AUTH_EVT     Event_Id_11   // Authentication Required Event
#define AP_EVT_AUTH_RSP            Event_Id_12   // Set Authentication Data Response
#define AP_EVT_SECURITY_STATE      Event_Id_13   // Security State Changed
#define AP_EVT_SECURITY_PARAM_RSP  Event_Id_14   // Set Security Param Response
#define AP_EVT_WHITE_LIST_RSP      Event_Id_15   // Set White List Policy Response
#define AP_EVT_NUM_CMP_BTN         Event_Id_16   // Numeric Comparison Button Press
#define AP_EVT_COPIED_ASYNC_DATA   Event_Id_30   // Copied Data From asyncRspData
#define AP_ERROR                   Event_Id_31   // Error

extern Event_Handle apEvent;
extern snp_msg_t *asyncRspData;
extern uint16_t _connHandle;
extern bool connected;
extern bool advertising;
extern snpEventParam_t eventHandlerData;

void AP_asyncCB(uint8_t cmd1, void *pParams);
void processSNPEventCB(uint16_t event, snpEventParam_t *param);
bool apEventPend(uint32_t event);
bool isError(uint8_t status);

#endif
