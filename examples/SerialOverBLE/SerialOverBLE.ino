
#include <BLE.h>
#include <String.h>

/* Pin number variables for LEDs on MSP432P401R LaunchPad */
uint8_t led1Pin = 78;
uint8_t led2RedPin = 75;
uint8_t led2GreenPin = 76;
uint8_t led2BluePin = 77;

/* +1 for null-terminator */
int numBytes = 0;
char serialData[BLE_SERIAL_BUFFER_SIZE + 1];


void setup() {
  Serial.begin(115200);
  ble.setLogLevel(BLE_LOG_ERRORS);
  // Initalize I/O
  pinMode(led1Pin, OUTPUT);
  pinMode(led2RedPin, OUTPUT);
  pinMode(led2GreenPin, OUTPUT);
  pinMode(led2BluePin, OUTPUT);
  ble.begin();
  ble.serial();
  ble.setAdvertName("Energia Serial");
  ble.startAdvert();
}

void parseSerialString(String dataString)
{
  // Check if we found Adafruit app msg start char
  if('!' == dataString.charAt(0))
  {
    switch (dataString.charAt(1))
    {
      case 'C':
      {
        // RGB color packet is of the form "[!],[C], [R byte], [G Byte], [B byte]"
        Serial.println("Received Adafruit RGB LED Color Packet");
        char redVal = dataString.charAt(2);
        char greenVal = dataString.charAt(3);
        char blueValue = dataString.charAt(4);

        // Use PWM to write the color to the RGB LED
        analogWrite(led2RedPin, redVal);
        analogWrite(led2GreenPin, greenVal);
        analogWrite(led2BluePin, blueValue);
        break;
      }
      case 'B':
      {
        // Buttons packet is of the form: "[!], [B], [Button #] [1 (pressed)/0 (released)]
        String state = (('1' == dataString.charAt(3)) ? " Pressed" : "  Released");
        String result = "Received Adafruit Button Packet: Button ";
        result += dataString.charAt(2) + state;
        Serial.println(result);
      }
      default:
        break;
    }
  }
}

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
    String leData = ble.readString();
    parseSerialString(leData);
    Serial.print("Received via BLE:");
    Serial.println(leData);
  }
}