
#include <Arduino.h>

/*******Controller**************/

#include <SoftwareSerial.h>

/*******LCD**************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>


/******Vars**************/

//CONTROLLER
#define ControllerCodeVersion "9.0.11"
#define screenType "3.5 inch MCUFriend TFT Touch Screen"
#define remoteSensorCodeVersion "7.2.2"
#define remoteDisplayVersion "1.0.0"

#define tankLevelOK 24 //Green LED
#define tankFull 28 // Green LED
#define tankFilling 26 //Blue LED
#define tankEmpty 22  //Red LED
#define pumpRunning 30  //Blue LED

#define radioSETPin 23  //Mega pin D23
#define rxRadioPin 68  //Mega pin 68 (A14)
#define txRadioPin 69  //Mega pin 69 (A15)

SoftwareSerial HC12(rxRadioPin, txRadioPin);
long baud = 9600;
float rxActualLevel = 0.0; // 0%=230, 10%=207, 100% = 20 //in cm
float rxActualLevelOld = 0.0;
float rxTemperatureLevel = 0.0;
float rxTemperatureLevelOld = 0.0;
float rxPercentageLevel = 0.0;
float rxBatteryVoltageLevel = 0.0;
float const BatteryVoltageLevelMin = 11.5;
float tankHeight = 230.0;
float tankLow = tankHeight - (tankHeight * 0.1); //10% of tank height, 230-23=207
float tankHigh = 20.0;
float readErrorTolerance = 5;
String rxTempString = "";
uint16_t rxChar = 0;
uint16_t rxFileChar = 0;
uint16_t radioOperationCount = 0;
char tftDegreesSymbol = char(247);
boolean initCompleted = false;

//LCD

MCUFRIEND_kbv tft;
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

const uint16_t XP = 7, XM = A1, YP = A2, YM = 6 ; //480x320 ID=0x9488, ID=0x6814

const uint16_t Orientation = 1; //Landscape 90 deg

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

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

uint16_t currentPage = 2; //var for current page on screen

//variables for Page Description & DTG Co-ords
uint16_t pageTitleX = 0;
uint16_t pageTitleY = 0;

//Page Frame Co-ords
const uint16_t frameBottom = 308;
const uint16_t frameTop = 10;
const uint16_t frameLeft = 1;
const uint16_t frameRight = 479;
const uint16_t numTanks = 2;


const float tankLevelDifferential = tankHeight - tankHigh; //in cm
const float tankRadius = 175.0;  //in cm
const float tankArea = 962.0; //Area=r*r*pi in cm
float volume = 0.0; // volume = Area * height
float volumeLevel = 0.0;  //map volume of tank

//vars for interfacing tank level with screen
uint16_t ptrOffset = 0; //for positioning of % level value in tank graphic
uint16_t ptrOffsetEmpty = 0; //for positioning of % level value in tank graphic when empty
uint16_t ptrOffsetFull = 0; //for positioning of > 100% level value in tank graphic
uint16_t ptrOffsetPercentValueLabel = 0; //for positioning of > 100% level % value in tank graphic


//vars for drawing LED's on screen
//const uint16_t ledRadius = 7;
//const uint16_t ledSpacing = 20;
//const uint16_t ledLitRadius = 3; //var for size of color dots



// Tank Levels array for page on 'tank page
const uint16_t tankLevelArraySize = 6;
//const char* tankLevelLabels[tankLevelArraySize] = {">= 0 & < 10%", ">= 10 & < 50%", ">= 50 & < 75%", ">= 75 & < 95%", ">= 95%", "\0"};
const char* tankLevelLabels[tankLevelArraySize] = {">= 0%", ">= 10%", ">= 50%", ">= 75%", ">= 95%", "\0"};
/*****************************************************************************************************************************************/
//SETUP
/*****************************************************************************************************************************************/
void setup() {
  Serial.begin(baud);
  delay(50);
  Serial.println(F("========================================"));
  Serial.println(F("Remote Display SERIAL OUTPUT"));
  Serial.println(F("========================================"));

  /****tft*****/

  pinMode(A0, OUTPUT);       //.kbv mcufriend have RD on A0
  digitalWrite(A0, HIGH);

  // Setup the tft
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(Orientation);
  tft.fillScreen(Black);


  radioOperation();

  showSplash();

  /****Controller*****/


  do {
    // Serial.print(" rxActualLevel: ") && Serial.println(rxActualLevel);
  } while (radioRxVariables() == false);
}

/*****************************************************************************************************************************************/
//LOOP
/*****************************************************************************************************************************************/
void loop() {
  if (boolean(initCompleted) == false) {
    initCompleted = true;
  }

  radioRxVariables();
  if (rxActualLevelOld != rxActualLevel or rxTemperatureLevelOld!=rxTemperatureLevel) {
    showTank();
    drawTankLevel(rxActualLevel);
    getTankDetails();
    rxActualLevelOld = rxActualLevel;
    rxTemperatureLevelOld = rxTemperatureLevel;
  }
}
//----------------- TFT FUNCTIONS-----------------------------//

//---------------------------------------------------------------------------------
void showPageTitle() { //shows page titles bottom left corner of screen

  pageTitleX = 1;
  pageTitleY = 310;
  tft.setTextSize(1);
  tft.setCursor(pageTitleX, pageTitleY);
  tft.setTextColor(White);
  switch (currentPage) {

    case 0: {
        tft.print("Splash");
        serialShowCurrentPage(currentPage);
        break;
      }
  }
  //   case 1: {
  //      tft.print("Tank Status");
  //      serialShowCurrentPage(currentPage);
  //      break;
  //     }
}
//----------------------------------------------------------------------------
void serialShowCurrentPage(uint16_t currentPage) { //shows current page # on serial

  if (currentPage < 2) { //10 = splash page (don't show spash page number on serial)
    Serial.print("CurrentPage: ") && Serial.println(currentPage);
  }
}
//----------------------------------------------------------------------------------
void showSplash() {

  currentPage = 0;

  showPageTitle();
  showProjectTitle();
  showTFTDetails();
  showVersion();
  showAbout();

  tft.setCursor(200, 280);
  tft.setTextColor(Lime);
  tft.println("Initialising........");

}
//----------------------------------------------------------------------------------
void showVersion() {

  tft.setCursor(100, 160);
  tft.setTextColor(LightSkyBlue);
  tft.print("Remote Display Version: ")&& tft.println(remoteDisplayVersion);
  Serial.print("Remote Display Version: ")&& Serial.println(remoteDisplayVersion);
  Serial.println(F("---------------------------------------------------"));
}
//-------------------------------------------------------------------------------
void showAbout() {

  tft.setTextSize(1);
  tft.setTextColor(LightSkyBlue);
  tft.setCursor(100, 250);
  tft.println("System developed by Tony Birznieks");
}
//---------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void showProjectTitle() {

  tft.setTextSize(1);
  tft.setTextColor(LightSkyBlue);
  tft.setCursor(5, 1);
  tft.println("Water Tank Remote Display");
}
//---------------------------------------------------------------------------------
void showPageInformation() { //shows info on all pages

  showProjectTitle();
}
//---------------------------------------------------------------------------------
void showTank() {
  currentPage = 1;

  tft.fillScreen(Black);
  showPageInformation();
  buildPageFrame();
  showTankLevelDefinitions();
  drawTankLevel(rxActualLevel);
  getTankDetails();
}
//---------------------------------------------------------------------------------
uint16_t labelTankLevels(const char* arrayName[], uint16_t arraySize, uint16_t spacing, uint16_t xPosT, uint16_t yPosT) {

  for (uint16_t idx = 0; idx < arraySize - 1; idx++) {
    tft.setTextSize(2);
    tft.setTextColor(White);
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

  tft.drawLine(frameLeft, frameTop, frameLeft, frameBottom, ForestGreen);
  tft.drawLine(frameLeft, frameTop, frameRight, frameTop, ForestGreen);
  tft.drawLine(frameLeft, frameTop, frameRight, frameTop, ForestGreen);
  tft.drawLine(frameLeft, frameBottom, frameRight, frameBottom, ForestGreen);
  tft.drawLine(frameRight, frameTop, frameRight, frameBottom, ForestGreen);
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
float outOfLimitsCheck(float rxActualLevel) {

  if (rxActualLevel < tankHigh) {
    rxActualLevel = tankHigh;
  }

  else if (rxActualLevel > tankHeight) {
    rxActualLevel = tankHeight;
  }

  return rxActualLevel;
}
//---------------------------------------------------------------------------------
void drawTankLevel(float rxActualLevel) {

  if (currentPage == 1) { //tank page
    uint16_t xPosRectStart = 20;
    uint16_t yPosRectStart = 245;
    uint16_t rectWidth = 100;
    uint16_t rectHeight = -180;
    uint16_t txtSpace = 0;
    float tankLevelPercent = 0.0;
    float tankLevelPtr = 0.0;

    //Setup dividers and headings additional to frame for tank page
    tft.drawLine(140, frameTop, 140, frameBottom, ForestGreen);
    tft.drawLine(310, frameTop, 310, frameBottom, ForestGreen);
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

    rxActualLevel = outOfLimitsCheck(rxActualLevel);
    tankLevelPercent = mapTankLevels(rxActualLevel, tankHeight, tankHigh);

    tankLevelPtr = mapTankLevelPtr(tankLevelPercent, yPosRectStart, rectHeight);
    //Serial.println("Tank Sections co-ords: Page 1");
    tft.drawRect(xPosRectStart, yPosRectStart, rectWidth, rectHeight, White);  //tank
    updateTankLevel (tankLevelPercent, xPosRectStart, yPosRectStart, rectWidth, tankLevelPtr, txtSpace);
  }
}
//----------------------------------------------------------------------
void getTankDetails() {

  uint16_t tankCapacity = 0;

  if (currentPage == 1) { //tankPage
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
    tft.setCursor(315, 255);
    tft.print("Water Temperature ")&& tft.print("(") && tft.print(tftDegreesSymbol)&& tft.print("C):");


    tft.setTextColor(LightSteelBlue, Black);
    tft.setCursor(418 , 70);
    tft.print(tankHeight) && tft.print("cm");
    tft.setCursor(418, 85);
    tft.print(tankLow) && tft.print("cm");
    tft.setCursor(418, 100);
    tft.print(tankHigh) && tft.print("cm");

    tft.setCursor(418, 115);
    tft.print(rxActualLevel) && tft.print("cm");

    tft.setCursor(418, 145);
    tft.print(calcTotalCapacity()) && tft.print("L"); // Total capacity of all tanks
    tft.setTextSize(2);
    tft.setCursor(350, 190);
    tft.setTextColor(White, Black);

    tankCapacity = mapTankLevels(rxActualLevel, tankHeight, tankHigh);

    if (tankCapacity >= 0 && tankCapacity < 10) {
      //  tft.setTextColor(Red, Black);
      tft.print(calcTankVolume(rxActualLevel))&& tft.print("L");

    }
    else if (tankCapacity >= 10 && tankCapacity < 50) {
      // tft.setTextColor(Orange, Black);
      tft.print(calcTankVolume(rxActualLevel))&& tft.print("L");
    }
    else if (tankCapacity >= 50 && tankCapacity < 75) {
      //  tft.setTextColor(Yellow, Black);
      tft.print(calcTankVolume(rxActualLevel))&& tft.print("L");
    }
    else if  (tankCapacity >= 75 &&  tankCapacity < 95) {
      // tft.setTextColor(Green, Black);
      tft.print(calcTankVolume(rxActualLevel));
    }
    else if (tankCapacity > 95) {
      // tft.setTextColor(Blue, Black);
      tft.print(calcTankVolume(rxActualLevel))&& tft.print("L");
    }
    else {
      tft.print(calcTankVolume(rxActualLevel))&& tft.print("L");
    }

    //displays percentage value of water available
    tft.setCursor(350, 230);
    tft.setTextColor(White, Black);
    tft.print(mapTankLevels(rxActualLevel, tankHeight, tankHigh)) && tft.print("%");

    //displays water temperature
    showTemperature();
  }
}
//---------------------------------------------------------------------------------
float calcTotalCapacity() {
  float totalCapacity = 0;

  totalCapacity = ((tankArea * tankLevelDifferential) * numTanks) / 10;

  //Serial.print("Total Capacity: " ) && Serial.println(totalCapacity);
  return totalCapacity;
}
//---------------------------------------------------------------------------------
float calcTankVolume(float rxActualLevel) {
  //volumeLevel = 0;
  rxActualLevel = outOfLimitsCheck(rxActualLevel);
  volumeLevel = map(rxActualLevel, tankHeight, tankHigh, 0, tankLevelDifferential);
  //Serial.print("volumeLevel: " ) && Serial.println(volumeLevel);

  volume  = (tankArea * volumeLevel * numTanks) / 10; //max vol calculated to tankHigh
  // Serial.print("Volume: " ) && Serial.println(volume);

  return volume;
}
//---------------------------------------------------------------------------------
float mapTankLevels(float rxActualLevel, uint16_t tankHeight, uint16_t tankHigh) {
  rxActualLevel = outOfLimitsCheck(rxActualLevel);
  float tankLevelPercent = map(rxActualLevel, tankHeight, tankHigh, 0, 100);
  // Serial.print("rxActual: ") && Serial.print(rxActualLevel) && Serial.print("tank percentage: ") && Serial.println(tankLevelPercent);

  return tankLevelPercent;
}
//---------------------------------------------------------------------------------
float mapTankLevelPtr(float tankLevelPercent, uint16_t yPosRectStart, uint16_t rectHeight) {

  float tankLevelPtr = map(tankLevelPercent, 0, 100, yPosRectStart, yPosRectStart - rectHeight);

  // Serial.print("tank ptr: ") && Serial.println(tankLevelPtr);
  return tankLevelPtr;
}
//---------------------------------------------------------------------------------
void updateTankLevel (float tankPercent, uint16_t xPosRectStart, uint16_t yPosRectStart, uint16_t rectWidth, float ptr, uint16_t txtSpace) {

  if (currentPage == 1) { //tank page
    txtSpace = 32;
    ptrOffset = 248;
    ptrOffsetFull = 424;
    ptrOffsetEmpty = 10;
    ptrOffsetPercentValueLabel = 180;
  }


  if (tankPercent > 100) { //tank > 100% reading

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Blue);
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptrOffsetPercentValueLabel);
  }

  else {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank >95% & < 100% reading
  }

  if ((tankPercent >= 95) && (tankPercent <= 100)) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Blue);
    tft.setTextColor(White);
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank >95% & < 100% reading
  }

  else if (tankPercent >= 75 && tankPercent < 95) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Green);
    tft.setTextColor(White);
    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <95% reading
  }

  else if (tankPercent >= 50 && tankPercent < 75) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Yellow);
    tft.setTextColor(Black);

    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <75% reading
  }

  else if (tankPercent >= 10 && tankPercent < 50) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Orange);
    tft.setTextColor(White);

    tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <50% reading
  }

  else if (tankPercent < 10) {
    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

    tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptr, Red);
    tft.setTextColor(White);

    if (tankPercent < 6) {
      tft.fillRect(xPosRectStart + 1, yPosRectStart - 1, rectWidth - 2, yPosRectStart + 1 - ptrOffsetFull, Black);  //fill empty section of tank with 'black'

      tft.setTextColor(Red);
      tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptrOffsetEmpty); //tank <6% reading
    }

    else {
      tft.setCursor(xPosRectStart + txtSpace, yPosRectStart + 1 - ptr + ptrOffset); //tank <10% reading
    }
  }

  else {
    tft.setTextSize(1);
    tft.setTextColor(LightSalmon);
  }
  tft.print(tankPercent)&& tft.print("%");
}

//--------------------------------------------------------------------------------------
void showTFTDetails() {

  uint16_t tftID = tft.readID();
  Serial.println("---------------------------------------------------");
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
}
//-------------------------------------------------------------------------------
bool radioOperation() {

  if (radioOperationCount == 0) {
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Serial monitor available... OK");
    Serial.print("Serial link available... ");
    HC12.begin(baud);

    if (HC12.isListening()) {
      //digitalWrite(radioLinkOnLED, HIGH);
      Serial.println(F("OK"));
    }

    else {
      Serial.println(F("HC-12 Serial Not Listening"));

      return false;
    }
    //test HC-12
    Serial.println(F("HC-12 Initialization Completed."));

    Serial.print("HC-12 available... ");
    HC12.write("AT+DEFAULT");
    Serial.println("OK");


    Serial.println();
    Serial.println(F("Initial Readings being sought:......"));
    Serial.println();

    radioOperationCount = 1;
  }
  return true;
}
//-------------------------------------------------------------//
bool radioRxVariables() { //read data out of buffer

  if (radioOperation() == true) {

    while (HC12.available() > 0) {

      uint16_t rxChar = (char(HC12.read())); //read received data into string
      if (rxChar != '\n') {
        rxTempString += (char)rxChar;
        // Serial.print(" String: ") && Serial.println(rxTempString);
      }
      else {

        if (rxTempString.startsWith("L")) {
          rxTempString.remove(0, 1); //remove the letter L from string of numbers
          rxActualLevel = rxTempString.toFloat(); //convert to float
          Serial.print("Distance to water (cm): ") && Serial.println(rxActualLevel);
        }

        if (rxTempString.startsWith("T")) {      // rxActualLevel goes to "0"
          rxTempString.remove(0, 1); //remove the letter T from string of numbers
          rxTemperatureLevel = rxTempString.toFloat(); //convert to float
          Serial.print("Water Temperature (ºC): ") && Serial.println(rxTemperatureLevel);
        }

        if (rxTempString.startsWith("V")) {      // rxActualLevel goes to "0"
          rxTempString.remove(0, 1); //remove the letter V from string of numbers
          rxBatteryVoltageLevel = rxTempString.toFloat(); //convert to float
          // Serial.print("Test - rxBatteryVoltageLevel (V): ") && Serial.println(rxBatteryVoltageLevel );
        }

        rxTempString = "";
        while (HC12.available() > 0 ) {
          char junk = HC12.read();
        }
        float rxPercentageLevel = map(rxActualLevel, tankHeight, tankHigh, 0, 100);
      }
      if (rxActualLevel > 0) {
        return true;
      }
    }
  }
}
//-------------------------------------------------------------//
void showTemperature() {
  if (currentPage == 1) {
 
    if (rxTemperatureLevel == 0 ) {
      tft.setTextSize(2);
      tft.setCursor(340, 270);
      tft.setTextColor(Yellow, Black);
      tft.print(char(0)) && tft.print(rxTemperatureLevel) && tft.print(tftDegreesSymbol) && tft.print("C");
      tft.setTextSize(1);
      tft.setCursor(350, 290);
      tft.setTextColor(Yellow);
      tft.print("(measuring...)");
    }
    else if (rxTemperatureLevel > 0 ) {

      tft.setTextSize(2);
     tft.setTextColor(LightSkyBlue,Black);
      if (rxTemperatureLevel > 0 && rxTemperatureLevel < 10) {
      tft.setCursor(340, 270);
      tft.print(rxTemperatureLevel) && tft.print(tftDegreesSymbol)&& tft.print("C");
      }
      else if(rxTemperatureLevel > 10) {
      tft.setCursor(350, 270);
      tft.print(rxTemperatureLevel) && tft.print(tftDegreesSymbol)&& tft.print("C");
      }
      tft.setTextSize(1);
      tft.setCursor(350, 290);
      tft.setTextColor(Black);
      tft.print("(measuring...)");
    }
  }
}
