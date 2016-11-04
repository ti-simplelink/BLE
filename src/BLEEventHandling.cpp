
#include <BLE.h>
#include "BLEEventHandling.h"
#include "BLELog.h"
#include "BLEServiceList.h"

/* Global event all event handling. */
Event_Handle apEvent = NULL;

/* Used to pass data from an async response back to a user call (HCI, test). */
snp_msg_t *asyncRspData = NULL;

/* Stores the connection handle. Should always be 0. */
uint16_t _connHandle = -1;

/* State variable for whether the device is connected. */
bool connected = false;

/* State variable for whether the device is advertising. */
bool advertising = false;

/* Used to pass event data to the handleEvents() function. */
snpEventParam_t eventHandlerData = {};

/* Interrupt for the button that indicates an equal numeric comparison. */
static void numCmpInterruptEqual(void);

/* Interrupt for the button that indicates a different numeric comparison. */
static void numCmpInterruptDifferent(void);

/* Helper function for posting AP_ERROR, setting error, and logging. */
static void apPostError(uint8_t status, const char errMsg[]);

/*
 * Must be called in the main loop to poll for events that must be
 * handled outside of the NPI task. Required for sending NPI messages
 * or human interaction (e.g. button presses for security).
 */
int BLE::handleEvents(void)
{
  /*
   * This logRelease and the logAcquire at the end of this function ensure
   * that the NPI task gets to log every main loop.
   */
  logRelease();
  uint32_t events = AP_EVT_HANDLE_AUTH_EVT | AP_EVT_NUM_CMP_BTN;
  opcode = Event_pend(apEvent, AP_NONE, events, 1);
  int status = BLE_SUCCESS;
  if (opcode & AP_EVT_HANDLE_AUTH_EVT)
  {
    snpAuthenticationEvt_t *evt = (snpAuthenticationEvt_t *) &eventHandlerData;
    if (evt->numCmp)
    {
      handleNumCmp(evt);
    }
    else if (evt->display)
    {
      if (isError(handleAuthKey(evt)))
      {
        status = BLE_CHECK_ERROR;
      }
    }
  }
  if (opcode & AP_EVT_NUM_CMP_BTN)
  {
    /* User pressed a button for numeric comparison. */
    detachInterrupt(PUSH1);
    detachInterrupt(PUSH2);
    logRPC("Send num cmp rsp");
    logRelease();
    if (isError(SAP_setAuthenticationRsp(authKey)) ||
        !apEventPend(AP_EVT_AUTH_RSP))
    {
      status = BLE_CHECK_ERROR;
    }
  }
  logAcquire();
  return status;
}

/*
 * Passkey security handling. Generates a random 6 digit passkey, displays it,
 * and sends it to the SNP if necessary.
 */
int BLE::handleAuthKey(snpAuthenticationEvt_t *evt)
{
  authKey = getRand() % 1000000;
  if (displayStringFxn && displayUIntFxn)
  {
    displayStringFxn("Auth key:");
    displayUIntFxn(authKey);
    displayStringFxn("\n");
  }
  else if (Serial)
  {
    Serial.print("Auth key:");
    Serial.println(authKey);
  }
  /*
   * evt->input is set when SNP requests the key is sent to it. Not sure of a
   * case when this wouldn't always be needed unless a keyboard was somehow
   * connected.
   */
  if (evt->input)
  {
    logRPC("Send auth key");
    logRelease();
    if (isError(SAP_setAuthenticationRsp(authKey)) ||
      !apEventPend(AP_EVT_AUTH_RSP))
    {
      return BLE_CHECK_ERROR;
    }
  }
  return BLE_SUCCESS;
}

/*
 * Numeric comparison security handling. Displays the number sent from the
 * SNP. If requested, the user inputs with the buttons whether this number
 * and the one displayed on the client are the same.
 */
void BLE::handleNumCmp(snpAuthenticationEvt_t *evt)
{
  if (displayStringFxn && displayUIntFxn)
  {
    displayStringFxn("Check if equal:");
    displayUIntFxn(evt->numCmp);
    displayStringFxn("\n");
  }
  else if (Serial)
  {
    Serial.print("Check if equal:");
    Serial.println(evt->numCmp);
  }
  /*
   * If user feedback is requested, setup interrupts on buttons to respond to
   * the user in the function handleEvents().
   */
  if (evt->input)
  {
    if (displayStringFxn && displayUIntFxn)
    {
      displayStringFxn("Press button1 if equal, button2 if not.");
      displayStringFxn("\n");
    }
    else if (Serial)
    {
      Serial.println("Press button1 if equal, button2 if not.");
    }
    pinMode(PUSH1, INPUT_PULLUP);
    attachInterrupt(PUSH1, numCmpInterruptEqual, FALLING);
    pinMode(PUSH2, INPUT_PULLUP);
    attachInterrupt(PUSH2, numCmpInterruptDifferent, FALLING);
  }
}

/* Send true if numbers are equal, false if different */
static void numCmpInterruptEqual(void)
{
  ble.authKey = 1;
  Event_post(apEvent, AP_EVT_NUM_CMP_BTN);
}

static void numCmpInterruptDifferent(void)
{
  ble.authKey = 0;
  Event_post(apEvent, AP_EVT_NUM_CMP_BTN);
}

/*
 * Even though many events and resposes are asynchronous, we still handle them
 * synchronously. Any request that generates an asynchronous response should
 * Event_pend on the corresponding Event_post here.
 * There is one case for each cmd1 and only one case per call should run.
 */
void AP_asyncCB(uint8_t cmd1, void *pParams)
{
  switch (SNP_GET_OPCODE_HDR_CMD1(cmd1))
  {
    case SNP_DEVICE_GRP:
    {
      switch (cmd1)
      {
        case SNP_POWER_UP_IND:
        {
          logAsync("SNP_POWER_UP_IND", cmd1);
          // Notify state machine of Power Up Indication
          // Log_info0("Got PowerUp indication from NP");
          Event_post(apEvent, AP_EVT_PUI);
        } break;
        case SNP_HCI_CMD_RSP:
        {
          logAsync("SNP_HCI_CMD_RSP", cmd1);
          snpHciCmdRsp_t *hciRsp = (snpHciCmdRsp_t *) pParams;
          ble.opcode = hciRsp->opcode;
          logParam("opcode", hciRsp->opcode);
          if (hciRsp->status == SNP_SUCCESS)
          {
            asyncRspData = (snp_msg_t *) hciRsp;
            Event_post(apEvent, AP_EVT_HCI_RSP);
            Event_pend(apEvent, AP_NONE, AP_EVT_COPIED_ASYNC_DATA,
                       AP_EVENT_PEND_TIMEOUT);
          }
          else
          {
            apPostError(hciRsp->status, "SNP_HCI_CMD_RSP");
          }
        } break;
        case SNP_TEST_RSP:
        {
          logAsync("SNP_TEST_RSP", cmd1);
          snpTestCmdRsp_t *testRsp = (snpTestCmdRsp_t *) pParams;
          asyncRspData = (snp_msg_t *) testRsp;
          logParam("memAlo", testRsp->memAlo);
          logParam("memMax", testRsp->memMax);
          logParam("memSize", testRsp->memSize);
          Event_post(apEvent, AP_EVT_TEST_RSP);
          Event_pend(apEvent, AP_NONE, AP_EVT_COPIED_ASYNC_DATA,
                     AP_EVENT_PEND_TIMEOUT);
          // No status code in response
        } break;
        default:
          break;
      }
    } break;
    case SNP_GAP_GRP:
    {
      switch (cmd1)
      {
        case SNP_SET_ADV_DATA_CNF:
        {
          logAsync("SNP_SET_ADV_DATA_CNF", cmd1);
          snpSetAdvDataCnf_t *advDataRsp = (snpSetAdvDataCnf_t *) pParams;
          if (advDataRsp->status == SNP_SUCCESS)
          {
            Event_post(apEvent, AP_EVT_ADV_DATA_RSP);
          }
          else
          {
            apPostError(advDataRsp->status, "SNP_SET_ADV_DATA_CNF");
          }
        } break;
        // Just a confirmation that the request update was sent.
        case SNP_UPDATE_CONN_PARAM_CNF:
        {
          logAsync("SNP_UPDATE_CONN_PARAM_CNF", cmd1);
          snpUpdateConnParamCnf_t *connRsp =
            (snpUpdateConnParamCnf_t *) pParams;
          if (connRsp->status == SNP_SUCCESS)
          {
            Event_post(apEvent, AP_EVT_CONN_PARAMS_CNF);
          }
          else
          {
            apPostError(connRsp->status, "SNP_UPDATE_CONN_PARAM_CNF");
          }
        } break;
        case SNP_SEND_AUTHENTICATION_DATA_RSP:
        {
          logAsync("SNP_SEND_AUTHENTICATION_DATA_RSP", cmd1);
          snpSetAuthDataRsp_t *authRsp = (snpSetAuthDataRsp_t *) pParams;
          if (authRsp->status == SNP_SUCCESS)
          {
            Event_post(apEvent, AP_EVT_AUTH_RSP);
          }
          else
          {
            apPostError(authRsp->status, "SNP_SEND_AUTHENTICATION_DATA_RSP");
          }
        }
        case SNP_SET_WHITE_LIST_POLICY_RSP:
        {
          logAsync("SNP_SET_WHITE_LIST_POLICY_RSP", cmd1);
          snpSetWhiteListRsp_t *whiteListRsp = (snpSetWhiteListRsp_t *) pParams;
          if (whiteListRsp->status == SNP_SUCCESS)
          {
            Event_post(apEvent, AP_EVT_WHITE_LIST_RSP);
          }
          else
          {
            apPostError(whiteListRsp->status, "SNP_SET_WHITE_LIST_POLICY_RSP");
          }
        }
        default:
          break;
      }
    }
    case SNP_GATT_GRP:
    {
      switch (cmd1)
      {
        case SNP_SEND_NOTIF_IND_CNF:
        {
          logAsync("SNP_SEND_NOTIF_IND_CNF", cmd1);
          snpNotifIndCnf_t *notifIndRsp = (snpNotifIndCnf_t *) pParams;
          if (notifIndRsp->status == SNP_SUCCESS)
          {
            Event_post(apEvent, AP_EVT_NOTIF_IND_RSP);
          }
          else
          {
            apPostError(notifIndRsp->status, "SNP_SEND_NOTIF_IND_CNF");
          }
        } break;
        default:
          break;
      }
    }
    default:
      break;
  }
  logRelease();
}

/* Handles events encapsulated by the SNP message with opcode 0x05. */
void processSNPEventCB(uint16_t cmd1, snpEventParam_t *param)
{
  switch (cmd1)
  {
    case SNP_CONN_EST_EVT:
    {
      logAsync("SNP_CONN_EST_EVT", cmd1);
      snpConnEstEvt_t *evt = (snpConnEstEvt_t *) param;
      _connHandle                           = evt->connHandle;
      ble.usedConnParams.connInterval       = evt->connInterval;
      ble.usedConnParams.slaveLatency       = evt->slaveLatency;
      ble.usedConnParams.supervisionTimeout = evt->supervisionTimeout;
      logParam("connInterval", evt->connInterval);
      logParam("slaveLatency", evt->slaveLatency);
      logParam("supervisionTimeout", evt->supervisionTimeout);
      memcpy(&ble.bleAddr, &(evt->pAddr), sizeof(evt->pAddr));
      connected = true;
      Event_post(apEvent, AP_EVT_CONN_EST);
    } break;
    case SNP_CONN_TERM_EVT:
    {
      logAsync("SNP_CONN_TERM_EVT", cmd1);
      connected = false;
      Event_post(apEvent, AP_EVT_CONN_TERM);
      BLE_resetCCCD();
    } break;
    case SNP_CONN_PARAM_UPDATED_EVT:
    {
      /* Update parameters stored in ble.usedConnParams. */
      logAsync("SNP_CONN_PARAM_UPDATED_EVT", cmd1);
      snpUpdateConnParamEvt_t *evt = (snpUpdateConnParamEvt_t *) param;
      /* Log only changed parameters. */
      if (ble.usedConnParams.connInterval != evt->connInterval)
      {
        logParam("connInterval", evt->connInterval);
      }
      if (ble.usedConnParams.slaveLatency != evt->slaveLatency)
      {
        logParam("slaveLatency", evt->slaveLatency);
      }
      if (ble.usedConnParams.supervisionTimeout != evt->supervisionTimeout)
      {
        logParam("supervisionTimeout", evt->supervisionTimeout);
      }
      ble.usedConnParams.connInterval       = evt->connInterval;
      ble.usedConnParams.slaveLatency       = evt->slaveLatency;
      ble.usedConnParams.supervisionTimeout = evt->supervisionTimeout;
      Event_post(apEvent, AP_EVT_CONN_PARAMS_UPDATED);
    } break;
    case SNP_ADV_STARTED_EVT:
    {
      logAsync("SNP_ADV_STARTED_EVT", cmd1);
      snpAdvStatusEvt_t *evt = (snpAdvStatusEvt_t *) param;
      if (evt->status == SNP_SUCCESS)
      {
        advertising = true;
        Event_post(apEvent, AP_EVT_ADV_ENB);
      }
      else
      {
        apPostError(evt->status, "SNP_ADV_STARTED_EVT");
      }
    } break;
    case SNP_ADV_ENDED_EVT:
    {
      logAsync("SNP_ADV_ENDED_EVT", cmd1);
      snpAdvStatusEvt_t *evt = (snpAdvStatusEvt_t *) param;
      if (evt->status == SNP_SUCCESS) {
        advertising = false;
        Event_post(apEvent, AP_EVT_ADV_END);
      }
      else
      {
        apPostError(evt->status, "SNP_ADV_ENDED_EVT");
      }
    } break;
    case SNP_ATT_MTU_EVT:
    {
      logAsync("SNP_ATT_MTU_EVT", cmd1);
      snpATTMTUSizeEvt_t *evt = (snpATTMTUSizeEvt_t *) param;
      ble.mtu = evt->attMtuSize - 3; // -3 for non-user data
      logParam("mtu", ble.mtu);
    } break;
    case SNP_SECURITY_EVT:
    {
      logAsync("SNP_SECURITY_EVT", cmd1);
      snpSecurityEvt_t *evt = (snpSecurityEvt_t *) param;
      ble.securityState = evt->state;
      logParam("state", evt->state);
      if (evt->status == SNP_SUCCESS) {
        Event_post(apEvent, AP_EVT_SECURITY_STATE);
      }
      else
      {
        apPostError(evt->status, "SNP_SECURITY_EVT");
      }
    } break;
    case SNP_AUTHENTICATION_EVT:
    {
      logAsync("SNP_AUTHENTICATION_EVT", cmd1);
      memcpy(&eventHandlerData, param, sizeof(eventHandlerData));
      Event_post(apEvent, AP_EVT_HANDLE_AUTH_EVT);
    } break;
    case SNP_ERROR_EVT:
    {
      logAsync("SNP_ERROR_EVT", cmd1);
      snpErrorEvt_t *evt = (snpErrorEvt_t *) param;
      ble.opcode = evt->opcode;
      logParam("Opcode", evt->opcode);
      apPostError(evt->status, "SNP_ERROR_EVT");
    } break;
  }
  logRelease();
}

/*
 * Can't return specific error beacuse that's only in the async handler,
 * so we return true/false and let caller check ble.error.
 */
bool apEventPend(uint32_t event)
{
  ble.error = BLE_SUCCESS;
  uint32_t postedEvent = Event_pend(apEvent, AP_NONE, event | AP_ERROR,
                                    AP_EVENT_PEND_TIMEOUT);
  /* Bug in NPI causes this specific event to get posted twice */
  if (event == AP_EVT_ADV_DATA_RSP)
  {
    Event_pend(apEvent, AP_NONE, event | AP_ERROR, AP_EVENT_PEND_TIMEOUT);
  }
  bool status = false;
  if (postedEvent & event)
  {
    status = true;
  }
  else if (postedEvent == 0)
  {
    ble.error = BLE_TIMEOUT;
    status = false;
  }
  else if (postedEvent & AP_ERROR)
  {
    // Function that posts AP_ERROR should set ble.error
    status = false;
  }
  return status;
}

static void apPostError(uint8_t status, const char errMsg[])
{
  logError(errMsg, status);
  logRelease();
  ble.error = status;
  Event_post(apEvent, AP_ERROR);
}

/*
 * Handles propogating errors through stack to Energia sketch. Use when
 * failure of the checked call requires immediate return (e.g. if the
 * next statements depend on it). This preserves ble.error when it already
 * has been set to something besides BLE_SUCCESS, and otherwise sets it
 * to status.
 */
bool isError(uint8_t status)
{
  if (status == BLE_CHECK_ERROR)
  {
    return true;
  }
  else if ((ble.error = status) != SNP_SUCCESS)
  {
    logError(status);
    logRelease();
    return true;
  }
  return false;
}
