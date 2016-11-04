
#ifndef BLE_H
#define BLE_H

#include <Energia.h>
#include "Stream.h"

#include "BLETypes.h"
#include "BLEServices.h"

class BLE : public Stream
{
  private:
    uint8_t _portType; // UART or SPI connection with network processor
    uint8_t *advertDataArr[MAX_ADVERT_IDX];

    int resetPublicMembers(void);
    uint8_t advertDataInit(void);
    int setAdvertName(uint8_t advertNameLen, const char *advertName);
    int setSingleConnParam(size_t offset, uint16_t value);
    int apCharWriteValue(BLE_Char *bleChar, void *pData,
                         size_t size, bool isBigEnd);
    uint8_t writeNotifInd(BLE_Char *bleChar);
    uint8_t readValueValidateSize(BLE_Char *bleChar, size_t size);
    int writeValue(BLE_Char *bleChar, const char *str, int len);
    int setSecurityParam(uint16_t paramId, uint16_t len, uint8_t *pData);
    int handleAuthKey(snpAuthenticationEvt_t *evt); // BLEEventHandling.cpp
    void handleNumCmp(snpAuthenticationEvt_t *evt); // BLEEventHandling.cpp

  public:
    int error; // Set to BLE_SUCCESS before conditionally setting
    int opcode; // Command that caused an error. Not guaranteed to be set.

    /*
     * The actual connection parameters used. Set by the async event handler
     * in response to a connection establishment event.
     */
    BLE_Conn_Params usedConnParams;
    uint8_t bleAddr[6];

    uint32_t authKey;
    int securityState;

    /* Maximum transmission unit */
    uint16_t mtu;

    /* For security prompts when using a display method besides serial.
       Manually set these in setup(). */
    displayStringFxn_t displayStringFxn;
    displayUIntFxn_t displayUIntFxn;

    BLE(byte portType=BLE_PORT_UART);

    /* BLE state */
    int begin(void);
    void setLogLevel(uint8_t newLogLevel);
    int handleEvents(void); // BLEEventHandling.cpp
    int terminateConn(void);
    void end(void);
    bool isConnected(void);
    bool isAdvertising(void);

    /* Advertising */
    int startAdvert(BLE_Advert_Settings *advertSettings=NULL);
    int stopAdvert(void);
    int setAdvertData(uint8_t advertType, uint8_t len, uint8_t *advertData);
    uint8_t* getAdvertData(uint8_t advertType);
    int setAdvertName(const char *advertName);
    int setAdvertName(String *advertName);

    /* Advanced parameters */
    int setGattParam(uint8_t serviceID, uint8_t charID,
                     uint16_t len, uint8_t *pData);
    int getGattParam(uint8_t serviceID, uint8_t charID,
                     uint16_t *len, uint8_t *pData);
    int setGapParam(uint16_t paramId, uint16_t value);
    int getGapParam(uint16_t paramId, uint16_t *Value);
    uint8_t *hciCommand(uint16_t opcode, uint16_t len, uint8_t *pData);

    /* Connection parameters */
    int setConnParams(BLE_Conn_Params_Update_Req *connParams);
    int setMinConnInt(uint16_t intervalMin); // Number of 1.25ms time slots
    int setMaxConnInt(uint16_t intervalMax); // Number of 1.25ms time slots
    int setRespLatency(uint16_t slaveLatency); // Measured in number of connection intervals the slave can miss.
    int setBleTimeout(uint16_t supervisionTimeout);

    /* Services and characteristics */
    int addService(BLE_Service *bleService);
    int writeValue(BLE_Char *bleChar, bool value); //_bool
    int writeValue(BLE_Char *bleChar, char value); //_char
    int writeValue(BLE_Char *bleChar, unsigned char value); //_uchar
    int writeValue(BLE_Char *bleChar, int value); //_int
    int writeValue(BLE_Char *bleChar, unsigned int value); //_uint
    int writeValue(BLE_Char *bleChar, long value); //_long
    int writeValue(BLE_Char *bleChar, unsigned long value); //_ulong
    int writeValue(BLE_Char *bleChar, float value); //_float
    int writeValue(BLE_Char *bleChar, double value); //_double
    int writeValue(BLE_Char *bleChar, const uint8_t *buf, int len); //_uint8_t
    int writeValue(BLE_Char *bleChar, const char *str); // Char array //_string
    int writeValue(BLE_Char *bleChar, String *str); // Object, calls fxn for char array //_String
    bool readValue_bool(BLE_Char *bleChar);
    char readValue_char(BLE_Char *bleChar);
    unsigned char readValue_uchar(BLE_Char *bleChar);
    int readValue_int(BLE_Char *bleChar);
    unsigned int readValue_uint(BLE_Char *bleChar);
    long readValue_long(BLE_Char *bleChar);
    unsigned long readValue_ulong(BLE_Char *bleChar);
    float readValue_float(BLE_Char *bleChar);
    double readValue_double(BLE_Char *bleChar);
    uint8_t* readValue_uint8_t(BLE_Char *bleChar, int *len);
    char* readValue_charArr(BLE_Char *bleChar);
    String readValue_String(BLE_Char *bleChar);
    void setValueFormat(BLE_Char *bleChar, uint8_t valueFormat,
                        int8_t valueExponent=0);

    /* Security */
    int setPairingMode(uint8_t pairingMode);
    int setIoCapabilities(uint8_t param);
    int useBonding(bool param);
    int eraseAllBonds(void);
    int replaceLruBond(bool param);
    int sendSecurityRequest(void);
    int useWhiteListPolicy(bool useWhiteList);

    /* Diagnostics */
    unsigned int getRand(void);
    void getRevision(BLE_Get_Revision_Rsp *getRevisionRsp);
    void getStatus(BLE_Get_Status_Rsp *getStatusRsp);
    int testCommand(BLE_Test_Command_Rsp *testRsp);

    /* Serial over BLE */
    int serial(void);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void);
    virtual size_t write(uint8_t c);
    virtual size_t write(const uint8_t *buffer, size_t size);
};

extern BLE ble;

#endif
