 #include <Arduino.h>

/*******Controller**************/
#include <SD.h>
#include <Servo.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_PWMServoDriver.h>
/*******LCD**************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

/******Vars**************/

//CONTROLLER
#define ControllerCodeVersion "9.0.22"
#define screenType "3.5 inch MCUFriend TFT Touch Screen"
#define remoteSensorCodeVersion "7.2.6"

//#define DEBUG true

#define tankLevelOK 24 //Green LED
#define tankFull 28 // Green LED
#define tankFilling 26 //Blue LED
#define tankEmpty 22  //Red LED
#define pumpRunning 30  //Blue LED

#define relay1 29
#define relay2 27

#define radioSETPin 23  //Mega pin D23

#define ledTest 41 //Mega pin D41
#define vacConst 37  //Mega pin D37
#define vacSwitched 39  //Mega pin D39
#define vacActiveLED 36 //Mega pin D36
#define modeAuto 45  //Mega pin D45
#define modeManual 44  //Mega pin D44
#define modeAutoLED 42  //Mega pin D42
#define modeManualLED 38  //Mega pin D38
#define modeBypassLED 40  //Mega pin D40 //modeByass active when modeAuto & modeManual both set HIGH
#define servosRunningLED 32  //Mega pin D32
#define radioLinkOnLED 31  //Mega pin D31
#define alarmLED 34  //Mega pin D34

#define analogInputA13 67
#define analogInputA12 66
#define analogInputA11 65
#define analogInputA10 64

#define piezoBuzzerPin 25

#define dataLoggerCS 53
char dataFilename[] = "DATALOG.TXT";
boolean sdOk = false;
File dataFile;
RTC_DS1307 rtc;

// Declare variables for Servo's used on the pwm Controller.

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40); //default address 0x40
/* you can also call it with a different address you want to use by:
  //Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41); */
#define SERVOMIN  137 //old 135 //old 130 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  598 //old 600 //old 605 // this is the 'maximum' pulse length count (out of 4096)

String rxTempString = "";
long baud = 9600;

float rxActualLevel = 0.0;  // 0%=230, 10%=207, 100% = 20 //in cm
float rxActualLevelOld = 0.0;
float rxTemperatureLevel = 0.0;
float rxPercentageLevel = 0.0;
float rxAvailableVolume = 0.0;
float rxTotalCapacity = 0.0;
float rxBatteryVoltageLevel = 0.00;
float rxSensorControl5v = 0.0;
float rxTankHeight = 0.0;
float rxRefillSetPoint = 0.0;
float rxFullSetPoint = 0.0;
float rxController12v = 0.0;
float rxController7v = 0.0;
float rxController5v = 0.0;
float rxController3v3 = 0.0;
float rxControllerVacConst = 0.0;
float rxControllerVacSw = 0.0;
uint16_t manualModeCount = 0;
uint16_t modeInt = 0;
uint16_t modeMessageCount = 0;
uint16_t servoStepCount = 1;
uint16_t buttonLEDTest = 0;
uint16_t msgCount = 0;
uint16_t msgCountOld = 0;
uint16_t rxFileChar = 0;
uint16_t readErrorTolerance = 5;
uint16_t serialSetupCount = 0;
char tftDegreesSymbol = char(247);

//const float R1 = 100000.0;  //R1 100K
//const float R2 = 10000.0;   //R2 10K

const float v12vUL = 13.2; //10% above
const float v12vLL = 10.8; //10% below
const float v7vUL = 7.70;
const float v7vLL = 6.30;
const float v5vUL = 5.50;
const float v5vLL = 4.50;
const float v3vUL = 3.66;
const float v3vLL = 2.97;

const float vPinMax = 5.0; //with usb not connected

const float tempMin = 0.0;
const float tempMax = 85.0;
const float fullOffset = 0.5;
boolean rtcRunning = false;
boolean initCompleted = false;
boolean autoLEDTest = true;
boolean tankConnected = true;
boolean servoControllerFound = false;
boolean miniDataLoggerModuleFound = false;
boolean serialWifiFound = false;
boolean servo1Failed = false;
boolean servo2Failed = false;
boolean logFile = false;
const byte maxBuffer = 64;
static char buffer[maxBuffer];


//LCD

MCUFRIEND_kbv tft;
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//*********************************************
//SELECT DESIRED TFT TOUCH PIN WIRING FOR INSTALLED SCREEN
//const int XP = 7, XM = A1, YP = A2, YM = 6 ; //480x320 ID=0x9488, ID=0x6814
const int XP = 6, XM = A2, YP = A1, YM = 7; //320x480 ID=0x9486, ID=0x1581
//*********************************************

const uint16_t Orientation = 1; //Landscape 90 deg

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define BUTTON_X 240
#define BUTTON_Y 75
#define BUTTON_W 300
#define BUTTON_H 75

#define returnBtn_X 434
#define returnBtn_Y 295
#define returnBtn_W 95
#define returnBtn_H 50
#define returnBtn_TxtSize 2

#define BUTTON_SPACING_X 10
#define BUTTON_SPACING_Y 10

#define BUTTON_TEXTSIZE 2
#define ReturnBtn_TxtSize 2

#define minPressure 10
#define maxPressure 1000
#define labelLength 9
#define mainMenuBtns 4

uint16_t btnOffsetY = 0;
uint16_t btnOffsetX = 0;

TouchScreen ts(XP, YP, XM, YM, 300);

TSPoint p;

#define RGB(r, g, b) (((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3))

#define Black RGB(0,0,0)
#define White RGB(255,255,255)
#define Red RGB(255,0,0)
#define Blue RGB(0,0,255)
#define Navy RGB(0,0,128)
#define LightSalmon RGB(255,160,122)
#define Yellow RGB(255,255,0)
#define Orange RGB(255,165,0)
#define Green RGB(0,128,0)
#define ForestGreen RGB(34,139,34)
#define Lime RGB(0,255,0)
#define LimeGreen RGB(50,205,50)
#define CadetBlue RGB(95,158,160)
#define DodgerBlue RGB(30,144,255)
#define LightSkyBlue RGB(135,206,250)
#define Navy RGB(0,0,128)
#define LightSteelBlue RGB(176,196,222)

#define buttonColor Lime
#define buttonTextColor Navy

uint16_t currentPage = 0; //var for current page on screen

//variables for Page Description & DTG Co-ords
uint16_t pageTitleX = 0;
uint16_t pageTitleY = 0;



const uint16_t numTanks = 2;
const float tankRadius = 175.0;  //in cm
const float tankArea = 962.0; //Area=r*r*pi in cm
float volume = 0.0; // volume = Area * height
float volumeLevel = 0.0;  //map volume of tank

//vars for interfacing tank level with screen
uint16_t ptrOffset = 0; //for positioning of % level value in tank graphic
uint16_t ptrOffsetEmpty = 0; //for positioning of % level value in tank graphic when empty
uint16_t ptrOffsetFull = 0; //for positioning of > 100% level value in tank graphic
uint16_t ptrOffsetPercentValueLabel = 0; //for positioning of > 100% level % value in tank graphic

// arrays for main menu buttons
Adafruit_GFX_Button buttons[mainMenuBtns];
char mainMenuBtnLabels[mainMenuBtns][labelLength] = {"System", "Control", "Tank", "\0"};
uint16_t mainMenuBtnColors[mainMenuBtns] = {buttonColor, buttonColor, buttonColor, buttonColor};

//button used as 'return' button on all pages except main menu
Adafruit_GFX_Button returnBtn;

//vars for drawing LED's on screen
const uint16_t ledRadius = 7;
const uint16_t ledSpacing = 20;
const uint16_t ledLitRadius = 3; //var for size of color dots

//arrays building headings and LED arrays on 'control' page
//control LED array
const uint16_t ledControlArraySize = 9;
const char* ledControlLabels[ledControlArraySize] = {"Radio Rx", "VAC", "Servo's", "Pump On", "Empty", "Level OK", "Filling", "Full", "\0"};
//relay LED array
const uint16_t ledRelayArraySize = 3;
const char* ledRelayLabels[ledRelayArraySize] = {"Relay1", "Relay2", "\0"};
//relay control headings array
const uint16_t ledRelayControlArraySize = 5;
const char* ledRelayControlLabels[ledRelayControlArraySize] = {"NC", "NO", "NC", "NO", "\0"};
//control servo array
const uint16_t ledServoArraySize = 3;
const char* ledServoLabels[ledServoArraySize] = {"Servo1", "Servo2", "\0"};
//mode control array
const uint16_t ledModeArraySize = 4;
const char* ledModeLabels[ledModeArraySize] = {"Auto", "Manual", "Bypass", "\0"};
//VAC Voltage array
const uint16_t ledACVoltageArraySize = 3;
const char* ledACVoltageLabels[ledACVoltageArraySize] = {"Constant", "Switched", "\0"};
//DC voltage array
const unsigned short ledDCVoltageArraySize = 7;
const char* ledDCVoltageLabels[ledDCVoltageArraySize] = {"+12V", "+7V", "+5V", "+3.3V", "Sensor+12v", "Sensor+5v", "\0"};
//I2C Modules array
const uint16_t ledI2CModuleArraySize = 4;
const char* ledI2CModuleLabels[ledI2CModuleArraySize] = {"Servo Controller", "DataLogger", "Wifi Module", "\0"};
//controller Indicators To WebPage
const uint16_t controllerIndicatorsArraySize = 16;
const uint16_t controllerVoltagesArraySize = 7;
char* contIndValue[controllerIndicatorsArraySize];
String contVolValue[controllerVoltagesArraySize];
// Tank Levels array for page on 'tank page
const uint16_t tankLevelArraySize = 6;
const char* tankLevelLabels[tankLevelArraySize] = {">= 0 & < 10%", ">= 10 & < 50%", ">= 50 & < 75%", ">= 75 & < 95%", ">= 95%", "\0"};
/*****************************************************************************************************************************************/
//SETUP
/*****************************************************************************************************************************************/
void setup() {

  Serial.begin(baud);
  delay(50);

  Serial.println(F("========================================"));
  Serial.println(F("PUMP CONTROLLER SERIAL OUTPUT"));
  Serial.println(F("========================================"));

  /****tft*****/

  pinMode(A0, OUTPUT);       //.kbv mcufriend have RD on A0
  digitalWrite(A0, HIGH);

  // Setup the tft
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(Orientation);
  tft.fillScreen(Black);

  serialSetup();

  showSplash();

  /****Controller*****/

  scanI2C(); //scan for I2C devices (SD/RTC & Servo)


  if (rtcSetup() == true ) {

    if (SD.begin(dataLoggerCS)) {
      Serial.print("Initialising SD card........");// setup for the SD card
      Serial.println();
      sdOk = true;
    }
    else if (!SD.begin(dataLoggerCS)) {
      Serial.println();
      Serial.println(F("Initialisation of SD card....failed!"));
      Serial.println();
      sdOk = false;
      return;
    }
    if (sdOk == true) {
      Serial.println(F("SD Card Initialised.......OK."));
      Serial.println();
      dataFile = SD.open(dataFilename, FILE_WRITE); //open/create file
    }
    if (SD.exists(dataFilename)) {
      logFile = true;
      Serial.print("File: ") && Serial.print(dataFilename) && Serial.println(" has been found and opened.");
      Serial.println();

      if (dataFile.size() > 0) {
        Serial.print("Data Headings have already been written to the file: ") && Serial.println(dataFilename);
        Serial.println();
        dataFile.close();
        Serial.print("File: ") && Serial.print(dataFilename)&& Serial.println(" has been closed");
        Serial.println();
      }
      else {
        Serial.print("File: ") && Serial.print(dataFilename) && Serial.println(" is Empty!");
        Serial.println();
        if (dataFile) {  // if the file opened ok, write to it:
          Serial.print("File: ") && Serial.print(dataFilename) && Serial.println(" is ready for writing to!");
          Serial.println();
          Serial.print("Writing data 'Headings' to file: ") && Serial.print(dataFilename) && Serial.println(" .........");
          Serial.println();

          dataFile.println("Date,Time, Level, % Full, Temperature ºC, +12v, +7v, +5v, +3v3, Sensor Battery, Sensor Control"); // pruint16_t the headings for our data

          Serial.println(F("Date,Time, Level, % Full, Temperature ºC, +12v, +7v, +5v, +3v3, Sensor Battery, Sensor Control"));
          Serial.println();
          Serial.print("Writing data to: ") && Serial.print(dataFilename) && Serial.println(" has completed.");
          dataFile.close();
        }
        else {
          logFile = false;
          Serial.print("Error! Cannot write to file: ") && Serial.println(dataFilename);
          Serial.println();
          dataFile.close();
        }
      }
    }
    else {
      Serial.print("File: ") && Serial.print(dataFilename) && Serial.println(" cannot be found.");
      Serial.println();
    }
  }


  // define pwm module inits for servos
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  //define piezo buzzer pin
  pinMode(piezoBuzzerPin, OUTPUT);

  //define data logging pin modes
  pinMode(dataLoggerCS, OUTPUT);
  pinMode(10, OUTPUT);

  //define input pin modes for 28vac voltage sensing
  pinMode(vacConst, INPUT_PULLUP);
  pinMode(vacSwitched, INPUT_PULLUP);

  // define pin modes for tx, rx for Serial1 (HC12 radio link)

  pinMode(radioSETPin, OUTPUT);
  digitalWrite(radioSETPin, HIGH); //LOW for command mode


  //define analogue inputs
  pinMode(analogInputA13, INPUT); // reads 12v Supply
  pinMode(analogInputA12, INPUT); //reads 7v Supply
  pinMode(analogInputA11, INPUT); //reads 5v Supply
  pinMode(analogInputA10, INPUT); //reads 3v (3v3) Supply


  //define input pin modes for tank status
  pinMode(tankLevelOK, OUTPUT);
  pinMode(tankEmpty, OUTPUT);
  pinMode(tankFull, OUTPUT);
  pinMode(tankFilling, OUTPUT);
  pinMode(pumpRunning, OUTPUT);

  //define input pin modes for relay control
  pinMode(relay1, OUTPUT);    //28vac Constant
  pinMode(relay2, OUTPUT);    //28vac Switched

  //define input pin mode for control modes
  pinMode(modeAuto, INPUT);
  pinMode(modeManual, INPUT);
  pinMode(modeAutoLED, OUTPUT);
  pinMode(modeManualLED, OUTPUT);
  pinMode(modeBypassLED, OUTPUT);
  pinMode(vacActiveLED, OUTPUT);
  pinMode(servosRunningLED, OUTPUT);
  pinMode(radioLinkOnLED, OUTPUT);
  pinMode(alarmLED, OUTPUT);
  pinMode(ledTest, INPUT);

  ledTestCheck();

  do {
    //Serial.print(" rxActualLevel: ") && Serial.println(rxActualLevel);
  } while (rxSensorData() == false);
}
/*****************************************************************************************************************************************/
//LOOP
/*****************************************************************************************************************************************/
void loop() {
  if (boolean(initCompleted) == false) {
    servoChangeFromTank();
    showMainMenu();
    initCompleted = true;
  }
  buttonPressed();
  getMode();
  sendControllerIndicators(contIndValue, controllerIndicatorsArraySize);
  sendControllerVoltages(contVolValue, controllerVoltagesArraySize);
  getAlarmStatus();

  rxSensorData();

  drawLEDOperation();
  if (rxActualLevelOld != rxActualLevel) {
    drawTankLevel(rxActualLevel);
    getTankDetails();
    rxActualLevelOld = rxActualLevel;
  }
  showSupplyVoltages();
  vacVoltageCheck();
  vacRelayControl();
  modeControl();
  servoControl();
  monStatus();

}
//----------------- FUNCTIONS---------------------------------------------------//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
void buttonPressResults(TSPoint p, uint16_t b, uint16_t x, uint16_t y, uint16_t z) {  //Serial readout of x & y co-ords when screen/button is pressed, also finger pressure value

  if (buttons[b].contains (p.y + btnOffsetY, p.x - btnOffsetX )) {// offsets added to match pressure points with buttons
    Serial.print("Button: ") && Serial.print(b) && Serial.print(" ") && Serial.print(mainMenuBtnLabels[b])&& Serial.println(" Pressed = True");
    Serial.print("p.x="); Serial.print(p.x);
    Serial.print("\t p.y="); Serial.print(p.y);
    Serial.print("\t p.z="); Serial.println(p.z);
    Serial.println();
  }
}
//-------------------------------------------------------------------------------
void selectMenuPage(TSPoint p, uint16_t b, uint16_t x, uint16_t y, uint16_t z) {

  switch (b) {
    case 0:  {
        buttonPressResults(p, b, p.x, p.y, p.z);
        buttons[b].justReleased();
        buttons[b].drawButton();
        showSystem();
        break;
      }
    case 1: {
        buttonPressResults(p, b, p.x, p.y, p.z);
        buttons[b].justReleased();
        buttons[b].drawButton();
        showControl();
        break;
      }
    case 2: {
        buttonPressResults(p, b, p.x, p.y, p.z);
        buttons[b].justReleased();
        buttons[b].drawButton();
        showTank();
        break;
      }
  }
}
//-------------------------------------------------------------------------------
void buttonPressed() { //getScreenXY()

  uint16_t ID = tft.readID();
  p = ts.getPoint();
  uint16_t b = 0;

  pinMode(YP, OUTPUT); //A2     //restore shared pins
  pinMode(XM, OUTPUT);  //A1
  digitalWrite(YP, HIGH);  //pin 7
  digitalWrite(XM, HIGH); //pin 6

  //Serial.print("tft Width: ") && Serial.print(tft.width()) && Serial.print("  tft Height: ") && Serial.println(tft.height());
  // Serial.print("TFT ID = 0x") && Serial.println(ID, HEX);

  if (p.z > minPressure && p.z < maxPressure) {  //finger pressure
    Serial.println("Screen touch within parmeters...");
    switch (tft.readID()) { //read controller type
      case 0x1581: {
          btnOffsetY = 0;
          btnOffsetX = 0;
          p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
          p.y = map(p.y,  TS_MINY, TS_MAXY, tft.width(), 0);
          // x = vertical, y = horizontal in landscape mode
          break;
        }
      case 0x9486: {
          btnOffsetY = 0;
          btnOffsetX = 0;
          p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
          p.y = map(p.y,  TS_MAXY, TS_MINY, 0, tft.width());
          // x = vertical, y = horizontal in landscape mode
          break;
        }
      case 0x9488: {
          btnOffsetY = 115;
          btnOffsetX = 50;
          p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
          p.y = map(p.y,  TS_MAXY, TS_MINY, 0, tft.height());
          // x = vertical, y = horizontal in landscape mode
          break;
        }
      case 0x6814: {
          btnOffsetY = 115;
          btnOffsetX = 0;
          p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
          p.y = map(p.y,  TS_MAXY, TS_MINY, 0, tft.width());
          // x = vertical, y = horizontal in landscape mode
          break;
        }
    }
    buttonPressResults(p, b, p.x, p.y, p.z);  //Show Serial button press results

    //find Main Menu pressed button and make it true

    if (currentPage == 0) {  //Main Menu

      for (b = 0; b < mainMenuBtns; b++) {

        if (buttons[b].contains (p.y + btnOffsetY, p.x - btnOffsetX )) {// offsets added to match pressure points with buttons
          buttons[b].justPressed(); // tell the button it is pressed
          buttons[b].drawButton(true);
          // Serial.print("Button: ") && Serial.print(b) && Serial.print(" ") && Serial.print(mainMenuBtnLabels[b])&& Serial.println(" Pressed = True");

          selectMenuPage(p, b, p.x, p.y, p.z);
        }
      }
    }
    else if (currentPage != 0) { //Pages other than main menu
      switch (tft.readID()) { //read controller type
        case 0x1581: {
            if (p.y > 240 && p.y < 315 && p.x > 425 && p.x < 520) { //returnBtn
              returnBtnAction();
              break;
            }
          }
        case 0x9486: {
            if (p.y > 240 && p.y < 315 && p.x > 425 && p.x < 520) { //returnBtn
              returnBtnAction();
              break;
            }
          }
        case 0x9488: {
            if (p.y > 240 && p.y < 315 && p.x > 425 && p.x < 520) { //returnBtn
              returnBtnAction();
              break;
            }
          }
        case 0x6814: {
            if (p.x > 280 && p.x < 330 && p.y > 350 && p.y < 450) { //returnBtn
              returnBtnAction();
              break;
            }
          }
      }
    }
  }
  else if ((p.z < minPressure && minPressure >= 0) && p.z > maxPressure) {  //finger pressure
    Serial.println("Finger Pressure Incorrect.");
  }
}
//--------------------------------------------------------------------------
void returnBtnAction() {  //function to show return button pressed (inverted) & released (restored) returning to main menu screen
  returnBtn.justPressed();
  returnBtn.drawButton(true);
  delay (100);
  returnBtn.justReleased();
  returnBtn.drawButton();
  Serial.print("Return Button: ") && Serial.println(" Pressed & Released = True");  //Serialized confirmation
  tft.fillScreen(Black);
  showMainMenu();
}
//--------------------------------------------------------------------------
void getAlarmStatus() {
  uint16_t xPos = 295;
  uint16_t yPos = 1;
  uint16_t xPagePos = 10;
  uint16_t xPagePosTab = 225;
  uint16_t yPagePos = 175;
  uint16_t yPagePosTab = 175;
  const uint16_t maxMsgs = 13;
  uint16_t listLines = 1;
  const uint16_t col1Lines = 11;
  uint16_t displayLinesCutOff = 20;

  int msgStatusArray[maxMsgs];

  char * msgArray[] = {"M: Tank is Full. Switch Modes!", "A: Servo Module!", "A: Data Logger!",
                       "A: Serial Wifi!", "A: Sensor Data!", "A: Tank Full!", "A: Tank Empty!",
                       "A: VAC Supply!", "A: Control Voltage!", "A: Sensor Voltage!",
                       "A: Serial Comms!", "A: SD Card/File!",
                       "A: Servos!",
                       "\0"
                      };

  Serial.println("Alarm Status:");

  if (digitalRead(modeManual) == LOW && (rxActualLevel <= (rxFullSetPoint + fullOffset))) {
    while (digitalRead(modeManual) == LOW && (rxActualLevel <= (rxFullSetPoint + fullOffset))) {
      if ( buzzerOperation() == true) {
        Serial.println(msgArray[0]);
        msgStatusArray[0] = 1;
        contIndValue[13] = "b1";
      }
      else {
        msgStatusArray[0] = 0;
        contIndValue[13] = "b0";
      }
    }
    msgCount += 1;
  }
  if (servoControllerFound == false) {
    Serial.println(msgArray[1]);
    msgStatusArray[1] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[1] = 0;
  }
  if (miniDataLoggerModuleFound == false) {
    Serial.println(msgArray[2]);
    msgStatusArray[2] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[2] = 0;
  }
  if (!Serial2) {
    Serial.println(msgArray[3]);
    msgStatusArray[3] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[3] = 0;
  }

  if ((rxTemperatureLevel <= tempMin || rxTemperatureLevel > tempMax) || (rxActualLevel <= 0 || isnan(rxActualLevel)) ||
      (rxPercentageLevel <= 0 || isnan(rxPercentageLevel)) || (rxAvailableVolume <= 0 || isnan(rxAvailableVolume)) ||
      (rxTotalCapacity <= 0 || isnan(rxTotalCapacity)) || (rxTotalCapacity <= 0 || isnan(rxTotalCapacity)) ||
      (rxTankHeight <= 0 || isnan(rxTankHeight))) {
    Serial.println(msgArray[4]);
    msgStatusArray[4] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[4] = 0;
  }

  if (tankFull <= 0 || isnan(tankFull)) {
    Serial.println(msgArray[5]);
    msgStatusArray[5] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[5] = 0;
  }
  if (tankEmpty <= 0 || isnan(tankEmpty)) {
    Serial.println(msgArray[6]);
    msgStatusArray[6] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[6] = 0;
  }
  if (digitalRead(vacConst) == HIGH) {
    Serial.println(msgArray[7]);
    msgStatusArray[7] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[7] = 0;
  }
  if (voltageOutOfLimits() == 1) {
    Serial.println(msgArray[8]);
    msgStatusArray[8] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[8] = 0;
  }
  if (((rxBatteryVoltageLevel < v12vLL) || (rxBatteryVoltageLevel > v12vUL)) ||
      ((rxSensorControl5v < v5vLL) || (rxSensorControl5v > v5vUL))) {
    Serial.println(msgArray[9]);
    msgStatusArray[9] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[9] = 0;
  }
  if (!Serial1 || !Serial2) {
    Serial.println(msgArray[10]);
    msgStatusArray[10] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[10] = 0;
  }
  if (!SD.begin(dataLoggerCS) || logFile == false) {
    Serial.println(msgArray[11]);
    msgStatusArray[11] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[11] = 0;
  }
  if (servo1Failed == true || servo2Failed == true) {
    Serial.println(msgArray[12]);
    msgStatusArray[12] = 1;
    msgCount += 1;
  }
  else {
    msgStatusArray[12] = 0;
  }

  if (msgCount > 0) {
    tft.setTextSize(1);
    tft.setCursor(xPos, yPos);
    tft.setTextColor(Red, Black);
    tft.print("Alarms ") && tft.print("(") && tft.print(msgCount) && tft.print(") ");
    digitalWrite(alarmLED, HIGH);
    if (currentPage == 1) {
      if (msgCountOld != msgCount) {
        tft.fillRect(xPagePos, yPagePos + 5, 360, 125, Black);
        tft.fillRect(190, yPagePos - 10, 275, 100, Black);
        msgCountOld = msgCount;
      }
      tft.setTextColor(Yellow, Black);
      tft.setCursor(xPagePos, yPagePos);
      for (uint16_t idx = 0; idx < maxMsgs; idx++) {
        if (msgStatusArray[idx] == 1) {
          if (listLines > col1Lines) {
            tft.setCursor(xPagePosTab, yPagePosTab += 10);
          }
          else {
            tft.setCursor(xPagePos, yPagePos += 10);
          }
          tft.print(listLines) && tft.print(". ") && tft.print(msgArray[idx]);
          listLines += 1;
        }
        if (listLines >= displayLinesCutOff) {
          tft.setTextColor(LightSalmon);
          tft.setCursor(190, 170);
          tft.print("(List Incomplete. Max of ") && tft.print(displayLinesCutOff) && tft.print(" lines available!)");
          break;
        }

      }
    }
  }
  if (msgCount == 0) {
    digitalWrite(alarmLED, LOW);
    msgCountOld = msgCount;
    tft.setTextSize(1);
    tft.setCursor(xPos, yPos);
    tft.setTextColor(Lime, Black);
    tft.println("OK          ");
    if (currentPage == 1) { //System Page
      tft.setTextColor(Black);
      tft.setCursor(190, 170);
      tft.print("Max of ") && tft.print(displayLinesCutOff) && tft.print(" display lines available!)");
      if (msgCountOld != msgCount) {
        tft.fillRect(xPagePos, yPagePos + 5, 360, 125, Black);
        tft.fillRect(190, yPagePos - 10, 275, 100, Black);
        msgCountOld = msgCount;
      }
      if (msgCountOld == msgCount) {
        tft.fillRect(xPagePos, yPagePos + 5, 360, 125, Black);
        tft.fillRect(190, yPagePos - 10, 275, 100, Black);
      }
    }
  }
  Serial.print("System Alarm Count:") && Serial.println(msgCount);
  msgCount = 0;
}
//--------------------------------------------------------------------------
void showTopHeadings() {
  tft.setTextSize(1);
  tft.setTextColor(DodgerBlue);
  tft.setCursor(10, 1);  //top left hand corner
  tft.println("Water Tank Pump Controller");
  tft.setCursor(210, 1);
  tft.println("System Status:  ");
  tft.setCursor(410, 1);
  tft.println("Mode:  ");
}
//--------------------------------------------------------------------------
void getMode() {

  uint16_t xPos = 440;
  uint16_t yPos = 1;

  if (digitalRead(modeAuto) == LOW) {
    tft.setTextSize(1);
    tft.setCursor(xPos, yPos);
    tft.setTextColor(LimeGreen, Black);
    tft.println("Auto  ");
  }
  else if (digitalRead(modeManual) == LOW) {
    tft.setTextSize(1);
    tft.setCursor(xPos, yPos);
    tft.setTextColor(Yellow, Black);
    tft.println("Manual");
  }
  else  if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    tft.setTextSize(1);
    tft.setCursor(xPos, yPos);
    tft.setTextColor(White, Black);
    tft.println("Bypass");
  }
}
//--------------------------------------------------------------------------
void drawLEDOperation() { // draw LEDS showing different operating conditions

  uint16_t xPosLed = 0;
  uint16_t yPosLed = 0;

  if (currentPage == 2) {  //control page
    xPosLed = 20; //Mode LEDs...................
    if (digitalRead(modeAuto) == LOW) {
      if (buttonLEDTest == HIGH) {
        yPosLed = 251;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
        yPosLed = 271;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
        yPosLed = 2911;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      else if (buttonLEDTest == LOW)  {
        yPosLed = 251;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
        yPosLed = 271;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
        yPosLed = 291;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, White);
      }
    }
    if (digitalRead(modeManual) == LOW) {
      if (buttonLEDTest == HIGH) {
        yPosLed = 251;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
        yPosLed = 2711;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
        yPosLed = 291;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      else if (buttonLEDTest == LOW)  {
        yPosLed = 251;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
        yPosLed = 271;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
        yPosLed = 291;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, White);
      }
    }
    if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
      if (buttonLEDTest == HIGH) {
        yPosLed = 251;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
        yPosLed = 271;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
        yPosLed = 291;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, White);
      }
      else if (buttonLEDTest == LOW)  {
        yPosLed = 251;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
        yPosLed = 271;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
        yPosLed = 291;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, White);
      }
    }
    xPosLed = 20; //Controller LEDs.....................
    if (buttonLEDTest == HIGH)  {
      if (digitalRead(radioLinkOnLED) == HIGH) {
        yPosLed = 48;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      }
      else if (digitalRead(radioLinkOnLED) == LOW) {
        yPosLed = 48;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(vacActiveLED) == HIGH) {
        yPosLed = 68;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
      }
      else if (digitalRead(vacActiveLED) == LOW) {
        yPosLed = 68;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(servosRunningLED) == HIGH) {
        yPosLed = 88;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, DodgerBlue);
      }
      else if (digitalRead(servosRunningLED) == LOW) {
        yPosLed = 88;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(pumpRunning) == HIGH) {
        yPosLed = 108;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, DodgerBlue);
      }
      else if (digitalRead(pumpRunning) == LOW) {
        yPosLed = 108;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(tankEmpty) == HIGH) {
        yPosLed = 128;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Red);
      }
      else if (digitalRead(tankEmpty) == LOW) {
        yPosLed = 128;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(tankLevelOK) == HIGH) {
        yPosLed = 148;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      }
      else if (digitalRead(tankLevelOK) == LOW) {
        yPosLed = 148;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(tankFilling) == HIGH) {
        yPosLed = 168;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, DodgerBlue);
      }
      else if (digitalRead(tankFilling) == LOW) {
        yPosLed = 168;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
      if (digitalRead(tankFull) == HIGH) {
        yPosLed = 188;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      }
      else if (digitalRead(tankFull) == LOW) {
        yPosLed = 188;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      }
    }
    else if (buttonLEDTest == LOW)  {
      if (digitalRead(radioLinkOnLED) == HIGH) {
        yPosLed = 48;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      }
      if (digitalRead(vacActiveLED) == HIGH) {
        yPosLed = 68;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
      }
      if (digitalRead(servosRunningLED) == HIGH) {
        yPosLed = 88;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, DodgerBlue);
      }
      if (digitalRead(pumpRunning) == HIGH) {
        yPosLed = 108;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, DodgerBlue);
      }
      if (digitalRead(tankEmpty) == HIGH) {
        yPosLed = 128;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Red);
      }
      if (digitalRead(tankLevelOK) == HIGH) {
        yPosLed = 148;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      }
      if (digitalRead(tankFilling) == HIGH) {
        yPosLed = 168;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, DodgerBlue);
      }
      if (digitalRead(tankFull) == HIGH) {
        yPosLed = 188;
        tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      }
    }
    xPosLed = 178; //vac Relays Control..............................
    if (digitalRead(relay1) == HIGH) {  //const vac
      yPosLed = 60;//relay1
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      yPosLed = 80;//relay2
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
    }
    else if (digitalRead(relay1) == LOW) {  //switched vac
      yPosLed = 60;//relayK1
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
      yPosLed = 80;//relayK2
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if (digitalRead(relay2) == HIGH) {  //const vac
      yPosLed = 110;//relayK3
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      yPosLed = 130;//relayK4
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
    }
    else if (digitalRead(relay2) == LOW) {  //switched vac
      yPosLed = 110;//relayK3
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
      yPosLed = 130;//relayK4
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    xPosLed = 290; //vac monitored....................................
    if (digitalRead(vacConst) == LOW) {  //vac Const
      yPosLed = 48;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
    }
    else if (digitalRead(vacConst) == HIGH) {  //vac Const
      yPosLed = 48;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if (digitalRead(vacSwitched) == LOW) {  //vac Switched
      yPosLed = 68;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
    }
    else if (digitalRead(vacSwitched) == HIGH) {  //vac Switched
      yPosLed = 68;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    xPosLed = 290; //dc monitored..............................................
    if ((read12v() > v12vLL) && (read12v()  < v12vUL)) {
      yPosLed = 105;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read12v()  < v12vLL) || (read12v()  > v12vUL)) {
      yPosLed = 105;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((read7v() > v7vLL) && (read7v() < v7vUL)) {
      yPosLed = 125;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read7v() < v7vLL) || (read7v() > v7vUL)) {
      yPosLed = 125;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((read5v() > v5vLL) && (read5v() < v5vUL)) {
      yPosLed = 145;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read5v() < v5vLL) || (read5v() > v5vUL)) {
      yPosLed = 145;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((read3v() > v3vLL) && (read3v() < v3vUL)) {
      yPosLed = 165;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read3v() < v3vLL) || (read3v() > v3vUL)) {
      yPosLed = 165;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((rxBatteryVoltageLevel > v12vLL) && (rxBatteryVoltageLevel < v12vUL)) {
      yPosLed = 185;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((rxBatteryVoltageLevel < v12vLL) || (rxBatteryVoltageLevel > v12vUL)) {
      yPosLed = 185;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((rxSensorControl5v > v5vLL) && (rxSensorControl5v < v5vUL)) {
      yPosLed = 205;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((rxSensorControl5v < v5vLL) || (rxSensorControl5v > v5vUL))  {
      yPosLed = 205;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if (tankConnected == false) {
      //servo 0
      xPosLed = 290;
      yPosLed = 273;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      xPosLed = 370;
      yPosLed = 273;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      //servo 1
      xPosLed = 290;
      yPosLed = 293;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
      xPosLed = 370;
      yPosLed = 293;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    if (tankConnected == true) {
      //servo 0
      xPosLed = 290;
      yPosLed = 273;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius,  Black);
      xPosLed = 370;
      yPosLed = 273;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      //servo 1
      xPosLed = 290;
      yPosLed = 293;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
      xPosLed = 370;
      yPosLed = 293;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    //temperature readings

    showTemperature();
  }
  if (currentPage == 1) {
    xPosLed = 42; //vac monitored....................................
    if (digitalRead(vacConst) == LOW) {  //vac Const
      yPosLed = 65;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
    }
    else if (digitalRead(vacConst) == HIGH) {  //vac Const
      yPosLed = 65;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if (digitalRead(vacSwitched) == LOW) {  //vac Switched
      yPosLed = 85;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Yellow);
    }
    else if (digitalRead(vacSwitched) == HIGH)  {  //vac Switched
      yPosLed = 85;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    xPosLed = 165; //dc monitored..............................................
    getDCVoltageActuals();
    if ((read12v() > v12vLL) && (read12v() < v12vUL)) {
      yPosLed = 45;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read12v() < v12vLL) || (read12v() > v12vUL)) {
      yPosLed = 45;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((read7v() > v7vLL) && (read7v() < v7vUL)) {
      yPosLed = 65;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read7v() < v7vLL) || (read7v() > v7vUL)) {
      yPosLed = 65;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((read5v() > v5vLL) && (read5v() < v5vUL)) {
      yPosLed = 85;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read5v() < v5vLL) || (read5v() > v5vUL)) {
      yPosLed = 85;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((read3v() > v3vLL) && (read3v() < v3vUL)) {
      yPosLed = 105;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((read3v() < v3vLL) || (read3v() > v3vUL)) {
      yPosLed = 105;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((rxBatteryVoltageLevel > v12vLL) && (rxBatteryVoltageLevel < v12vUL)) {
      yPosLed = 125;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((rxBatteryVoltageLevel < v12vLL) || (rxBatteryVoltageLevel > v12vUL)) {
      yPosLed = 125;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if ((rxSensorControl5v > v5vLL) && (rxSensorControl5v < v5vUL)) {
      yPosLed = 145;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if ((rxSensorControl5v < v5vLL) || (rxSensorControl5v > v5vUL))  {
      yPosLed = 145;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }

    xPosLed = 325; //I2C Module Status..............................................
    if (servoControllerFound == true) {
      yPosLed = 65;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if (servoControllerFound == false) {
      yPosLed = 65;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if (miniDataLoggerModuleFound == true) {
      yPosLed = 85;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if (miniDataLoggerModuleFound == false) {
      yPosLed = 85;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
    if (serialWifiFound == true) {
      yPosLed = 105;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, LimeGreen);
    }
    else if (serialWifiFound == false) {
      yPosLed = 105;
      tft.fillCircle(xPosLed, yPosLed, ledLitRadius, Black);
    }
  }
}
//---------------------------------------------------------------------------------
void showPageTitle() { //shows page titles bottom left corner of screen

  pageTitleX = 1;
  pageTitleY = 310;
  tft.setTextSize(1);
  tft.setCursor(pageTitleX, pageTitleY);
  tft.setTextColor(White);
  switch (currentPage) {

    case 0: {
        tft.print("Main Menu");
        //serialShowCurrentPage(currentPage);
        break;
      }
    case 1: {
        tft.print("System Status");
        serialShowCurrentPage(currentPage);
        break;
      }
    case 2: {
        tft.print("Control Status");
        serialShowCurrentPage(currentPage);
        break;
      }
    case 3: {
        tft.print("Tank Status");
        serialShowCurrentPage(currentPage);
        break;
      }
    case 10: {
        tft.print("Splash");
        serialShowCurrentPage(currentPage);
        break;
      }
  }
}
//----------------------------------------------------------------------------
void serialShowCurrentPage(uint16_t currentPage) { //shows current page # on serial

  if (currentPage < 10) { //10 = splash page (don't show spash page number on serial)
    Serial.print("CurrentPage: ") && Serial.println(currentPage);
  }
}
//----------------------------------------------------------------------------
void showMainMenu() {  //Setup Main Menu buttons

  currentPage = 0;

  tft.fillScreen(Black);

  // create buttons for main menu

  for (uint16_t row = 0; row < 3; row++) {
    for (uint16_t col = 0; col < 1; col++) {
      buttons[col + row * 1].initButton(&tft, BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X),
                                        BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y),    // x, y, w, h, outline, fill, text, text size
                                        BUTTON_W, BUTTON_H, Black, mainMenuBtnColors[col + row * 1], buttonTextColor,
                                        mainMenuBtnLabels[col + row * 1], BUTTON_TEXTSIZE);
      buttons[col + row * 1].drawButton();

      //shows co-ords for main menu buttons on screen
      // Serial.print("Button Number: ") && Serial.print(col + row * 1) &&  Serial.print(" ")&&  Serial.print(mainMenuBtnLabels[col + row * 1])&&  Serial.print(" ");
      // Serial.print("X: ")&&Serial.pruint16_t(BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X)) && Serial.print("  W: ") && Serial.print(BUTTON_W);
      //  Serial.print("  Y: ")&&Serial.pruint16_t(BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y)) && Serial.print("  H: ") && Serial.println(BUTTON_H);
    }
  }
  buildPageFrame();
  showPageTitles();
}
//----------------------------------------------------------------------------------
void showSplash() {
  currentPage = 10;

  showPageTitle();
  showTFTDetails();
  showAbout();
  tft.setCursor(200, 280);
  tft.setTextColor(Lime);
  tft.println("Initialising........");
}
//----------------------------------------------------------------------------------
void showAbout() {

  tft.setTextSize(1);
  tft.setTextColor(LightSkyBlue);
  tft.setCursor(100, 160);
  tft.print("Controller Code Version: ") && tft.println(ControllerCodeVersion);
  Serial.println(F("---------------------------------------------------"));
  Serial.print("Controller Code Version: ") && Serial.println(ControllerCodeVersion);
  Serial.println(F("---------------------------------------------------"));
  tft.setCursor(100, 180);
  tft.print("Remote Sensor Code Version: ")&& tft.println(remoteSensorCodeVersion);
  Serial.print("Remote Sensor Code Version: ")&& Serial.println(remoteSensorCodeVersion);
  Serial.println(F("---------------------------------------------------"));
  Serial.println();
  tft.setCursor(100, 250);
  tft.println(F("System developed by Tony Birznieks"));
}
//---------------------------------------------------------------------------------
void showPageTitles() { //shows info on all pages

  showTopHeadings();
  showPageTitle();
  showDTG();
}
//---------------------------------------------------------------------------------
void showDTG () { //shows DTG details centre bottom of screen

  DateTime DTG= rtc.now();
  pageTitleX = 190;
  pageTitleY = 310;

  tft.setTextSize(1);
  tft.setTextColor(White, Black);
  tft.setCursor(pageTitleX, pageTitleY);
 
  if (rtc.begin()) { //if RTC is working show DTG details
    if (DTG.day() < 10) { //PRuint16_tA 0 IN FRONT OF THE DAYS IF LESS THAN 10
      tft.print('0');
      tft.print(DTG.day(), DEC);
    }

    else {
      tft.print(DTG.day(), DEC);
    }
    tft.print('/');

    if (DTG.month() < 10) { //PRuint16_tA 0 IN FRONT OF THE MONTHS IF LESS THAN 10
      tft.print('0');
      tft.print(DTG.month(), DEC);
    }
    else {
      tft.print(DTG.month(), DEC);
    }
    tft.print('/');
    tft.print(DTG.year(), DEC);
    tft.print(' ');

    if (DTG.hour() < 10) { //Print a 0 IN FRONT OF THE HOUR IF LESS THAN 10
      tft.print('0');
      tft.print(DTG.hour(), DEC);
    }
    else {
      tft.print(DTG.hour(), DEC);
    }
    tft.print(':');

    if (DTG.minute() < 10) { //Print a 0 IN FRONT OF THE MINUTES IF LESS THAN 10
      tft.print('0');
      tft.print(DTG.minute(), DEC);
    }
    else {
      tft.print(DTG.minute(), DEC);
    }
  }
  else {
    tft.print("DTG Error");
  }
}
//---------------------------------------------------------------------------------
void showTank() {
  currentPage = 3;

  tft.fillScreen(Black);
  showPageTitles();
  buildPageFrame();
  showTankLevelDefinitions();
  drawTankLevel(rxActualLevel);
  getTankDetails();
  returnMainMenu();
}
//---------------------------------------------------------------------------------
void showControl() {
  currentPage = 2;

  tft.fillScreen(Black);
  showPageTitles();
  buildPageFrame();
  drawLEDSCluster();
  returnMainMenu();
}
//---------------------------------------------------------------------------------
void showSystem() {

  currentPage = 1;

  tft.fillScreen(Black);
  showPageTitles();
  buildPageFrame();
  drawLEDSCluster();
  returnMainMenu();
}
//---------------------------------------------------------------------------------
boolean showTemperature() {
  if (currentPage == 2) { //Control Status
    tft.setTextSize(1);
    tft.setTextColor(White);
    tft.setCursor(195, 225);
    tft.print("Water");
    tft.setCursor(175, 235);
    tft.print("Temperature");

    if (rxTemperatureLevel <= tempMin || rxTemperatureLevel > tempMax) { //abnormal operating levels
      tft.setCursor(165, 260);
      tft.setTextColor(Red, Black);
      tft.print(rxTemperatureLevel) && tft.print(tftDegreesSymbol) && tft.print("C");
      // tft.print(char(0)) && tft.print(rxTemperatureLevel) && tft.print(tftDegreesSymbol) && tft.print("C");
      tft.setTextSize(2);
      tft.setCursor(165, 285);
      tft.setTextColor(Red);
      tft.print("ERROR!");
      return false;
    }
    else if (rxTemperatureLevel > tempMin && rxTemperatureLevel <= tempMax ) { //normal operating levels
      tft.setTextSize(2);
      tft.setCursor(165, 260);
      tft.setTextColor(Yellow, Black);
      tft.print(rxTemperatureLevel) && tft.print(tftDegreesSymbol)&& tft.print("C");
      tft.setTextSize(1);
      tft.setCursor(165, 285);
      tft.setTextColor(Black);
      tft.print("ERROR!");
      return true;
    }
  }
}
//---------------------------------------------------------------------------------
void drawLEDSCluster() {

  uint16_t ArraySize = 0;

  if (currentPage == 2) { //Control Status

    //Setup dividers and header names additional to frame for control page
    tft.drawLine(150, 10, 150, 308, ForestGreen);
    tft.drawLine(260, 10, 260, 308, ForestGreen);
    tft.drawLine(1, 220, 410, 220, ForestGreen);//crossmember
    tft.drawLine(150, 145, 260, 145, ForestGreen);//crossmember
    tft.drawLine(410, 10, 410, 308, ForestGreen);
    tft.setTextSize(2);
    tft.setTextColor(ForestGreen);
    tft.setCursor(20, 12);
    tft.print("Controller");
    tft.setCursor(165, 12);
    tft.print("Relays");
    tft.setCursor(295, 12);
    tft.print("Voltages");
    tft.setCursor(292, 225);
    tft.print("Servos");
    tft.setCursor(20, 225);
    tft.print("Mode");

    //Controller LED's

    uint16_t xContTxt = 40;
    uint16_t yContTxt = 45;
    uint16_t xContCircle = 20;
    uint16_t yContCircle = 48;
    //Serial.println("Controller LED co-ords:Page 2");
    ArraySize = labelLEDS(ledControlLabels, ledControlArraySize, ledSpacing, xContTxt, yContTxt);
    drawLEDArrays(ArraySize, ledSpacing, xContCircle, yContCircle);

    //Mode LED's

    uint16_t xModeTxt = 40;
    uint16_t yModeTxt = 248;
    uint16_t xModeCircle = 20;
    uint16_t yModeCircle = 251;
    //Serial.println("Mode LED co-ords:Page 2");
    ArraySize = labelLEDS(ledModeLabels, ledModeArraySize, ledSpacing, xModeTxt, yModeTxt);
    drawLEDArrays(ArraySize, ledSpacing, xModeCircle, yModeCircle);

    // Relay LED's

    uint16_t xRelayControlTxt = 200;
    uint16_t yRelayControlTxt = 57;
    uint16_t xRelayTxt = 175;
    uint16_t yRelayTxt = 42;
    uint16_t xRelayCircle = 178;
    uint16_t yRelayCircle = 40;

    tft.setTextSize(1);
    tft.setTextColor(Orange);
    tft.setCursor(163, 30);
    tft.print("Status");

    //Serial.println("Relay LED co-ords: Page 2");
    ArraySize = labelRelayLEDS(ledRelayLabels, ledRelayArraySize, ledSpacing, xRelayTxt, yRelayTxt);
    ArraySize = labelRelayControls(ledRelayControlLabels, ledRelayControlArraySize, ledSpacing, xRelayControlTxt, yRelayControlTxt);
    drawRelayLEDArrays(ledSpacing, xRelayCircle, yRelayCircle);

    //Servo LED's
    uint16_t xServoTxt = 312.5;
    uint16_t yServoTxt = 270;

    //Vertical Servo
    uint16_t xServoCircle1 = 290;
    uint16_t yServoCircle1 = 273;
    //Horizontal Servo
    uint16_t xServoCircle2 = 370;
    uint16_t yServoCircle2 = 273;

    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
    //tft.setCursor(270, 215);
    tft.setCursor(270, 253);
    tft.print("Vertical");
    // tft.setCursor(345, 215);
    tft.setCursor(345, 253);
    tft.print("Horizontal");

    //Serial.println("Servo Vertical & Horizontal LED co-ords: Page 2");
    ArraySize = labelLEDS(ledServoLabels, ledServoArraySize, ledSpacing, xServoTxt, yServoTxt);
    drawLEDArrays(ArraySize, ledSpacing, xServoCircle1, yServoCircle1); //Vertical
    drawLEDArrays(ArraySize, ledSpacing, xServoCircle2, yServoCircle2);  //Horizontal

    //Voltage LED's

    uint16_t xACVoltageTxt = 312.5;
    uint16_t yACVoltageTxt = 45;
    uint16_t xACVoltageCircle = 290;
    uint16_t yACVoltageCircle = 48;
    uint16_t xDCVoltageTxt = 312.5;
    uint16_t yDCVoltageTxt = 102;
    uint16_t xDCVoltageCircle = 290;
    uint16_t yDCVoltageCircle = 105;

    tft.setTextColor(LightSalmon);
    tft.setCursor(270, 30);
    tft.print("AC Supply");
    tft.setCursor(270, 85);
    tft.print("DC Control");
    //Serial.println("AC VOltage LED co-ords:Page 2");
    ArraySize = labelLEDS(ledACVoltageLabels, ledACVoltageArraySize, ledSpacing, xACVoltageTxt, yACVoltageTxt);
    drawLEDArrays(ArraySize, ledSpacing, xACVoltageCircle, yACVoltageCircle);
    //Serial.println("DC VOltage LED co-ords:Page 2");
    ArraySize = labelLEDS(ledDCVoltageLabels, ledDCVoltageArraySize, ledSpacing, xDCVoltageTxt, yDCVoltageTxt);
    drawLEDArrays(ArraySize, ledSpacing, xDCVoltageCircle, yDCVoltageCircle);

    //Temperature Display

    showTemperature();

    //Tank Display
    drawTankLevel(rxActualLevel);
  }
  else if (currentPage == 1) { // System, Module Status

    //Setup dividers and headings additional to frame for page 1
    tft.drawLine(160, 160, 160, 160, ForestGreen);
    tft.drawLine(300, 10, 300, 160, ForestGreen);
    tft.drawLine(1, 160, 479, 160, ForestGreen);
    tft.setTextSize(2);
    tft.setTextColor(ForestGreen);
    tft.setCursor(20, 15);
    tft.print("Voltages");
    tft.setCursor(20, 165);
    tft.print("System Status");
    tft.setCursor (310, 15);
    tft.print("Modules");

    //Voltage LED's

    uint16_t xACVoltageTxt = 57;
    uint16_t yACVoltageTxt = 63;
    uint16_t xACVoltageCircle = 42;
    uint16_t yACVoltageCircle = 65;

    uint16_t xDCVoltageTxt = 185;
    uint16_t yDCVoltageTxt = 43;
    uint16_t xDCVoltageCircle = 165;
    uint16_t yDCVoltageCircle = 45;

    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
    tft.setCursor(20, 43);
    tft.print("AC Supply");
    tft.setCursor(152, 25);
    tft.print("DC Control");
    //Serial.println("AC VOltage LED co-ords: Page 1");
    ArraySize = labelLEDS(ledACVoltageLabels, ledACVoltageArraySize, ledSpacing, xACVoltageTxt, yACVoltageTxt);
    drawLEDArrays(ArraySize, ledSpacing, xACVoltageCircle, yACVoltageCircle);
    //Serial.println("DC VOltage LED co-ords: Page 1");
    ArraySize = labelLEDS(ledDCVoltageLabels, ledDCVoltageArraySize, ledSpacing, xDCVoltageTxt, yDCVoltageTxt);
    drawLEDArrays(ArraySize, ledSpacing, xDCVoltageCircle, yDCVoltageCircle);
    getDCVoltageActuals();
    
    // I2C Module Status

    uint16_t xI2CModuleTxt = 345;
    uint16_t yI2CModuleTxt = 63;
    uint16_t xI2CModuleCircle = 325;
    uint16_t yI2CModuleCircle = 65;

    tft.setTextColor(LightSalmon);
    tft.setCursor(310, 43);
    tft.print("Status");

    ArraySize = labelLEDS(ledI2CModuleLabels, ledI2CModuleArraySize, ledSpacing, xI2CModuleTxt, yI2CModuleTxt);
    drawLEDArrays(ArraySize, ledSpacing, xI2CModuleCircle, yI2CModuleCircle);
  }
}
//---------------------------------------------------------------------------------
uint16_t labelTankLevels(const char* arrayName[], uint16_t arraySize, uint16_t spacing, uint16_t xPosT, uint16_t yPosT) {

  for (uint16_t idx = 0; idx < arraySize - 1; idx++) {
    tft.setTextSize(1);
    tft.setTextColor(LightSteelBlue);
    tft.setCursor(xPosT, yPosT);
    yPosT = yPosT + spacing;
    tft.print(arrayName[idx]);
    // Serial.print(idx) && Serial.print(". ") && Serial.println(arrayName[idx]);
  }
  return arraySize;
}
//---------------------------------------------------------------------------------
void drawTankLevelArrays(uint16_t Size, uint16_t spacing, uint16_t xPosRect, uint16_t yPosRect) {

  uint16_t rectHeight = 20;
  uint16_t rectWidth = 20;
  for (uint16_t idx = 0; idx < Size; idx++) {
    if (idx == Size - 1) {
      break;
    }
    tft.drawRect(xPosRect, yPosRect, rectWidth, rectHeight, White);
    switch (idx) {
      case 0: {
          tft.fillRect(xPosRect + 1, yPosRect + 1, rectWidth - 2, rectHeight - 2, Red);
          break;
        }
      case 1: {
          tft.fillRect(xPosRect + 1, yPosRect + 1, rectWidth - 2, rectHeight - 2, Orange);
          break;
        }
      case 2: {
          tft.fillRect(xPosRect + 1, yPosRect + 1, rectWidth - 2, rectHeight - 2, Yellow);
          break;
        }
      case 3: {
          tft.fillRect(xPosRect + 1, yPosRect + 1, rectWidth - 2, rectHeight - 2, Green);
          break;
        }
      case 4: {
          tft.fillRect(xPosRect + 1, yPosRect + 1, rectWidth - 2, rectHeight - 2, Blue);
          break;
        }
    }
    yPosRect = yPosRect + spacing;
    // Serial.print(idx) && Serial.print(". ") && Serial.print(xPosRect) && Serial.print(",") &&  Serial.println(yPosRect);
  }
}
//---------------------------------------------------------------------------------
void showTankLevelDefinitions() {

  uint16_t ArraySize = 0;
  uint16_t tankLevelSpacing = -40;
  uint16_t xPosT = 200;
  uint16_t yPosT = 230;
  uint16_t xPosRect = 165;
  uint16_t yPosRect = 225;

  ArraySize = labelTankLevels(tankLevelLabels, tankLevelArraySize, tankLevelSpacing, xPosT, yPosT);
  drawTankLevelArrays(ArraySize, tankLevelSpacing, xPosRect, yPosRect);
}
//---------------------------------------------------------------------------------
void buildPageFrame() {
  //Page Frame Co-ords
  const uint16_t frameBottom = 308;
  const uint16_t frameTop = 10;
  const uint16_t frameLeft = 1;
  const uint16_t frameRight = 479;

  tft.drawLine(1, 10, 1, 308, ForestGreen);
  tft.drawLine(1, 10, 479, 10, ForestGreen);
  tft.drawLine(1, 10, 479, 10, ForestGreen);
  tft.drawLine(1, 308, 479, 308, ForestGreen);
  tft.drawLine(479, 10, 479, 308, ForestGreen);
}
//---------------------------------------------------------------------------------
void getDCVoltageActuals() {

  if (currentPage == 1) {

    tft.setTextSize(1);

    //DC Voltages
    tft.setTextColor(White, Black);
    if ((read12v() > v12vLL) && (read12v() < v12vUL)) {
      tft.setCursor(250, 43);
      tft.print(read12v())&& tft.print("v"); 
    }
     else if ((read12v() < v12vLL) || (read12v() > v12vUL))  {
      tft.setTextColor(Red, Black);
      tft.setCursor(250, 43);
      tft.print(read12v())&& tft.print("v");
       if (read12v() < 10.0)  {
        tft.print(" ") && tft.print(rxBatteryVoltageLevel)&& tft.print("v");
      }
    }
    if ((read7v() > v7vLL) && (read7v() < v7vUL)) {
      tft.setTextColor(White, Black);
      tft.setCursor(250, 63);
      tft.print(read7v())&& tft.print("v");
    }
    else if ((read7v() < v7vLL) || (read7v() > v7vUL))  {
      tft.setCursor(250, 63);
      tft.setTextColor(Red, Black);
      tft.print(read7v())&& tft.print("v");
    }
    if ((read5v() > v5vLL) && (read5v() < v5vUL)) {
      tft.setTextColor(White, Black);
      tft.setCursor(250, 83);
      tft.print(read5v())&& tft.print("v");
    }
    else if ((read5v() < v5vLL) || (read5v() > v5vUL))  {
      tft.setTextColor(Red, Black);
      tft.setCursor(250, 83);
      tft.print(read5v())&& tft.print("v");
    }
    if ((read3v() > v3vLL) && (read3v() < v3vUL)) {
      tft.setTextColor(White, Black);
      tft.setCursor(250, 103);
      tft.print(read3v())&& tft.print("v");
    }
    else if ((read3v() < v3vLL) || (read3v() > v3vUL))  {
      tft.setTextColor(Red, Black);
      tft.setCursor(250, 103);
      tft.print(read3v())&& tft.print("v");
    }
    if ((rxBatteryVoltageLevel > v12vLL) && (rxBatteryVoltageLevel < v12vUL)) {
      tft.setTextColor(White, Black);
      tft.setCursor(250, 123);
      tft.print(rxBatteryVoltageLevel)&& tft.print("v");
    }
    else if ((rxBatteryVoltageLevel < v12vLL) || (rxBatteryVoltageLevel > v12vUL))  {
      tft.setTextColor(Red, Black);
      tft.setCursor(250, 123);
      tft.print(rxBatteryVoltageLevel)&& tft.print("v");
      if (rxBatteryVoltageLevel < 10.0)  {
        tft.print(" ") && tft.print(rxBatteryVoltageLevel)&& tft.print("v");
      }
    }
    else 
    if ((rxSensorControl5v > v5vLL) && (rxSensorControl5v < v5vUL)) {
      tft.setTextColor(White, Black);
      tft.setCursor(250, 143);
      tft.print(rxSensorControl5v)&& tft.print("v");
    }
    else if ((rxSensorControl5v < v5vLL) || (rxSensorControl5v > v5vUL))  {
      tft.setTextColor(Red, Black);
      tft.setCursor(250, 143);
      tft.print(rxSensorControl5v)&& tft.print("v");
    }
  }
}
//---------------------------------------------------------------------------------
float outOfLimitsCheckActual(float rxActualLevel) {
  if (rxActualLevel <= 0) {
    rxActualLevel = 0;
    return rxActualLevel;
  }
  else if (rxActualLevel > 0 && rxActualLevel < rxFullSetPoint) {
    rxActualLevel = rxFullSetPoint;
    return rxActualLevel;
  }
  else if (rxActualLevel > rxTankHeight) {
    rxActualLevel = rxTankHeight;
    return rxActualLevel;
  }
  else if (rxActualLevel >= rxFullSetPoint && rxActualLevel <= rxTankHeight) {
    return rxActualLevel;
  }
}
//---------------------------------------------------------------------------------
float outOfLimitsCheckPercentage(float rxPercentageLevel) {

  if (rxPercentageLevel < 0) {
    rxPercentageLevel = 0;
    return rxPercentageLevel;
  }
  else if (rxPercentageLevel > 100) {
    rxPercentageLevel = 100;
    return rxPercentageLevel;
  }
  else if (rxPercentageLevel >= 0 && rxPercentageLevel <= 100) {
    return rxPercentageLevel;
  }
}
//---------------------------------------------------------------------------------
void drawTankLevel(float rxActualLevel) {

  if (currentPage == 3) { //tank page
    uint16_t xPosRectStart = 20;
    uint16_t yPosRectStart = 245;
    uint16_t rectWidth = 100;
    uint16_t rectHeight = -180;
    uint16_t txtSpace = 0;
    float tankLevelPtr = 0.0;

    //Setup dividers and headings additional to frame for tank page
    tft.drawLine(140, 10, 140, 308, ForestGreen);
    tft.drawLine(310, 10, 310, 308, ForestGreen);
    tft.setTextSize(2);
    tft.setTextColor(ForestGreen);
    tft.setCursor(50, 12);
    tft.print("Tank");
    tft.setCursor(155, 12);
    tft.print("Level Colors");
    tft.setCursor(355, 12);
    tft.print("Details");

    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
    tft.setCursor(60, 45);
    tft.print("Full");
    tft.setCursor(55, 260);
    tft.print("Empty");

    tankLevelPtr = mapTankLevelPtr(rxPercentageLevel, yPosRectStart, rectHeight);
    //Serial.println("Tank Sections co-ords: Page 1");
    tft.drawRect(xPosRectStart, yPosRectStart, rectWidth, rectHeight, White);  //tank
    updateTankLevel (rxPercentageLevel, xPosRectStart, yPosRectStart, rectWidth, tankLevelPtr, txtSpace);
  }

  else if (currentPage == 2) { //control page
    uint16_t xPosRectStart = 420;
    uint16_t yPosRectStart = 220;
    uint16_t rectWidth = 50;
    uint16_t rectHeight = -150;
    uint16_t txtSpace = 0;
    float tankLevelPtr = 0.0;

    //Setup dividers and headings additional to frame for control page
    tft.setTextSize(2);
    tft.setTextColor(ForestGreen);
    tft.setCursor(420, 12);
    tft.print("Tank");

    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
    tft.setCursor(433, 50);
    tft.print("Full");
    tft.setCursor(428, 235);
    tft.print("Empty");

    tankLevelPtr = mapTankLevelPtr(rxPercentageLevel, yPosRectStart, rectHeight);
    tft.drawRect(xPosRectStart, yPosRectStart, rectWidth, rectHeight, White);
    updateTankLevel (rxPercentageLevel, xPosRectStart, yPosRectStart, rectWidth, tankLevelPtr, txtSpace);
  }
}
//----------------------------------------------------------------------
void getTankDetails() {

  if (currentPage == 3) { //tankPage
    tft.setTextSize(1);
    tft.setTextColor(LightSalmon, Black);
    tft.setCursor(315, 70);
    tft.print("Height: ");
    tft.setCursor(315, 85);
    tft.print("Low: ");
    tft.setCursor(315, 100);
    tft.print("High: ");
    tft.setCursor(315, 115);
    tft.print("Level from Top: ");
    tft.setCursor(315, 145 );
    tft.print("Tank(s) Capacity: ");
    tft.setCursor(315, 175);
    tft.print("Water Available (L): ");
    tft.setCursor(315, 215);
    tft.print("Water Available (%): ");

    tft.setTextColor(LightSteelBlue, Black);
    tft.setCursor(418 , 70);
    if (rxTankHeight == 0 ) {
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
    }
    else {
      tft.setTextColor(LightSteelBlue, Black);
      tft.print(rxTankHeight) && tft.print("cm");
    }
    tft.setCursor(418, 85);
    if (rxRefillSetPoint == 0 ) {
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
    }
    else {
      tft.setTextColor(LightSteelBlue, Black);
      tft.print(rxRefillSetPoint) && tft.print("cm");
    }
    tft.setCursor(418, 100);
    if (rxFullSetPoint == 0 ) {
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
    }
    else {
      tft.setTextColor(LightSteelBlue, Black);
      tft.print(rxFullSetPoint)&& tft.print("cm");
    }
    tft.setCursor(418, 115);
    if (rxActualLevel == 0 ) {
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
    }
    else {
      tft.setTextColor(LightSteelBlue, Black);
      tft.print(rxActualLevel)&& tft.print("cm");
    }
    tft.setTextColor(LightSteelBlue, Black);
    tft.setCursor(418, 145);
    if (calcTotalCapacity() == 0 ) {
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
    }
    else {
      tft.setTextColor(LightSteelBlue, Black);
      tft.print(calcTotalCapacity())&& tft.print("L"); // Total capcity of all tanks
    }
    tft.setTextSize(2);
    tft.setCursor(350, 190);
    tft.setTextColor(White, Black);

    if (rxPercentageLevel == 0) {
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
      //  tft.print(calcTankVolume())&& tft.print("L");
    }
    else if (rxPercentageLevel > 0 && rxPercentageLevel < 10) {
      //  tft.setTextColor(Red, Black);
      tft.print(calcTankVolume())&& tft.print("L");
    }
    else if (rxPercentageLevel >= 10 && rxPercentageLevel < 50) {
      // tft.setTextColor(Orange, Black);
      tft.print(calcTankVolume())&& tft.print("L");
    }
    else if (rxPercentageLevel >= 50 && rxPercentageLevel < 75) {
      //  tft.setTextColor(Yellow, Black);
      tft.print(calcTankVolume())&& tft.print("L");
    }
    else if  (rxPercentageLevel >= 75 &&  rxPercentageLevel < 95) {
      // tft.setTextColor(Green, Black);
      tft.print(calcTankVolume())&& tft.print("L");
    }
    else if (rxPercentageLevel > 95) {
      // tft.setTextColor(Blue, Black);
      tft.print(calcTankVolume())&& tft.print("L");
    }
    else {
      tft.print(calcTankVolume())&& tft.print("L");
    }

    //displays percentage value of water available

    if (rxPercentageLevel == 0) {
      tft.setCursor(350, 230);
      tft.setTextColor(Red, Black);
      tft.print("ERROR!  ");
    }
    else {
      tft.setCursor(350, 230);
      tft.setTextColor(Black);
      tft.print(rxPercentageLevel)&& tft.println("%");
      tft.setCursor(350, 230);
      tft.setTextColor(White, Black);
      tft.print(rxPercentageLevel)&& tft.println("%");
    }
  }
  getAlarmStatus();
}
//---------------------------------------------------------------------------------
int voltageOutOfLimits()
{
if (((read12v()  < v12vLL) || (read12v()  > v12vUL)) ||
      ((read7v()  < v7vLL) || (read7v()  > v7vUL)) ||
      ((read5v()  < v5vLL) || (read5v()  > v5vUL)) ||
      ((read3v()  < v3vLL) || (read3v()  > v3vUL))) {

        return 1;
      }
      else {
        return 0;
      }
}
//---------------------------------------------------------------------------------
float calcTotalCapacity() {

  float totalCapacity = ((tankArea * (rxTankHeight - rxFullSetPoint)) * numTanks) / 10;

  //Serial.print("Total Capacity: ") && Serial.println(totalCapacity);
  return totalCapacity;
}
//---------------------------------------------------------------------------------
float calcTankVolume() {

  float tankCurrentVolume = (calcTotalCapacity() * rxPercentageLevel) / 100;
  return tankCurrentVolume;
}
//---------------------------------------------------------------------------------
float mapTankLevelPtr(float rxPercentageLevel, uint16_t yPosRectStart, uint16_t rectHeight) {

  float tankLevelPtr = map(rxPercentageLevel, 0, 100, yPosRectStart, yPosRectStart - rectHeight);

  // Serial.print("tank ptr: ") && Serial.println(tankLevelPtr);
  return tankLevelPtr;
}
//---------------------------------------------------------------------------------
void updateTankLevel (float rxPercentageLevel, uint16_t xPosRectStart, uint16_t yPosRectStart, uint16_t rectWidth, float ptr, uint16_t txtSpace) {

  if (currentPage == 3) { //tank page
    txtSpace = 32;
    ptrOffset = 248;
    ptrOffsetFull = 424;
    ptrOffsetEmpty = 10;
    ptrOffsetPercentValueLabel = 180;
  }
  if (currentPage == 2) { //control page
    txtSpace = 7;
    ptrOffset = 220;
    ptrOffsetFull = 370;
    ptrOffsetEmpty = 10;
    ptrOffsetPercentValueLabel = 150;
  }
  if (rxPercentageLevel > 100) { //tank > 100% reading
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Blue);
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptrOffsetPercentValueLabel);
  }
  else {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank >95% & < 100% reading
  }
  if ((rxPercentageLevel >= 95) && (rxPercentageLevel <= 100)) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Blue);
    tft.setTextColor(White);
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank >95% & < 100% reading
  }
  else if (rxPercentageLevel >= 75 && rxPercentageLevel < 95) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Green);
    tft.setTextColor(White);
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <95% reading
  }
  else if (rxPercentageLevel >= 50 && rxPercentageLevel < 75) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Yellow);
    tft.setTextColor(Black);

    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <75% reading
  }
  else if (rxPercentageLevel >= 10 && rxPercentageLevel < 50) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Orange);
    tft.setTextColor(White);

    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <50% reading
  }
  else if (rxPercentageLevel < 10) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Red);
    tft.setTextColor(White);
    if (rxPercentageLevel > 0 && rxPercentageLevel < 6) {
      tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

      tft.setTextColor(Red);
      tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptrOffsetEmpty); //tank <6% reading
    }
    else if (rxPercentageLevel == 0) {
      tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'
      tft.setTextColor(Red);
      if (currentPage == 3) {
        tft.setCursor(40, 150);
        tft.setTextSize(2);
      }
      else if (currentPage == 2) {
        tft.setCursor(430, 150);
        tft.setTextSize(1);
      }
      tft.print("ERROR");
      tft.setTextColor(Black);
      tft.setTextSize(1);
      tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptrOffsetEmpty); //tank ==0% reading
    }
    else {
      tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <10% reading
    }
  }
  else {
    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
  }
  tft.print(rxPercentageLevel)&& tft.print("%");
}
//---------------------------------------------------------------------------------
void drawLEDArrays(uint16_t Size, uint16_t spacing, uint16_t xPosCirc, uint16_t yPosCirc) {

  for (uint16_t idx = 0; idx < Size; idx++) {

    if (idx == Size - 1) {
      break;
    }
    tft.drawCircle(xPosCirc, yPosCirc, ledRadius, White);
    yPosCirc = yPosCirc + spacing;
    // Serial.print(idx) && Serial.print(". ") && Serial.print(xPosCirc) && Serial.print(",") &&  Serial.println(yPosCirc);
  }
}
//---------------------------------------------------------------------------------
uint16_t labelLEDS(const char* arrayName[], uint16_t arraySize, uint16_t spacing, uint16_t xPosT, uint16_t yPosT) {

  for (uint16_t idx = 0; idx < arraySize - 1; idx++) {
    tft.setTextSize(1);
    tft.setTextColor(LightSteelBlue);
    tft.setCursor(xPosT, yPosT);
    yPosT = yPosT + spacing;
    if (arrayName[idx] == "Sensor+12v" || arrayName[idx] == "Sensor+5v") { //make sensor voltages orange in color
      tft.setTextColor(Orange);
    }
    tft.print(arrayName[idx]);
    // Serial.print(idx) && Serial.print(". ") && Serial.println(arrayName[idx]);
  }
  return arraySize;
}
//---------------------------------------------------------------------------------
uint16_t labelRelayLEDS(const char* arrayName[], uint16_t arraySize, uint16_t spacing, uint16_t xPosT, uint16_t yPosT) {

  for (uint16_t idx = 0; idx < arraySize - 1; idx++) {
    tft.setTextSize(1);
    tft.setTextColor(LightSteelBlue);
    tft.setCursor(xPosT, yPosT);
    yPosT = yPosT + spacing + 30;

    tft.print(arrayName[idx]);
    //Serial.print(idx) && Serial.print(". ") && Serial.println(arrayName[idx]);
  }
  return arraySize;
}
//---------------------------------------------------------------------------------
void drawRelayLEDArrays(uint16_t spacing, uint16_t xPosCirc, uint16_t yPosCirc) {

  uint16_t Size = 5;
  uint16_t idxCount = 0;

  for (uint16_t idx = 0; idx < Size - 1; idx++) {
    idxCount += 1;
    if (idxCount == 3) {
      yPosCirc = yPosCirc + spacing + 10;
    }
    else {
      yPosCirc = yPosCirc + spacing;
    }
    tft.drawCircle(xPosCirc, yPosCirc, ledRadius, White);

    // Serial.print(idx) && Serial.print(". ") && Serial.print(xPosCirc) && Serial.print(",") &&  Serial.println(yPosCirc);
  }
}
//---------------------------------------------------------------------------------
uint16_t labelRelayControls(const char* arrayName[], uint16_t arraySize, uint16_t spacing, uint16_t xPosT, uint16_t yPosT) {
  spacing = 20;
  uint16_t idxCount = 0;
  for (uint16_t idx = 0; idx < arraySize - 1; idx++) {
    idxCount += 1;
    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
    tft.setCursor(xPosT, yPosT);
    if (idxCount == 2) {
      yPosT = yPosT + spacing + 10;
    }
    else {
      yPosT = yPosT + spacing;
    }
    tft.print(arrayName[idx]);
    // Serial.print(idx) && Serial.print(". ") && Serial.println(arrayName[idx]);
  }
  return arraySize;
}
//---------------------------------------------------------------------------------
void returnMainMenu() {  // Return Button to main menu
  if (currentPage != 0) {
    returnBtn.initButton(&tft, returnBtn_X, returnBtn_Y, returnBtn_W, returnBtn_H, Black, buttonColor , buttonTextColor , (char*) "Return", returnBtn_TxtSize); // x, y, w, h, outline, fill, text, text size
    returnBtn.drawButton();
  }
}
//--------------------------------------------------------------------------------------
void showTFTDetails() {

  uint16_t tftID = tft.readID();
  Serial.println(F("---------------------------------------------------"));
  Serial.print("TFT Screen Type: ") && Serial.println(screenType);
  Serial.print("Resolution: " )&& Serial.print(tft.width()) && Serial.print("x") && Serial.println(tft.height());
  Serial.print("TFT ID = 0x") && Serial.println(tftID, HEX);
  tft.setTextColor(LightSkyBlue);
  tft.setCursor(100, 40);
  tft.print("TFT Screen Type: 3.5 inch MCUFriend: ")&& tft.print(tft.width()) && tft.print("x") && tft.println(tft.height());
  tft.setCursor(100, 60);
  tft.print("TFT ID = 0x") && tft.println(tftID, HEX);
  tft.setCursor(100, 80);
  tft.print("TFT Libraries Used: ");
  tft.setCursor(240, 80);
  tft.print("Adafruit_GFX");
  tft.setCursor(240, 100);
  tft.print("MCUFRIEND_kbv");
  tft.setCursor(240, 120);
  tft.print("Adafruit TouchScreen");
}
//-------------------------------------------------------------------------------
//----------------- Controller FUNCTIONS------------------------------------//
//-------------------------------------------------------------//
void scanI2C() {
  Serial.println(F("I2C scanner. Scanning ..."));
  byte count = 0;

  Wire.begin();
  for (byte i = 8; i < 127; i++)
  {
    Wire.beginTransmission (i);

    if (Wire.endTransmission () == 0)
    {
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println (")");
      if (i == 64) { //address 64 (0x40)
        Serial.println(F("Servo PWM Controller (PCA9685A) Installed."));
        servoControllerFound = true;
      }
      if (i == 104) { //address  104 (0x68)
        Serial.println(F("Mini Logger (RTC DS1307 & SD) Module Installed."));
        miniDataLoggerModuleFound = true;
      }
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
    else if (Wire.endTransmission () == 4)
    {
      Serial.print("Unknown error at address 0x");
      //  digitalWrite(alarmLED, HIGH);
      if (i < 16)
        Serial.print("0");
      Serial.println(i, HEX);
    }  // end of error response

  } // end of for loop

  Serial.println();
  Serial.println ("Done.");
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println (" device(s).");
}
//-------------------------------------------------------------//
void logTime() {

  if (sdOk == true) {
    DateTime DTG = rtc.now();
    if (rtc.begin()) {
      if (DTG.day() < 10) { //Print a 0 IN FRONT OF THE DAYS IF LESS THAN 10
        dataFile.print('0');
        dataFile.print(DTG.day(), DEC);
      }
      else {
        dataFile.print(DTG.day(), DEC);
      }
      dataFile.print('/');

      if (DTG.month() < 10) { //Print a 0 IN FRONT OF THE MONTHS IF LESS THAN 10
        dataFile.print('0');
        dataFile.print(DTG.month(), DEC);
      }
      else {
        dataFile.print(DTG.month(), DEC);
      }
      dataFile.print('/');
      dataFile.print(DTG.year(), DEC);
      dataFile.print(',');

      if (DTG.hour() < 10) { //Print a 0 IN FRONT OF THE HOUR IF LESS THAN 10
        dataFile.print('0');
        dataFile.print(DTG.hour(), DEC);
      }

      else {
        dataFile.print(DTG.hour(), DEC);
      }
      dataFile.print(':');

      if (DTG.minute() < 10) { //Print a 0 IN FRONT OF THE MINUTES IF LESS THAN 10
        dataFile.print('0');
        dataFile.print(DTG.minute(), DEC);
      }
      else {
        dataFile.print(DTG.minute(), DEC);
      }
      dataFile.print(':');

      if (DTG.second() < 10) { //Print a 0 IN FRONT OF THE SECONDS IF LESS THAN 10
        dataFile.print('0');
        dataFile.print(DTG.second(), DEC);
      }
      else {
        dataFile.print(DTG.second(), DEC);
      }
      dataFile.print(',');
    }
    else {
      dataFile.print("DTG Error");
      //   digitalWrite(alarmLED, HIGH);
    }
  }
}
//-------------------------------------------------------------//
void logData(float rxActualLevel, float rxPercentageLevel, float rxTemperatureLevel, float rxBatteryVoltageLevel) {

  if  (isnan(rxActualLevel) || isnan(rxTemperatureLevel) || isnan(rxBatteryVoltageLevel)) {
    Serial.println(F("Failed to read from Sensors!"));
  }
  dataFile.close();
  dataFile = SD.open(dataFilename, FILE_WRITE);

  if (dataFile) {
    Serial.print(dataFilename)&& Serial.println(" opened successfully for writing data....");
    logTime();
    dataFile.print(rxActualLevel);
    dataFile.print(",");
    dataFile.print(rxPercentageLevel);
    dataFile.print(",");
    dataFile.print(rxTemperatureLevel);
    dataFile.print(",");
    dataFile.print(read12v());
    dataFile.print(",");
    dataFile.print(read7v());
    dataFile.print(",");
    dataFile.print(read5v());
    dataFile.print(",");
    dataFile.print(read3v());
    dataFile.print(",");
    dataFile.print(rxBatteryVoltageLevel);
    dataFile.println(",");
    dataFile.print(rxSensorControl5v);
    dataFile.println(",");
    dataFile.close();
    Serial.print("Writing data to file: ") && Serial.print(dataFilename)&& Serial.println(" completed");
    Serial.println();
  }
  else {
    Serial.print("Error! SD Card file: ") && Serial.print(dataFilename) && Serial.println(" failed to open for writing.");
    Serial.println();
    dataFile.close();
  }
}
//-------------------------------------------------------------//
float read12v() {

  uint16_t sampleCountTotal = 10;
  float sumReading = 0;
  uint16_t sampleCount = 0;

  float dividerRatio12v = 0.0;

  float sensorV12v = 0.0;
  
  while (sampleCount < sampleCountTotal) {
    sumReading += analogRead(analogInputA10);
    sampleCount++;
    delay(5);
  }
  if (!Serial) {
      dividerRatio12v = 14.2;
    }    
   else if (Serial) {
      dividerRatio12v = 14.0;
    }
  //Serial.print("Analogue value12v: ") && Serial.println(sumReading / sampleCountTotal);
  sensorV12v = (((float(sumReading) / float(sampleCountTotal)) * vPinMax) / 1024.0)* dividerRatio12v;
  //  Serial.print("Sensor Voltage Out12v: ") && Serial.print(sensorV12v) && Serial.println(" Volts");


  dividerRatio12v = 0.0;
  return sensorV12v;
}
//-------------------------------------------------------------//
float read7v() {
  uint16_t sampleCountTotal = 10;
  float sumReading = 0.0;
  uint16_t sampleCount = 0;
  float vIn7v = 0.0;
  //const float dividerRatio7v = 13.22;
  const float dividerRatio7v = 12.00;
  float sensorV7v = 0.0;

  while (sampleCount < sampleCountTotal) {
    sumReading += analogRead(analogInputA11);
    sampleCount++;
    delay(10);
  }
  sensorV7v = (float(sumReading) / float(sampleCountTotal) * vPinMax) / 1024.0;
  //Serial.print("Sensor Voltage Out7v: ") && Serial.print(sensorV7v) && Serial.println(" Volts");
  vIn7v = sensorV7v * dividerRatio7v;

  return vIn7v;
}
//-------------------------------------------------------------//
float read5v() {
  uint16_t sampleCountTotal = 10;
  float sumReading = 0.0;
  uint16_t sampleCount = 0;
  float vIn5v = 0.0;
  const float dividerRatio5v = 13.22;
  float sensorV5v = 0.0;

  while (sampleCount < sampleCountTotal) {
    sumReading += analogRead(analogInputA12);
    sampleCount++;
    delay(10);
  }
  sensorV5v = (float(sumReading) / float(sampleCountTotal) * vPinMax) / 1024.0;
  // Serial.print("Sensor Voltage Out5v: ") && Serial.print(sensorV5v) && Serial.println(" Volts");
  vIn5v = sensorV5v * dividerRatio5v;
  return vIn5v;
}
//-------------------------------------------------------------//
float read3v() {
  uint16_t sampleCountTotal = 10;
  float sumReading = 0.0;
  uint16_t sampleCount = 0;
  float vIn3v = 0.0;
  const float dividerRatio3v = 13.00;
  float sensorV3v = 0.0;

  while (sampleCount < sampleCountTotal) {
    sumReading += analogRead(analogInputA13);
    sampleCount++;
    delay(10);
  }
  sensorV3v = (float(sumReading) / float(sampleCountTotal) * vPinMax) / 1024.0;
  // Serial.print("Sensor Voltage Out3v: ") && Serial.print(sensorV3v) && Serial.println(" Volts");
  vIn3v = sensorV3v * dividerRatio3v;
  return vIn3v;
}
//-------------------------------------------------------------//
void showSupplyVoltages() {
  String str12v = "";
  String str7v = "";
  String str5v = "";
  String str3v = "";
  Serial.println(F("-------------------------------------------------"));
  Serial.println("Voltages Summary");
  Serial.println("-------------------------------------------------");
  Serial.print("12v Supply: ") && Serial.print(read12v()) && Serial.println(" Volts");
  str12v = String(read12v(), 2);
  str12v = "I" + str12v;
  contVolValue[1] = str12v;
  //Serial.println(contVolValue[1]);

  Serial.print("7v Supply: ") && Serial.print(read7v()) && Serial.println(" Volts");
  str7v = String(read7v(), 2);
  str7v = "J" + str7v;
  contVolValue[2] = str7v;
  // Serial.println(contVolValue[2]);

  Serial.print("5v Supply: ") && Serial.print(read5v()) && Serial.println(" Volts");
  str5v = String(read5v(), 2);
  str5v = "K" + str5v;
  contVolValue[3] = str5v;
  //Serial.println(contVolValue[3]);

  Serial.print("3v3 Supply: ") && Serial.print(read3v()) && Serial.println(" Volts");
  str3v = String(read3v(), 2);
  str3v = "L" + str3v;
  contVolValue[4] = str3v;
  // Serial.println(contVolValue[4]);

  //Serial.println("----------------------------------------------------------------");

  getDCVoltageActuals();
}
//-------------------------------------------------------------//
bool serialSetup() {

  if (serialSetupCount == 0) {
    while (!Serial) {
      Serial.print("Serial0 Not Connected "); ; // wait for serial port to connect. Needed for native USB port only
    }    Serial.print("Serial monitor available?....");
    if (Serial) {
      Serial.println("OK");
    }
    while (!Serial1) {
      Serial.print("Serial1 Not Connected "); // wait for serial port to connect. Needed for native USB port only
    }
    Serial.print("HC-12 (Serial1) available?....");
    Serial1.begin(baud);
    if (Serial1) {
      Serial1.write("AT+DEFAULT");
      Serial.println("OK");
    }
    else {
      Serial.println("HC-12 (Serial1) Not Available");
      return false;
    }

    while (!Serial2) {
      Serial.print("Serial2 Not Connected "); // wait for serial port to connect. Needed for native USB port only
      serialWifiFound = false;
    }
    Serial2.begin(baud); //Web Serial, connected to ESP8266
    Serial.print("Web Serial (Serial2) link available?....");
    if (Serial2) {
      serialWifiFound = true;
      Serial.println("OK");
    }
    else {
      Serial.println(F("Web Serial (Serial2) link NOT available... "));
      serialWifiFound = false;
    }

    serialSetupCount = 1;
  }
  return true;
}
//-------------------------------------------------------------//
bool sendControllerIndicators (char* contIndValue[], uint16_t controllerIndicatorsArraySize) {
  const char startByte = '<';
  const char stopByte = '>';
  const char heartBeat ='H';
  Serial.println();
  Serial.println("Controller Indicators to be sent to the WebPage: ");
  
  if (Serial2) {
    //Serial2.print(heartBeat);
    Serial.print(startByte);
    Serial2.print(startByte);
   // Serial.print(heartBeat);
    for (uint16_t idxInd = 0; idxInd < controllerIndicatorsArraySize; idxInd++) {
      Serial2.print(contIndValue[idxInd]) && Serial2.write(",");
      Serial.print(contIndValue[idxInd]) && Serial.print(",");
    }
    Serial2.print(stopByte);
    Serial.print(stopByte);
    Serial.println();
    Serial2.flush(); //clear Tx buffer;
    Serial.println(F("Controller Indicators sent to WebPage: "));
    Serial.println();
    return true;
  }
  else {
    Serial.println(F("No Controller Indicators sent to the WebPage"));
    Serial.println();
    return false;
  }
}
//-------------------------------------------------------------//
bool sendControllerVoltages (String contVolValue[], uint16_t controllerVoltagesArraySize) {
  const char startByte = '<';
  const char stopByte = '>';

  Serial.println("Controller Voltages to be sent to the WebPage: ");
  Serial.print(startByte);
  Serial2.print(startByte);
  if (Serial2) {
    for (uint16_t idxVol = 0; idxVol < controllerVoltagesArraySize; idxVol++) {
      Serial2.print(contVolValue[idxVol]) && Serial2.write(",");
      Serial.print(contVolValue[idxVol]) && Serial.print(",");
    }
    Serial2.print(stopByte);
    Serial.print(stopByte);
    Serial.println();
    Serial2.flush(); //clear Tx buffer;
    Serial.println(F("Controller Voltages sent to the WebPage: "));
    Serial.println();
    return true;
  }
  else {
    Serial.println(F("No Controller Voltages sent to the WebPage"));
    Serial.println();
    return false;
  }
}
//-------------------------------------------------------------//
bool sendSensorData(char* buffer, byte idx) {
  const char startByte = '<';
  const char stopByte = '>';

  Serial.println(F("Sensor Data to be sent to the WebPage: "));
  if (Serial2 && idx > 0) {
    Serial2.print(startByte);
    Serial.print(startByte);
    for (uint16_t idxSend = 0; idxSend <= idx; idxSend++) {
      Serial2.print(buffer[idxSend]);
      Serial.print(buffer[idxSend]);
    }

    Serial2.print(stopByte);
    Serial.println(stopByte);

    Serial2.flush(); //clear Tx buffer
    Serial.println(F("Sensor Data sent to the WebPage"));
    Serial.println();
    return true;
  }
  else {
    Serial.println(F("No Sensor Data sent to the WebPage"));
    Serial.println();
    return false;
  }
}
//-------------------------------------------------------------//
bool processSensorData(char* buffer, byte idxData) {
  int maxValueLength = 10;

  for (uint16_t idx = 0; idx < idxData; idx++) {

    if (buffer[idx] == 'A') {   //Available Capacity
      for (uint16_t idxSub = 1; idxSub < maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxAvailableVolume = rxTempString.toFloat(); //convert to float
      Serial.print("rxAvailableVolume: ") &&  Serial.println( rxAvailableVolume);
      rxTempString = "";
    }
    if (buffer[idx] == 'B') {   //Sensor Battery Voltage Level
      for (uint16_t idxSub = 1; idxSub < maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxBatteryVoltageLevel = rxTempString.toFloat(); //convert to float
      Serial.print("rxBatteryVoltageLevel: ") &&  Serial.println(rxBatteryVoltageLevel);
      rxTempString = "";
    }
    if (buffer[idx] == 'C') {    //Total Capacity
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxTotalCapacity = rxTempString.toFloat(); //convert to float
      Serial.print("rxTotalCapacity: ") &&  Serial.println(rxTotalCapacity);
      rxTempString = "";
    }
    if (buffer[idx] == 'D') {     //Actual level
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }

      rxActualLevel = rxTempString.toFloat(); //convert to float
      Serial.print("rxActualLevel: ") &&  Serial.println(rxActualLevel);
      rxTempString = "";
    }
    if (buffer[idx] == 'F') {     //Full SetPoint
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxFullSetPoint = rxTempString.toFloat(); //convert to float
      Serial.print("rxFullSetPoint: ") &&  Serial.println(rxFullSetPoint);
      rxTempString = "";
    }
    if (buffer[idx] == 'H') {     //Tank height
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxTankHeight = rxTempString.toFloat(); //convert to float
      Serial.print("rxTankHeight: ") &&  Serial.println(rxTankHeight);
      rxTempString = "";
    }
    if (buffer[idx] == 'P') {     //Percentage Level
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxPercentageLevel = rxTempString.toFloat(); //convert to float
      Serial.print("rxPercentageLevel: ") &&  Serial.println(rxPercentageLevel);
      rxTempString = "";
    }
    if (buffer[idx] == 'R') {
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxRefillSetPoint = rxTempString.toFloat(); //convert to float
      Serial.print("rxRefillSetPoint: ") &&  Serial.println(rxRefillSetPoint);
      rxTempString = "";
    }
    if (buffer[idx] == 'T') {
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxTemperatureLevel = rxTempString.toFloat(); //convert to float
      Serial.print("rxTemperatureLevel: ") &&  Serial.println(rxTemperatureLevel);
      rxTempString = "";
    }
    if (buffer[idx] == 'V') {
      for (uint16_t idxSub = 1; idxSub < idx + maxValueLength ; idxSub++) {
        if (isAlpha(buffer[idx + idxSub]) or buffer[idx + idxSub] == ',' or buffer[idx + idxSub] == '>') {
          buffer[idx] = buffer[idx + idxSub];
          break;
        }
        else {
          rxTempString += buffer[idx + idxSub];
        }
      }
      rxSensorControl5v = rxTempString.toFloat(); //convert to float
      Serial.print("rxSensorControl5v: ") &&  Serial.println(rxSensorControl5v);
      rxTempString = "";
    }
  }
  digitalWrite(radioLinkOnLED, LOW);
  Serial.println();
  rxActualLevel = outOfLimitsCheckActual(rxActualLevel);
  rxPercentageLevel = outOfLimitsCheckPercentage(rxPercentageLevel);
  tankLevelCheck(rxActualLevel, rxPercentageLevel, rxRefillSetPoint, rxFullSetPoint, rxTankHeight);

  if (rxActualLevel > 0) {
    return true;
  }
}
//-------------------------------------------------------------//
bool rxSensorData() { //read data out of buffer of serial radio and transmit to web page
  const char startByte = '<';
  const char stopByte = '>';
  static byte index = 0;

  if (serialSetup() == true) {
    digitalWrite(radioLinkOnLED, HIGH);
    if (Serial1.available()) {
      Serial.println();
      Serial.println("Serial1 available to rx data.");

      while (Serial1.available() > 0 ) {
        char inChar = Serial1.read();
        // Serial.print("InChar[") && Serial.print(index) && Serial.print("]") && Serial.println(inChar);
        if (inChar == startByte) { // If start byte is received
          index = 0;
        } else if (inChar == stopByte) { // If end byte is received
          if (sendSensorData(buffer, index) == true) { // send Data to WebPage
            processSensorData(buffer, index); // and process the data\
          } else {
            //  Serial.println("Webdata not Sent!");
          }
          buffer[index] = '\0'; // then null terminate for end of data
          index = 0; // this isn't necessary, but helps limit overflow
        }
        else { // otherwise
          buffer[index + 1] = inChar; // put the character into our array
          index++; // and move to the next key in the array
        }
        if (index >= maxBuffer) {
          index = 0;
          Serial.println("Overflow occured, next value is unreliable");
        }
      }
      char junk = Serial1.read();
    }
    else {
      Serial.println();
      Serial.println("Serial1 NOT available to rx data.");
    }
  }
  return true;
}
//-------------------------------------------------------------//
void PrintCurrentRxedValues(float rxActualLevel, float rxPercentageLevel, float rxRefillSetPoint, float rxFullSetPoint, float rxTankHeight) {

  Serial.println();
  Serial.println(F("----------------------------------------------------------------"));
  Serial.println(F("Receive Variables Capture"));
  Serial.println(F("----------------------------------------------------------------"));

  if (rxActualLevel == 0) {
    Serial.println();
    Serial.println(F("Distance to water value not yet initialised."));
    Serial.println(F("Tank Capacity value not yet initialised."));
  }
  else {
    Serial.println();
    Serial.print("Distance to water (cm): ") &&  Serial.println(rxActualLevel);
    Serial.print("Tank Capacity (%): "); Serial.println(rxPercentageLevel);
  }

  if (rxTemperatureLevel == 0) {
    Serial.println("Current Water Temperature Sensor measuring....");
  }
  else {
    Serial.print("Current Water Temperature (ºC): ") && Serial.println(rxTemperatureLevel);
  }

  if (rxBatteryVoltageLevel <= v12vLL) {
    Serial.print("Solar Battery is reading (volts): ") &&  Serial.print(rxBatteryVoltageLevel) && Serial.println(", not yet initialised or needs attention!!");
  }
  else {
    Serial.print("Solar Battery Voltage Level (volts): ") && Serial.println(rxBatteryVoltageLevel);
  }
  if (rxRefillSetPoint == 0) {
    Serial.println(F("rxRefillSetPoint value not yet initialised."));
  }
  else {
    Serial.print("rxRefillSetPoint value: ") && Serial.println(rxRefillSetPoint);
  }
  if (rxFullSetPoint == 0) {
    Serial.println(F("rxFullSetPoint value not yet initialised."));
  }
  else {
    Serial.print("rxFullSetPoint value: ") && Serial.println(rxFullSetPoint);
  }
  if (rxTankHeight == 0) {
    Serial.println(F("rxTankHeight value not yet initialised."));
  }
  else {
    Serial.print("rxTankHeight value: ") && Serial.println(rxTankHeight);
  }
  Serial.println(F("----------------------------------------------------------------"));

  logTime();
  logData(rxActualLevel, rxPercentageLevel, rxTemperatureLevel, rxBatteryVoltageLevel);
}
//-------------------------------------------------------------//
float tankLevelCheck(float rxActualLevel, float rxPercentageLevel, float rxRefillSetPoint, float rxFullSetPoint, float rxTankHeight) {
  // Tank water level is full (<=rxFullSetPoint with pump running)

  if (rxActualLevel <= (rxFullSetPoint + fullOffset) && digitalRead(pumpRunning) == HIGH) {
    PrintCurrentRxedValues(rxActualLevel, rxPercentageLevel, rxRefillSetPoint, rxFullSetPoint, rxTankHeight);
    digitalWrite(tankEmpty, LOW);
    digitalWrite(tankFilling, LOW);
    digitalWrite(tankFull, HIGH);
    digitalWrite(tankLevelOK, HIGH);
    digitalWrite(pumpRunning, LOW);

    return rxActualLevel;
  }
  else if (rxActualLevel <= (rxFullSetPoint + fullOffset) && digitalRead(pumpRunning) == LOW) { // Tank water level is full (<=rxFullSetPoint with pump stopped)
    PrintCurrentRxedValues(rxActualLevel, rxPercentageLevel, rxRefillSetPoint, rxFullSetPoint, rxTankHeight);
    digitalWrite(tankEmpty, LOW);
    digitalWrite(tankFilling, LOW);
    digitalWrite(tankFull, HIGH);
    digitalWrite(tankLevelOK, HIGH);
    digitalWrite(pumpRunning, LOW);

    return rxActualLevel;
  }
  else if (rxActualLevel > rxFullSetPoint && rxActualLevel < rxRefillSetPoint) { //water in tank, level between rxFullSetPoint & rxRefillSetPoint
    PrintCurrentRxedValues(rxActualLevel, rxPercentageLevel, rxRefillSetPoint, rxFullSetPoint, rxTankHeight);
    switch (digitalRead(pumpRunning)) {
      case LOW:  {
          digitalWrite(tankLevelOK, HIGH);
          digitalWrite(tankFilling, LOW);
          digitalWrite(tankFull, LOW);
          digitalWrite(tankEmpty, LOW);
          break;
        }
      case HIGH: {
          digitalWrite(tankLevelOK, HIGH);
          digitalWrite(tankFilling, HIGH);
          digitalWrite(tankFull, LOW);
          digitalWrite(tankEmpty, LOW);
          break;
        }
        Serial.println();
        Serial.println("-----------------------------------------------");
        return rxActualLevel;
    }
  }
  else if (rxActualLevel > rxRefillSetPoint && rxActualLevel < rxTankHeight)  {  //tank is at low to empty mark >=tankLow & <= tankHeight & not in bypass mode
    PrintCurrentRxedValues(rxActualLevel, rxPercentageLevel, rxRefillSetPoint, rxFullSetPoint, rxTankHeight);

    Serial.println("LOW Water Level.");

    digitalWrite(tankEmpty, HIGH);
    digitalWrite(tankFull, LOW);
    digitalWrite(tankLevelOK, LOW);
    if (digitalRead(modeAuto) == LOW || digitalRead(modeManual) == LOW)  {
      digitalWrite(pumpRunning, HIGH);
      Serial.println("Pump ON.");
      digitalWrite(tankFilling, HIGH);
    }
    else if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
      digitalWrite(pumpRunning, LOW);
      Serial.println("Pump Off.");
      digitalWrite(tankFilling, LOW);
    }
    Serial.println();
    Serial.println("-----------------------------------------------");
    return rxActualLevel;
  }

  else if (rxActualLevel > rxTankHeight || rxActualLevel <= rxFullSetPoint - readErrorTolerance) { //error notification if level >rxTankHeight or <=rxFullSetPoint
    Serial.println(F("--------------Erroneous Water Level Reading."));
    Serial.print("--------------")&& Serial.println(rxActualLevel);
    Serial.println(F("--------------Investigation Required."));
    // digitalWrite(alarmLED, HIGH);
    digitalWrite(pumpRunning, LOW);
    Serial.println(F("-----------------------------------------------"));
    return rxActualLevel;
  }
  else if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    digitalWrite(tankFilling, LOW);
    digitalWrite(pumpRunning, LOW);
  }
}
//-------------------------------------------------------------//
void vacVoltageCheck() {
  if (digitalRead(vacConst) == HIGH) {
    digitalWrite(vacActiveLED, LOW);
    contVolValue[5] = "M0";
    Serial.println(F("28VAC Constant Failed"));
  }
  else if (digitalRead(vacConst) == LOW)  {
    digitalWrite(vacActiveLED, HIGH);
    contVolValue[5] = "M28";
    Serial.println(F("28VAC Constant ON"));
  }
  if (digitalRead(vacSwitched) == HIGH) {
    contVolValue[6] = "N0";
    Serial.println(F("28VAC Switched OFF/Failed"));
  }
  else if  (digitalRead(vacSwitched) == LOW)  {
    contVolValue[6] = "N28";
    Serial.println(F("28VAC Switched ON"));
  }
  Serial.println("-------------------------------------------------");
}
//-------------------------------------------------------------//
void vacRelayControl() {

  if ((digitalRead(modeAuto) == LOW) || (digitalRead(modeManual) == LOW)) {  //use 28vacSwitched or 28vacConst as 28vac Supply to drive pump control
    if (digitalRead(vacSwitched) == LOW && digitalRead(vacConst) == LOW) { //use 28vacSwitched to drive pump control
      digitalWrite(relay1, LOW);
      digitalWrite(relay2, LOW);
    }
    if (digitalRead(vacSwitched) == HIGH && digitalRead(vacConst) == LOW) { //use 28vacConst to drive pump control
      digitalWrite(relay1, HIGH);
      digitalWrite(relay2, HIGH);
    }
  }
  if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {  //bypass mode, use 28vacSwitched only (if available) to drive pump control, relays at rest
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
  }
}
//-------------------------------------------------------------//
void monStatus() {
  modeControlMessage();

  if (digitalRead(alarmLED) == LOW) {
    Serial.println(F("No Alarms Active"));
    contIndValue[14] = "a0";
  }
  else if (digitalRead(alarmLED) == HIGH) {
    Serial.println(F("System in Alarm"));
    contIndValue[14] = "a1";
  }
  if (digitalRead(modeAuto) == LOW && digitalRead(modeManual) == HIGH) {
    Serial.println(F("Auto Mode Selected"));
  }
  if (digitalRead(modeManual) == LOW && digitalRead(modeAuto) == HIGH) {
    Serial.println(F("Manual Mode Selected"));
  }
  if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    Serial.println(F("Bypass Mode Selected"));
  }
  if (digitalRead(modeAutoLED) == LOW) {
    Serial.println(F("Auto Mode LED OFF"));
  }
  else  if (digitalRead(modeAutoLED) == HIGH) {
    Serial.println(F("Auto Mode LED ON"));
  }
  if (digitalRead(modeManualLED) == LOW) {
    Serial.println(F("Manual Mode LED OFF"));
  }
  else if (digitalRead(modeManualLED) == HIGH) {
    Serial.println(F("Manual Mode LED ON"));
  }
  if (digitalRead(modeBypassLED) == LOW) {
    Serial.println(F("Bypass Mode LED OFF"));
  }
  else if (digitalRead(modeBypassLED) == HIGH) {
    Serial.println(F("Bypass Mode LED ON"));
  }
  if (digitalRead(relay1) == HIGH) {
    Serial.println(F("Relay1 Switched"));
    contIndValue[9] = "w1";
  }
  if (digitalRead(relay2) == HIGH) {
    Serial.println(F("Relay2 Switched"));
    contIndValue[10] = "x1";
  }
  if (digitalRead(relay1) == LOW) {
    Serial.println(F("Relay1 at Rest"));
    contIndValue[9] = "w0";
  }
  if (digitalRead(relay2) == LOW) {
    Serial.println(F("Relay2 at Rest"));
    contIndValue[10] = "x0";
  }
  if (digitalRead(tankFilling) == LOW) {
    Serial.println(F("tankFilling LED OFF"));
    contIndValue[7] = "t0";
  }
  if (digitalRead(tankFilling) == HIGH) {
    Serial.println(F("tankFilling LED ON"));
    contIndValue[7] = "t1";
  }
  if (digitalRead(pumpRunning) == LOW) {
    Serial.println(F("pumpRunning LED OFF"));
    contIndValue[4] = "p0";
  }
  if (digitalRead(pumpRunning) == HIGH) {
    Serial.println(F("pumpRunning LED ON"));
    contIndValue[4] = "p1";
  }
  if (digitalRead(servosRunningLED) == LOW) {
    Serial.println("servosRunning LED OFF");
    contIndValue[6] = "s0";
  }
  if (digitalRead(servosRunningLED) == HIGH) {
    Serial.println(F("servosRunning LED ON"));
    contIndValue[6] = "s1";
  }
  if (digitalRead(tankEmpty) == HIGH) {
    Serial.println(F("tankEmpty LED ON"));
    contIndValue[1] = "e1";
  }
  if (digitalRead(tankEmpty) == LOW) {
    Serial.println(F("tankEmpty LED OFF"));
    contIndValue[1] = "e0";
  }
  if (digitalRead(tankFull) == HIGH) {
    Serial.println(F("tankFull LED ON"));
    contIndValue[2] = "f1";
  }
  if (digitalRead(tankFull) == LOW) {
    Serial.println(F("tankFull LED OFF"));
    contIndValue[2] = "f0";
  }
  if (digitalRead(tankLevelOK) == HIGH) {
    Serial.println(F("tankLevelOK LED ON"));
    contIndValue[3] = "l1";
  }
  if (digitalRead(tankLevelOK) == LOW) {
    Serial.println(F("tankLevelOK LED OFF"));
    contIndValue[3] = "l0";
  }
  if (digitalRead(vacActiveLED) == LOW) {
    Serial.println(F("28vac LED OFF"));
    contIndValue[8] = "v0";
  }
  if (digitalRead(vacActiveLED) == HIGH) {
    Serial.println(F("28vac LED ON"));
    contIndValue[8] = "v1";
  }
  if (digitalRead(radioLinkOnLED) == HIGH) {
    Serial.println(F("Radio Link OK LED ON"));
    contIndValue[5] = "r1";
  }
  if (digitalRead(radioLinkOnLED) == LOW) {
    Serial.println(F("Radio Link OK LED OFF"));
    contIndValue[5] = "r0";
  }
}
//-------------------------------------------------------------//
void modeControl() {

  if (digitalRead(modeAuto) == LOW && digitalRead(modeManual) == HIGH) {
    autoMode();
    modeInt += 1;
    modeControlMessageDisplayCheck();
  }
  else if (digitalRead(modeManual) == LOW && digitalRead(modeAuto) == HIGH) {
    manualMode();
    modeInt += 1;
    modeControlMessageDisplayCheck();
  }
  else if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    bypassMode();
    modeInt += 1;
    modeControlMessageDisplayCheck();
  }
}
//-------------------------------------------------------------//
void modeControlMessageDisplayCheck() {
  if (modeInt == 1) {
    modeControlMessage();
  }
  modeInt == 0;
}
//-------------------------------------------------------------//
void modeControlMessage() {

  if (digitalRead(modeAuto) == LOW) {
    Serial.println(F("System is in AUTO mode."));
  }

  if (digitalRead(modeManual) == LOW) {
    Serial.println(F("System is in MANUAL mode."));
  }

  if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    Serial.println(F("System is in BYPASS mode."));
  }
}
//-------------------------------------------------------------//
void autoMode() {

  if (digitalRead(modeAuto) == LOW) {
    digitalWrite(modeManualLED, LOW);
    digitalWrite(modeAutoLED, HIGH);
    digitalWrite(modeBypassLED, LOW);
    if ((manualModeCount == 1) && (digitalRead(modeManual) == HIGH)) {
      digitalWrite(pumpRunning, LOW);
      digitalWrite(tankFilling, LOW);
      manualModeCount = 0;
    }
  }
  if (digitalRead(modeAuto) == LOW && digitalRead(modeManual) == LOW) {
    Serial.println(F("Auto Mode - Switch Fault Needs Investigation."));
    return;
  }
}
//-------------------------------------------------------------//
void manualMode() {

  if (digitalRead(modeManual) == LOW && (rxActualLevel > rxFullSetPoint)) {
    digitalWrite(pumpRunning, HIGH);
    digitalWrite(tankFilling, HIGH);
    digitalWrite(modeManualLED, HIGH);
    digitalWrite(modeAutoLED, LOW);
    digitalWrite(modeBypassLED, LOW);
    manualModeCount = 1;
  }
  if (digitalRead(modeManual) == LOW && (rxActualLevel <= (rxFullSetPoint + fullOffset))) {
    digitalWrite(pumpRunning, LOW);
    digitalWrite(tankFilling, LOW);
    digitalWrite(modeManualLED, HIGH);
    digitalWrite(modeAutoLED, LOW);
    digitalWrite(modeBypassLED, LOW);
    manualModeCount = 1;
  }

  if (digitalRead(modeAuto) == LOW && digitalRead(modeManual) == LOW) {
    Serial.println(F("Manual Mode - Switch Fault Needs Investigation."));
    return;
  }
}
//-------------------------------------------------------------//
boolean buzzerOperation() {
  tone(piezoBuzzerPin, 1000, 500);
  delay(1000);
  return true;
}
//-------------------------------------------------------------//
void buzzerActive() {

  while (digitalRead(modeManual) == LOW && (rxActualLevel <= (rxFullSetPoint + fullOffset))) {
    if (buzzerOperation() == true) {
      contIndValue[13] = "b1";
    }
  }
  contIndValue[13] = "b0";
  // digitalWrite(alarmLED, LOW);
}
//-------------------------------------------------------------//
void bypassMode() {

  if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    digitalWrite(modeManualLED, LOW);
    digitalWrite(modeAutoLED, LOW);
    digitalWrite(modeBypassLED, HIGH);
    digitalWrite(pumpRunning, LOW);
    digitalWrite(tankFilling, LOW);
  }
  if (digitalRead(modeAuto) == LOW && digitalRead(modeManual) == LOW) {
    Serial.println(F("Bypass Mode Switch Fault Needs Investigation."));
    return;
  }
}
//-------------------------------------------------------------//
uint16_t angleToPulse(uint16_t ang) {
  const uint16_t angMax = 180;
  const uint16_t ang0 = 0;

  uint16_t pulse = map(ang, ang0, angMax, SERVOMIN, SERVOMAX); // map angle of 0 to 180 to Servo min and Servo max
  //Serial.print("AngleServo: "); Serial.print(ang);
  //Serial.print(" PulseServo: "); Serial.println(pulse);
  return pulse;
}
//-------------------------------------------------------------//
boolean servoChangeToTank() {
  const uint16_t ang90 = 90;
  const uint16_t ang0 = 0;
  long servo1PreviousMillis = 0;
  long servo2PreviousMillis = 0;
  long interval = 15000;

  if (digitalRead(pumpRunning) == LOW && boolean(tankConnected) == false) {
    digitalWrite(servosRunningLED, HIGH);
    contIndValue[6] = "s1"; //servosRunning
    unsigned long servo1CurrentMillis = millis();
    for (uint16_t angle = ang0; angle < ang90 + 1; angle += servoStepCount) {

      pwm.setPWM(0, 0, angleToPulse(ang90));
      delay(15);
    }
    if (servo1CurrentMillis - servo1PreviousMillis > interval) {
      servo1PreviousMillis = servo1CurrentMillis;
      servo1Failed = true;
      contIndValue[11] = "y0"; //Servo1 Pos (default)
    }
    else {
      Serial.println(F("Valve 1 set to Horizontal - Change to tank."));
      Serial.println();
      contIndValue[11] = "y1"; //Servo1 Pos (switched)
      servo1Failed = false;  //servo1 operation worked
    }
    unsigned long servo2CurrentMillis = millis();
    for (uint16_t angle = ang0; angle < ang90 + 1; angle += servoStepCount) {
      pwm.setPWM(1, 0, angleToPulse(ang0));
      delay(15);
    }
    if (servo2CurrentMillis - servo2PreviousMillis > interval) {
      servo2PreviousMillis = servo2CurrentMillis;
      servo2Failed = true;
      contIndValue[12] = "z0"; //Servo2 Pos (default)
    }
    else {
      Serial.println(F("Valve 2 set to Vertical - Change to tank."));
      contIndValue[12] = "z1"; //Servo2 Pos (switched)
      servo2Failed = false;  //servo2 operation worked
      digitalWrite(servosRunningLED, LOW);
      contIndValue[6] = "s0"; //servos not running
    }
    if (servo1Failed == true || servo2Failed == true) {
      contIndValue[6] = "s0"; //servos not running
      return tankConnected = false;
      Serial.print("Servos failed to operate Correctly!");
    }
    else if (servo1Failed == false && servo2Failed == false)
      contIndValue[6] = "s0";  //servos not running
    digitalWrite(servosRunningLED, LOW);
    Serial.print("Servos operation completed Successfully!");
    return tankConnected = true;
  }
  //digitalWrite(servosRunningLED, LOW);
}
//-------------------------------------------------------------//
boolean servoChangeFromTank() {

  const uint16_t ang90 = 90;
  const uint16_t ang0 = 0;
  long servo1PreviousMillis = 0;
  long servo2PreviousMillis = 0;
  long interval = 15000;

  if (digitalRead(pumpRunning) == LOW && boolean(tankConnected) == true) {
    digitalWrite(servosRunningLED, HIGH);
    contIndValue[6] = "s1"; //ServosRunning
    unsigned long servo1CurrentMillis = millis();
    for (uint16_t angle = ang0; angle < ang90 + 1; angle += servoStepCount) {
      pwm.setPWM(0, 0, angleToPulse(ang0));
      delay(15);
    }
    if (servo1CurrentMillis - servo1PreviousMillis > interval) {
      servo1PreviousMillis = servo1CurrentMillis;
      servo1Failed = true; //servo failed to rotate in given time
      contIndValue[11] = "y1"; //Servo1 Pos (switched)
    }
    else {
      Serial.println(F("Valve 1 set back to Vertical - Change from tank."));
      contIndValue[11] = "y0"; //Servo1 Pos (default)
      servo1Failed = false; //servo1 operation worked
    }
    unsigned long servo2CurrentMillis = millis();
    for (uint16_t angle = ang0; angle < ang90 + 1; angle += servoStepCount) {
      pwm.setPWM(1, 0, angleToPulse(ang90));
      delay(15);
    }
    if (servo2CurrentMillis - servo2PreviousMillis > interval) {
      servo2PreviousMillis = servo2CurrentMillis;
      servo2Failed = true;//servo failed to rotate in given time
      contIndValue[12] = "z1"; //Servo2 Pos (switched)
    }
    else {
      Serial.println(F("Valve 2 set back to Horizontal - Change from tank."));
      contIndValue[12] = "z0"; //Servo2 Pos (default)
      servo2Failed = false;  //servo2 operation worked
      digitalWrite(servosRunningLED, LOW);
      contIndValue[6] = "s0"; //servos not running
    }
    if (servo1Failed == true || servo2Failed == true) {
      contIndValue[6] = "s0"; //servos not running
      return tankConnected = true;
      Serial.print("Servos failed to operate Correctly!");
    }
    else if (servo1Failed == false && servo2Failed == false)
      contIndValue[6] = "s0"; //servos not running
    digitalWrite(servosRunningLED, LOW);
    Serial.print("Servos operation completed Successfully!");
    return tankConnected = false;
  }
  //digitalWrite(servosRunningLED, LOW);
}
//-------------------------------------------------------------//
void servoControl() {

  if (digitalRead(pumpRunning) == HIGH && boolean(tankConnected) == false) {
    digitalWrite(pumpRunning, LOW);
    digitalWrite(tankFilling, LOW);
    servoChangeToTank();
    digitalWrite(pumpRunning, HIGH);
    digitalWrite(tankFilling, HIGH);
    servoModeControlMessage();
  }

  else if (digitalRead(pumpRunning) == HIGH && boolean(tankConnected) == true) {
    servoModeControlMessage();
  }

  else if (digitalRead(pumpRunning) == LOW && boolean (tankConnected) == false) {
    servoModeControlMessage();
  }

  else if (digitalRead(pumpRunning) == LOW && boolean(tankConnected) == true) {
    // digitalWrite(tankFilling, LOW);
    servoChangeFromTank();
    servoModeControlMessage();
  }

  else if (digitalRead(modeAuto) == HIGH && (digitalRead(modeAuto) == HIGH)) { //bypass mode
    digitalWrite(pumpRunning, LOW);
    digitalWrite(tankFilling, LOW);
    servoChangeFromTank();
    servoModeControlMessage();
  }
  else if (digitalRead(modeManual) == LOW && (rxActualLevel <= (rxFullSetPoint + fullOffset))) {
    digitalWrite(pumpRunning, LOW);
    digitalWrite(tankFilling, LOW);
    servoChangeFromTank();
    servoModeControlMessage();
  }
  else if (digitalRead(modeAuto) == LOW && (rxActualLevel <= (rxFullSetPoint + fullOffset))) {
    digitalWrite(pumpRunning, LOW);
    digitalWrite(tankFilling, LOW);
    servoChangeFromTank();
    servoModeControlMessage();
  }
}
//-------------------------------------------------------------//
void servoModeControlMessage() {

  if (digitalRead(modeAuto) == LOW) {

    if (digitalRead(pumpRunning) == HIGH && boolean(tankConnected) == true) {
      Serial.println();
      Serial.println(F("TCPRA - Tank Connected & Pump Running.........in 'Auto' mode"));
      Serial.println();
      contIndValue[15] = "o1";
    }
    else if (digitalRead(pumpRunning) == LOW && boolean (tankConnected) == false) {
      Serial.println();
      Serial.println(F("TDPSA - Tank Disconnected & Pump Stopped......in 'Auto' mode"));
      Serial.println();
      contIndValue[15] = "o0";
    }

  }

  if (digitalRead(modeManual) == LOW) {
    if (rxActualLevel > rxFullSetPoint) {
      if (digitalRead(pumpRunning) == HIGH && boolean(tankConnected) == true) {
        Serial.println();
        Serial.println(F("TCPRM - Tank Connected & Pump Running.........in 'Manual' mode"));
        Serial.println();
        contIndValue[15] = "o3";
      }
      else if (digitalRead(pumpRunning) == LOW && boolean (tankConnected) == false) {
        Serial.println();
        Serial.println(F("TDPSM - Tank Disconnected & Pump Stopped......in 'Manual' mode"));
        Serial.println();
        contIndValue[15] = "o2";
      }
    }
    if (rxActualLevel <= (rxFullSetPoint + fullOffset)) {
      Serial.println();
      Serial.println(F("Tank is full, please switch back to 'Auto' or 'Bypass' mode!"));
      Serial.println();
      contIndValue[15] = "o2";
    }
  }
  if (digitalRead(modeAuto) == HIGH && digitalRead(modeManual) == HIGH) {
    Serial.println();
    Serial.println(F("TDPSB - Tank Disconnected & Pump Stopped......in 'Bypass' mode"));
    Serial.println();
    contIndValue[15] = "o4";
  }
}
//-------------------------------------------------------------//
bool rtcSetup() {

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  else {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //only required once then run script again with rtc.adjust commented out
    // or
    //rtc.adjust(DateTime(2021, 1, 06, 20, 14, 0));
    //or
    // rtc.adjust(DateTime(rtc.now()));
    Serial.println(F("RTC is running!"));
    return true;
  }
  if (!rtc.isrunning()) {
    Serial.println(F("RTC is NOT running!"));
    return false;
  }
}
//-------------------------------------------------------------//
void ledTestCheck() {

  uint16_t saveStateServosRunningLed = 0;
  uint16_t saveStateTankFull = 0;
  uint16_t saveStateTankFilling = 0;
  uint16_t saveStateTankLevelOK = 0;
  uint16_t saveStateTankEmpty = 0;
  uint16_t saveStatePumpRunning = 0;
  uint16_t saveStateVACActiveLED = 0;
  uint16_t saveStateModeAutoLED = 0;
  uint16_t saveStateModeManualLED = 0;
  uint16_t saveStateModeBypassLED = 0;
  uint16_t saveStateRadioLinkOnLED = 0;
  uint16_t saveStateAlarmLED = 0;

  buttonLEDTest = digitalRead(ledTest);

  saveStateServosRunningLed = digitalRead(servosRunningLED);
  saveStateTankFull = digitalRead(tankFull);
  saveStateTankFilling = digitalRead(tankFilling);
  saveStateTankLevelOK = digitalRead(tankLevelOK);
  saveStateTankEmpty = digitalRead(tankEmpty);
  saveStatePumpRunning = digitalRead(pumpRunning);
  saveStateVACActiveLED = digitalRead(vacActiveLED);
  saveStateModeAutoLED = digitalRead(modeAutoLED);
  saveStateModeManualLED = digitalRead(modeManualLED);
  saveStateModeBypassLED = digitalRead(modeBypassLED);
  saveStateRadioLinkOnLED = digitalRead(radioLinkOnLED);
  saveStateAlarmLED = digitalRead(alarmLED);
  //  Serial.print("buttonLEDTest: ") && Serial.println(buttonLEDTest);

  if (buttonLEDTest == LOW or autoLEDTest == true) {
    Serial.println();
    Serial.println(F("LED Test Activated..."));
    Serial.println();
    digitalWrite(servosRunningLED, HIGH);
    digitalWrite(tankFull, HIGH);
    digitalWrite(tankFilling, HIGH);
    digitalWrite(tankLevelOK, HIGH);
    digitalWrite(tankEmpty, HIGH);
    digitalWrite(pumpRunning, HIGH);
    digitalWrite(vacActiveLED, HIGH);
    digitalWrite(modeAutoLED, HIGH);
    digitalWrite(modeManualLED, HIGH);
    digitalWrite(modeBypassLED, HIGH);
    digitalWrite(radioLinkOnLED, HIGH);
    digitalWrite(alarmLED, HIGH);
    drawLEDOperation();
    monStatus();
    delay(5000);
    Serial.println();
    Serial.println(F(".....LED Test Completed."));
    Serial.println();
  }
  digitalWrite(servosRunningLED, saveStateServosRunningLed);
  digitalWrite(tankFull, saveStateTankFull);
  digitalWrite(tankFilling, saveStateTankFilling);
  digitalWrite(tankLevelOK, saveStateTankLevelOK);
  digitalWrite(tankEmpty, saveStateTankEmpty);
  digitalWrite(pumpRunning, saveStatePumpRunning);
  digitalWrite(vacActiveLED, saveStateVACActiveLED);
  digitalWrite(modeAutoLED, saveStateModeAutoLED);
  digitalWrite(modeManualLED, saveStateModeManualLED);
  digitalWrite(modeBypassLED, saveStateModeBypassLED);
  digitalWrite(radioLinkOnLED, saveStateRadioLinkOnLED);
  digitalWrite(alarmLED, saveStateAlarmLED);
  autoLEDTest = false;
}
//-------------------------------------------------------------//
