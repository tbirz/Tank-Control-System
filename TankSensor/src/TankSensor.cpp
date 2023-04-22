
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DallasTemperature.h>

//#include <OneWire.h>



#define trigger 2 //Nano D2 pin 5 
#define echo 3    //Nano D3 pin 6 
#define rxPin 5   //Nano D5, pin 8
#define txPin 6   //Nano D6, pin 9
#define sensorCodeVersion "7.2.6"


/********************************************************************/
// Data wire is plugged into pin 7 on the Arduino
#define ONE_WIRE_BUS 7
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
/********************************************************************/
//LiquidCrystal_I2C lcdI2C(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //

const byte HC12SetPin = 4;  // "SET" Pin on HC12, High for transparent, Low for command

float initialDistance = 0.0;
const float tankHeight = 230.0;
const float refillSetPoint = 207.0; //rxTankHeight - (rxTankHeight * 0.1) //10% of tank height, 230-23=207
const float fullSetPoint = 20.0;

float actualLevel = 0.0;

float sum = 0.0;
const uint8_t  averageCount = 10;
float averagedDistance = 0.0;
float tempReading = 0.0;
uint8_t i = 0;
uint8_t sensorError = 0;
const uint8_t sensorErrorCount = 10;



const float speedConst = 0.034 / 2;
boolean txInProgress = false;
boolean I2CLCDFound = false;


const float R1 = 100000.0;  //R1 100K
const float R2 = 10000.0;   //R2 10K
const float vRef = 3.19;



SoftwareSerial HC12(rxPin, txPin); //HC-12 Serial Connection


SoftwareSerial testing(8, 9); //rx,tx
long baud = 9600;

void(* resetFunc) (void) = 0; //declare reset function @ address 0


void setup()
{
  Serial.begin(baud);  //USB Serial start
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //  scanI2C();

  //define LCD to 16 columns and 2 rows
  // lcdI2C.begin(16, 2);
  //  lcdI2C.backlight();//Power on the back light
  ////lcdI2C.backlight(); Power off the back light

  // lcdI2C.setCursor(0, 0);
  //lcdI2C.print("Initialising.........");



  // define pin modes for tx, rxn HC-12:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  pinMode(HC12SetPin, OUTPUT);                  //Designated ATSet pin (ATMode pin)
  digitalWrite(HC12SetPin, HIGH);               // HIGH=Transparent mode, LOW=AT configuration mode


  Serial.println(F("---------------------------------------------------"));
  Serial.print("TankWaterLevelSensorAverage Version ") && Serial.println(sensorCodeVersion);
  Serial.println(F("---------------------------------------------------"));
  Serial.println();

  Serial.println(F("Serial monitor available... OK"));

  Serial.print("Serial link.......... ");


  HC12.begin(baud); // HC-12 Serial
  if (HC12.isListening()) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("Serial Link Failed"));
  }



  //test HC-12
  Serial.print("HC-12 available... ");
  HC12.write("AT+DEFAULT");
  while (HC12.available() > 0) {
    Serial.write(HC12.read());
  }
  Serial.println("OK");
  Serial.println();
  Serial.println(F("HC-12 Serial Comms Setup Completed."));
  Serial.println();

  // Start up the Temperature Sensor library
  sensors.begin();

  pinMode(trigger, OUTPUT); //HC-SR04
  pinMode(echo, INPUT);    //HC-SR04

  //initial default position
  Serial.println("Initialising Sensors........");

  showData();   //reads current data
  Serial.println();
  Serial.print("Tank Height (cm): ") && Serial.println(tankHeight);
  Serial.print("Tank Full Set Point (cm): ") && Serial.println(fullSetPoint);
  Serial.print("Tank Refill Set Point (cm): ") && Serial.println(refillSetPoint);
  Serial.print("Averaging Count: ") && Serial.println(averageCount);
  Serial.println();
  //Initial Tank level sensor reading being taken
  initialDistance = calculateDistance();
  Serial.print("Initial Reading Tank Full: ") && Serial.print(calculatePercentage(initialDistance)) && Serial.println("%");
  Serial.print("Initial Volume reading: ") && Serial.print(calculateAvailableVolume())&& Serial.println(" Litres");
  Serial.println();
  Serial.println(F(".........Sensors initialised."));
  Serial.println();
  //lcdI2C.clear();
}

void loop() {

  if (i == 0) {
    Serial.println("Sampling in progress.....");
    Serial.println();
  }

  calculateDistance();
  averagedDistance = (float(sum / averageCount));
  //  writeToLCDI2C(averagedDistance);
  txData();

}
//--------------------FUNCTIONS--------------------------------//
/*void writeToLCDI2C(float averagedDistance) {

  if ( I2CLCDFound = true) {
  lcdI2C.setCursor(0, 0); //we start writing from the first row first column //16 characters per line

  lcdI2C.print("L:") && lcdI2C.print(averagedDistance);


  lcdI2C.setCursor(9, 0);

  if (calculatePercentage(averagedDistance) > 99.99 ) {
    lcdI2C.print(calculatePercentage(averagedDistance))&& lcdI2C.print("%");
  }
  else if (calculatePercentage(averagedDistance) > 9.99 <= 99.99 ) {
    lcdI2C.print(" ")&&  lcdI2C.print(calculatePercentage(averagedDistance))&& lcdI2C.print("%");
  }
  else if (calculatePercentage(averagedDistance) <= 9.99 ) {
    lcdI2C.print("  0")&& lcdI2C.print(calculatePercentage(averagedDistance))&& lcdI2C.print("%");
  }

  lcdI2C.setCursor(0, 1);
  lcdI2C.print(senseTemperature()) && lcdI2C.print(char(223)) && lcdI2C.print("C") && lcdI2C.print(" ") && lcdI2C.print(readSupplyVoltage())&& lcdI2C.print("V");
  }
  }
*/
//-----------------------------------------------------------//
void txData() {
  transmitActuals();
  transmitStatic();
}
//-----------------------------------------------------------//
void  transmitActuals() {
  char startByte = '<';
  char stopByte = '>';

  i++;

  if (i >= averageCount)
  {
    showData();
    Serial.println();
    Serial.println(F("Sampling now completed and averaged."));
    Serial.println();
    Serial.println(F("'Actuals' Data transmitting....."));
    Serial.println();
    if (txInProgress == false) {
      txInProgress = true;
      HC12.print(startByte);
      Serial.print(startByte);
      HC12.print("D") && HC12.print(averagedDistance)&& HC12.print(","); //ActualLevel (from Top)
      Serial.print("Averaged Distance to Water: ") &&Serial.print("D") && Serial.print(averagedDistance)&& Serial.println(",");
      HC12.print("P") && HC12.print(calculatePercentage(averagedDistance))&& HC12.print(",");
      Serial.print("Percentage Full: ") && Serial.print("P") && Serial.print(calculatePercentage(averagedDistance))&& Serial.println(",");
      HC12.print("A") && HC12.print(calculateAvailableVolume())&& HC12.print(",");
      Serial.print("AvailableVolume: ") && Serial.print("C") && Serial.print(calculateAvailableVolume())&& Serial.println(",");
      HC12.print("T") && HC12.print(senseTemperature())&& HC12.print(",");
      Serial.print("Current Temperature ºC: ") && Serial.print("T") && Serial.print(senseTemperature())&& Serial.println(",");
      HC12.print("B") && HC12.print(readSupplyVoltage())&& HC12.print(",");
      Serial.print("Battery Voltage: ")  && Serial.print("B") && Serial.print(readSupplyVoltage())&& Serial.println(",");
      HC12.print("V") && HC12.print(readSensorControl5v())&& HC12.print(","); 
      Serial.print("SensorControl5v: ")  && Serial.print("V") && Serial.print(readSensorControl5v())&& Serial.println(",");
      HC12.print(stopByte);
      Serial.println(stopByte);
    }
    if (txInProgress == true) {
      Serial.println();
      Serial.println(F(".....'Actuals' Data transmitted OK"));
      Serial.println();
      txInProgress = false;
      while (HC12.available() > 0 ) {
        char junk = HC12.read();
        // Serial.write(HC12.read());
      }
    }
    else {
      Serial.println(F("..............'Actuals' Data transmit Failed"));
      txInProgress = false;
    }
  }
}
//-----------------------------------------------------------//
void  transmitStatic() {
  const char startByte = '<';
  const char stopByte = '>';

  if (i >= averageCount)
  {
    Serial.println(F("'Static' Data transmitting....."));
    Serial.println();
    if (txInProgress == false) {
      txInProgress = true;
      //delay(1000);
      
      HC12.print(startByte);
      Serial.print(startByte);
      HC12.print("R") && HC12.print(refillSetPoint) && HC12.print(",");
      Serial.print("Refill Set Point: ") && Serial.print("R") && Serial.print(refillSetPoint)&& Serial.println(",");
      HC12.print("F") && HC12.print(fullSetPoint)&& HC12.print(",");
      Serial.print("Full Set Point: ") && Serial.print("F") && Serial.print(fullSetPoint)&& Serial.println(",");
      HC12.print("H") && HC12.print(tankHeight)&& HC12.print(",");
      Serial.print("Tank Height: ") && Serial.print("H") && Serial.print(tankHeight)&& Serial.println(",");
      HC12.print("C") && HC12.print(calcTotalCapacity())&& HC12.print(",");
      Serial.print("Total Capacity: ") && Serial.print("C") && Serial.print(calcTotalCapacity())&& Serial.println(",");
      Serial.println(stopByte);
      HC12.print(stopByte);
    }
    if (txInProgress == true) {
      Serial.println();
      Serial.println(".....'Static' Data transmitted OK");
      Serial.println();
      txInProgress = false;
      while (HC12.available() > 0 ) {
        char junk = HC12.read();
        // Serial.write(HC12.read());
      }
    }
    else {
      Serial.println(F("..............'Static' Data transmit Failed"));
      txInProgress = false;
    }
    i = 0;
    sum = 0;
    tempReading = 0;
    averagedDistance = 0;
  }
}
//-----------------------------------------------------------//
float readSupplyVoltage() {
  int sampleCountTotal = 10;
  float sumReading = 0.0;
  int sampleCount = 0;
const float dividerRatio12v = 14.33;
  while (sampleCount < sampleCountTotal) {
    sumReading += analogRead(A0);
    sampleCount++;
    delay(10);
  }
  float sensorVoltage = ((float)sumReading / (float) sampleCountTotal * vRef) / 1024.0;
  float batteryVoltageIn = sensorVoltage * dividerRatio12v;

  return batteryVoltageIn;
}
//------------------------------------------------------------//
float readSensorControl5v() {
  int sampleCountTotal = 10;
  float sumReading = 0.0;
  int sampleCount = 0;
const float dividerRatio5v = 14.33;

  while (sampleCount < sampleCountTotal) {
    sumReading += analogRead(A1);
    sampleCount++;
    delay(10);
  }
  float sensorVoltage = ((float)sumReading / (float) sampleCountTotal * vRef) / 1024.0;
  float controlVoltageIn = sensorVoltage * dividerRatio5v;

  return controlVoltageIn;
}
//------------------------------------------------------------//

void showData() {

  if (averagedDistance == 0 && i == 0) {
    Serial.print("Averaged Distance: ") && Serial.print(averagedDistance) && Serial.print(" (cm)") && Serial.println(" (Distance yet to be averaged)");
 //   Serial.print("Available Volume: ") && Serial.print(calculateAvailableVolume())&& Serial.print(" (Litres)")&& Serial.println(" (Volume yet to be measured)");
 //   Serial.print("Percentage Full: ") && Serial.print(calculatePercentage(averagedDistance))&& Serial.print(" (%)")&& Serial.println(" (Percentage yet to be measured)");
  }
  else if (averagedDistance <= fullSetPoint && i > 0 || averagedDistance> tankHeight) {
      Serial.println(F("Sensor Error, ATTENTION REQUIRED ASAP"));
    }
  else {
    Serial.println();
    Serial.print("Averaged Distance: ") && Serial.print(averagedDistance) && Serial.println(" (cm)");
    Serial.print("Tank Full (%): ") && Serial.print(calculatePercentage(averagedDistance))&& Serial.println("%");
    Serial.print("Available Volume: ") && Serial.print(calculateAvailableVolume())&& Serial.println(" Litres");
  }
 Serial.print("Current Temperature: ") &&  Serial.print(senseTemperature())&& Serial.println("ºC"); 
  if (senseTemperature() <= 0 || senseTemperature() >= 85) {   
    Serial.println(F("Temperature Sensor Error, ATTENTION REQUIRED ASAP"));
    Serial.println();
  }

  Serial.print("Battery Voltage: ") && Serial.print(readSupplyVoltage()) && Serial.println(" Volts");
  if (readSupplyVoltage() <= 10.0) {
    Serial.println(F("Battery Reading is LOW, ATTENTION REQUIRED ASAP"));
  }
}
//-----------------------------------------------------------//
float calculateDistance() {
  float duration = 0.0, distance = 0.0;

  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * speedConst;
  // Serial.print("i: ") && Serial.println(i);

  Serial.print("Distance to Water: ") && Serial.print(distance) && Serial.println(" cm");

  delay(250); //adjusts the data read speed

  if (distance > tankHeight or distance < fullSetPoint) {
    i -= 1;
    Serial.print("Sensor Reading Error, Outside Tolerance - Value: ") && Serial.print(distance) && Serial.println(" not used.");
    sensorError += 1;
    if (sensorError > sensorErrorCount) {
      Serial.println(F("--------------Erroneous Water Sensor Level Readings."));
      Serial.println(F("--------------Error Count Exceeded."));
      Serial.println(F("--------------Investigation Required."));
    }
    if (sensorError > sensorErrorCount + 10) {
      Serial.println(F("Automatic Reset about to occur."));
      Serial.println(F("Resetting......"));
      delay(1000);
      resetFunc();
    }
  }
  else {
    sensorError = 0;
    sum = sum + distance;
    return sum;
  }
  return sum;
}
//-----------------------------------------------------------//
float calcTotalCapacity() {
  const uint16_t numTanks = 2;
  const float tankArea = 962.23; //Area=r*r*pi/100 in cm3
  // total volume = tankArea * height * numTanks /10

  float totalCapacity = ((tankArea * (tankHeight - fullSetPoint)) * numTanks) / 10;

  //Serial.print("Total Capacity: ") && Serial.println(totalCapacity);
  return totalCapacity;
}
//-----------------------------------------------------------//
float outOfLimitsCheck(float averagedDistance) {

  if (averagedDistance < fullSetPoint or averagedDistance > tankHeight) {
    averagedDistance = tankHeight;
    return averagedDistance;
  }
  else if (averagedDistance >= fullSetPoint or averagedDistance <= tankHeight) {
    return averagedDistance;;
  }
  return averagedDistance;
}
//------------------------------------------------------------//
float calculatePercentage(float averagedDistance) {

  averagedDistance = outOfLimitsCheck(averagedDistance);
  float percentageLevel = map(averagedDistance, tankHeight, fullSetPoint, 0, 100);

  return percentageLevel;
}
//---------------------------------------------------------------------------------
float calculateAvailableVolume() {
  if (i == 0) {
    float tankCurrentVolume = (calcTotalCapacity() * calculatePercentage(initialDistance)) / 100;
    return tankCurrentVolume;
  }
  else {
    float tankCurrentVolume = (calcTotalCapacity() * calculatePercentage(averagedDistance)) / 100;
    return tankCurrentVolume;
  }
}
//-----------------------------------------------------------//

float senseTemperature() {

  sensors.requestTemperatures(); // Send the command to get temperature readings
  tempReading = sensors.getTempCByIndex(0); //0 index for 1 temp Sensor
  delay(1000);
  return tempReading;
}
//------------------------------------------------------------//
