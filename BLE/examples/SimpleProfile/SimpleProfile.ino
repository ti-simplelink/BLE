
#include <BLE.h>

/* State variables for LED chars */
uint8_t led1Value = 0;
uint8_t led2RedValue = 0;
uint8_t led2GreenValue = 0;
uint8_t led2BlueValue = 0;

/* State Variables for Button Chars */
// Buttons are active low so initial state is 1
uint8_t button1State = 1;
uint8_t button2State = 1;

/* Pin number variables for LEDs on MSP432P401R LaunchPad */
uint8_t led1Pin = 78;
uint8_t led2RedPin = 75;
uint8_t led2GreenPin = 76;
uint8_t led2BluePin = 77;

/* Pin number variables for Buttons on MSP432P401R LaunchPad */
uint8_t button1Pin = 73;
uint8_t button2Pin = 74;

/* Declare Simple LED Service Characteristics here */

// Char 1 requires authentication, this means you will need to
// pair with your phone before being able to write/read from it
BLE_Char led1Char =
{
  {0xF1, 0xFF},                               /* 16 bit BLE UUID = 0xFFF1 */
  BLE_AUTHEN | BLE_WRITABLE| BLE_READABLE,    /* BLE Properties           */
  "LED 1"                                     /* User Description         */
};

BLE_Char led2RedChar =
{
  {0xF2, 0xFF},
  BLE_WRITABLE| BLE_READABLE,
  "LED 2: Red"
};

BLE_Char led2GreenChar =
{
  {0xF3, 0xFF},
  BLE_WRITABLE| BLE_READABLE,
  "LED 2: Green"
};

BLE_Char led2BlueChar =
{
  {0xF4, 0xFF},
  BLE_WRITABLE| BLE_READABLE,
  "LED 2: Blue"
};

/* BLE LED Service is made up of LED Chars */
BLE_Char *simpleLEDServiceChars[] = {&led1Char, &led2RedChar, &led2GreenChar, &led2BlueChar};

/* LED Service Declaration */
BLE_Service simpleLEDService =
{
  {0xF0, 0xFF},
  4, simpleLEDServiceChars
};

BLE_Char button1Char =
{
  {0x01, 0xFF},
  BLE_NOTIFIABLE,
  "Button 1 State"
};

BLE_Char button2Char =
{
  {0x02, 0xFF},
  BLE_NOTIFIABLE,
  "Button 2 State"
};

/* BLE LED Service is made up of LED Chars */
BLE_Char *simpleButtonServiceChars[] = {&button1Char, &button2Char};

/* LED Service Declaration */
BLE_Service simpleButtonService =
{
  {0x00, 0xFF},
  2, simpleButtonServiceChars
};

void setup() {
  Serial.begin(115200);
  ble.setLogLevel(BLE_LOG_ERRORS);
  ble.begin();

  // Initalize I/O
  pinMode(led1Pin, OUTPUT);
  pinMode(led2RedPin, OUTPUT);
  pinMode(led2GreenPin, OUTPUT);
  pinMode(led2BluePin, OUTPUT);

  // Note that the switches on the MSP432P401R LP need pullups
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);

  /* Add and intialize LED service */
  ble.addService(&simpleLEDService);
  ble.writeValue(&led1Char, led1Value);
  ble.writeValue(&led2RedChar, led2RedValue);
  ble.writeValue(&led2GreenChar, led2GreenValue);
  ble.writeValue(&led2BlueChar, led2BlueValue);

  /* Add and initialize Button Service */
  ble.addService(&simpleButtonService);
  ble.writeValue(&button1Char, button1State);
  ble.writeValue(&button2Char, button2State);

  /* Start Advertising */
  ble.setAdvertName("BLE I/O Demo");
  ble.startAdvert();

  /* Setup security params */
  ble.setPairingMode(BLE_SECURITY_WAIT_FOR_REQUEST);
  ble.setIoCapabilities(BLE_DISPLAY_YES_NO);
  ble.useBonding(true);

  /* Print a message to the console */
  Serial.println(" BLE Energia Buttons/LED Demo");
}

// the loop routine runs over and over again forever as a task.
void loop() {
  ble.handleEvents();

  if(ble.isConnected())
  {
    // Read the state of both buttons and send a notification
    // if either is pressed, note that your app will
    // need to register for notifications by writing 01:00
    // to the button char's CCCD
    uint8_t newButton1State = digitalRead(button1Pin);
    uint8_t newButton2State = digitalRead(button2Pin);

    if(button1State != newButton1State)
    {
      button1State = newButton1State;
      Serial.print("Button 1 Val Changed=");Serial.println(button1State);
      ble.writeValue(&button1Char, button1State);
    }

    if(button2State != newButton2State)
    {
      button2State = newButton2State;
      Serial.print("Button 2 Val Changed=");Serial.println(button2State);
      ble.writeValue(&button2Char, button2State);
    }

    // Poll the values of the characteristics in SimpleProfile
    // Print char value if it changed
    if(led1Value != ble.readValue_char(&led1Char))
    {
      led1Value = ble.readValue_char(&led1Char);
      Serial.print("LED 1 Val Changed=");Serial.println(led1Value);
    }

    if(led2RedValue != ble.readValue_char(&led2RedChar))
    {
      led2RedValue = ble.readValue_char(&led2RedChar);
      Serial.print("LED 2 Red Val Changed=");Serial.println(led2RedValue);
    }

    if(led2GreenValue != ble.readValue_char(&led2GreenChar))
    {
      led2GreenValue = ble.readValue_char(&led2GreenChar);
      Serial.print("LED 2 Green Val Changed=");Serial.println(led2GreenValue);
    }

    if(led2BlueValue != ble.readValue_char(&led2BlueChar))
    {
      led2BlueValue = ble.readValue_char(&led2BlueChar);
      Serial.print("LED 2 Blue Val Changed=");Serial.println(led2BlueValue);
    }

    // Set LEDs based on new characteristic values
    // LED1 can only be on or off, mask out all other bits
    digitalWrite(led1Pin, (led1Value & 0x01));
    // Tri color LED can be PWM'd to create different colors
    // Values between 0-255 are acceptable
    analogWrite(led2RedPin, led2RedValue);
    analogWrite(led2GreenPin, led2GreenValue);
    analogWrite(led2BluePin, led2BlueValue);
  }
}