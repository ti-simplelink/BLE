
#include <BLE.h>

void setup() {
  Serial.begin(115200);
  ble.setLogLevel(BLE_LOG_ERRORS);
  ble.begin();
  ble.serial();
  ble.setAdvertName("Energia Serial");
  ble.startAdvert();
}

/* +1 for null-terminator */
int numBytes = 0;
char serialData[BLE_SERIAL_BUFFER_SIZE + 1];

void loop() {
  ble.handleEvents();
  /* Forward Energia serial monitor to BLE serial. */
  if (Serial.available())
  {
    while ((numBytes = Serial.available()))
    {
      Serial.readBytes(serialData, numBytes);
      serialData[numBytes] = '\0';
      Serial.print("Sending via serial:");
      Serial.println(serialData);
      ble.print(serialData);
    }
  }
  if (ble.available())
  {
    /* All the Serial functions are available with BLE serial. */
    Serial.println(ble.readString());
  }
}
