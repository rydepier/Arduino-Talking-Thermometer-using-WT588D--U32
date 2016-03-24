/*------------------------------------------------------------
Talking Thermometer with WT588D - U 32M audio module
Using audio file 'Talking Measurements'

This will display and talk either Centigrade or Farhenheit

Arduino Mega

Connections::
OLED display is I2C

Vcc to Arduino 5 volts
Gnd to Arduino Gnd
SDA to Analog pin 20
SCL to Analog pin 21

(Be very careful with the OLED connections, they will burn out if not connected correctly)

WTD588D::
Upload the audio to this device before connecting into the circuit

Arduino pin 3 to Module pin 7 RESET
Arduino pin 4 to Module pin 21 BUSY
Arduino pin 5 to Module pin 18 PO1
Arduino pin 6 to Module pin 17 PO2
Arduino pin 7 to Module pin 16 PO3
Arduino Gnd to Module pin 14
Arduino 5v to Module pin 20

Push Button::
Connect N/O push switch between
Arduino pin 2
Arduino pin Gnd
(Note pin 2 is held HIGH by the internal pullup resistor)

DS18B20 thremometer:
Gnd to Arduino Gnd
Data to Arduino pin 10
Vcc to Arduino 5 volts
A 4.7K resistor is connected between Vcc and Data

By Chris Rouse
Feb 2016
------------------------------------------------------------*/
// incluide the Library
#include "WT588D.h"  // audio module
#include "U8glib.h"  // graphics
#include <SPI.h>
#include <Wire.h> // I2C
#include <OneWire.h> // Thermometer
#include <DallasTemperature.h> // Thermometer
#include <SoftwareSerial.h>
//
// define pins etc
#define ONE_WIRE_BUS 10 // DS18B20 connected to arduino pin 10
// change between Centigrade and Fahrenheit here
// REM out the line not needed
boolean centigrade = true; //Un REM this for Celcius and REM out line below
//boolean centigrade = false; // Un REM this for Fahrenheit and REM out line above
//
// setup u8g object
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C 
//
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer;  
//
//
// set the correct pin connections for the WT588D chip
#define WT588D_RST 3  //Module pin "REST"
#define WT588D_CS 6   //Module pin "P02"
#define WT588D_SCL 7  //Module pin "P03"
#define WT588D_SDA 5  //Module pin "P01"
#define WT588D_BUSY 4 //Module pin "LED/BUSY"  
WT588D myWT588D(WT588D_RST, WT588D_CS, WT588D_SCL, WT588D_SDA, WT588D_BUSY);
#define talkPin 2 // pin used to request temperature
#define ledPin 13 // onboard LED
//
const int maxTempLimit = 125; // highest temperature to allow
const int minTempLimit = -25; // lowest temperature allowed
int decimal;
boolean minus = false; // false if temp is less then 0
float currentTemp = 0.00;
String thisTemp = "";
String thisData = "";
int maxTemp = 0; // maximum temperature reached
int minTemp = 0; // minimum temperature reached
int pad = 0;
float tempC; // centigrade value read from DS18B20
float tempF; // farenheit value read from DS18B20
boolean andPhrase = false;
int b;
int tensOffset = 0x12; // 2 less than its actual value
int hundredsOffset = 0x1b; // 1 less than its actual value

/*--------------------------------------------------------*/ 

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(talkPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // turn off onboard LED
  // draw first splash screen
  drawSplash();
  //
  // initialize the chip and port mappiing
  myWT588D.begin();
  //
  delay(500); // leave it displayed for a while
  //
  // Setup DS18B20
 // locate devices on the bus
  Serial.print("Locating DS19B20...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
  // set the resolution to 12 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 12);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
  //
  drawDataScreen(); // show data for the DS18B20 - if available
  delay(5000); // leave on screen for a while
  //
  //
  // play boot up sound
  speakPhrase(116); // ready
  speakPhrase(65); // digital thermometer  
}
/*--------------------------------------------------------*/  
void loop() {
  // read the thermometer
  getTemp();
  u8g.firstPage();  
  do {
    drawThermometer();
  } while( u8g.nextPage() );

  if(digitalRead(talkPin) == 0 ){ // talk key pressed
    speakTemperature();
  }

/*--------------------------------------------------------*/  
} // end main loop
/*--------------------------------------------------------*/ 
//
/*-----------------------------------------------------*/
void getTemp(){
  printTemperature(insideThermometer); // read the temperature
  if(centigrade){
    currentTemp = tempC;
  }
  else{
    currentTemp = tempF;     
  }
  //currentTemp = 21; // used to debug
  if(currentTemp <0){
    currentTemp = currentTemp * -1;
    minus = true;
  }
  else minus = false;
  int dp = currentTemp;
  decimal = (10 * currentTemp) - (10 * dp);
}
/*--------------------------------------------------------*/
//
void drawThermometer(void) {
  u8g.setFont(u8g_font_profont12);
  u8g.drawStr(30,10, "Temperature");
  u8g.setFont(u8g_font_profont12); 
 // show max temp reached 
  u8g.drawStr(10,25, "max");
  if(minus) currentTemp = currentTemp * -1;  
  if(currentTemp > maxTemp){ maxTemp = int(currentTemp);}
  if(minus) currentTemp = currentTemp * -1;  
  thisTemp = String(maxTemp);
  if(centigrade){
    thisTemp = thisTemp + "\260C";
  }
  else{
    thisTemp = thisTemp + "\260F";    
  }
  const char* maxTempC = (const char*) thisTemp.c_str();
  u8g.drawStr(30,25, maxTempC); 
 // show the min temp reached 
  u8g.drawStr(70,25, "min");
  if(minus) currentTemp = currentTemp * -1;
  if(currentTemp <= int(minTemp)){ minTemp = int(currentTemp);}
  if(minus) currentTemp = currentTemp * -1;  
  thisTemp = String(minTemp);
  if(centigrade){
    thisTemp = thisTemp + "\260C";
  }
  else{
    thisTemp = thisTemp + "\260F";    
  }
  const char* minTempC = (const char*) thisTemp.c_str();
  u8g.drawStr(90,25, minTempC);  
  u8g.setFont(u8g_font_profont29);
  int temp = currentTemp;
  thisTemp = String(temp) + "." + String(decimal);
  if(minus) thisTemp = "-" + thisTemp;
  if(centigrade){
    thisTemp = thisTemp + "\260C";
  }
  else{
    thisTemp = thisTemp + "\260F";    
  } 
  pad = thisTemp.length(); // get the string length
  pad = 8 - pad; // max length - actual length
  pad = pad * 16; // number of pixels free
  pad = (pad/2) + 1; // find the start point to ensure string is centred
  const char* newDispC = (const char*) thisTemp.c_str(); 
  u8g.drawStr(pad,50, newDispC);
}
/*-----------------------------------------------------*/
//
void speakTemperature(){
  getTemp();
  if(currentTemp >= maxTempLimit || (currentTemp >= minTempLimit && minus)){
    speakPhrase(0x92); // out of range
  }
  else{
    speakPhrase(0x87); // the temperature is 
    if(minus){ // temperature is negative
      speakPhrase(0x5F); // minus
    } 
    speakNumber(currentTemp);
    speakDecimal(currentTemp);
    // now add C or F
    //if(centigrade) speakPhrase(54); // degrees celcius
    if(centigrade) speakPhrase(55); // degrees centigrade
    else speakPhrase(56);  // degrees farenheit
  }
}

/*--------------------------------------------------------*/
// function to get the temperature for a DS18B20 device
void printTemperature(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  tempC = sensors.getTempC(deviceAddress);
  tempF = DallasTemperature::toFahrenheit(tempC); // Converts tempC to Fahrenheit
}
/*--------------------------------------------------------*/
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
/*--------------------------------------------------------*/
void drawDS18B20data(){
  u8g.setFont(u8g_font_profont15);  
  u8g.drawStr(10,13,"Locating DS18B20");
  thisData = "Found ";
  thisData += String(sensors.getDeviceCount(), DEC);
  if(sensors.getDeviceCount() == 1){
    thisData += " device";
  }
  else thisData += " devices";
  const char* newData = (const char*) thisData.c_str(); 
  u8g.drawStr(10,27, newData);
  if (!sensors.getAddress(insideThermometer, 0)){
    u8g.drawStr(10,40, "No address");
  }
  else{
    u8g.drawStr(10,42, "Using DS18B20");
    u8g.drawStr(10,55, "Loading data ..");
  }
}

void drawDataScreen(){
  // picture loop for DS18B20 data screen
  u8g.firstPage();  
  do {
    drawDS18B20data();
  } while( u8g.nextPage() );
  
}

/*--------------------------------------------------------*/
void busy(int pause){
  // waits for WT588D to finish sound
  delay(100);
  while (myWT588D.isBusy() ) {
  }
  delay(pause);
}

void speakPhrase(int phrase) {
  myWT588D.playSound(phrase);
  busy(0);
}
//
/*-----------------------------------------------------*/
void drawSplashScreen(){
    u8g.setFont(u8g_font_profont15); 
    u8g.drawStr(36,23, "Talking");
    u8g.drawStr(20,43, "Measurements");
    u8g.drawFrame(2,2,124,60);
    u8g.drawFrame(4,4,120,56);    
}

void drawSplash(){
      // picture loop for splash screen
    u8g.firstPage();  
    do {
      drawSplashScreen();
    } while( u8g.nextPage() );
}
/*--------------------------------------------------------*/
void speakNumber(int number){
  // will speak an integer from 1 to 9999
  // speak a number routine
  //
  if(number > 99){
   andPhrase = true;
  }
  else andPhrase = false;
  //
  if(minus) speakPhrase(0x5F); // minus
   // positive number over 1000
  if(number >= 1000){ 
    b = number/1000;
    speakPhrase(b);
    speakPhrase(0x8B); // thousand
    number = number - b*1000;
  }
  // positive number from 100 to 999
  if(number >=100){
    b = number/100;
    speakPhrase(b+ hundredsOffset);
    number = number - b*100;
    if(number > 0){
      speakPhrase(0x27); // short and
      andPhrase = false;
    }
  }
  // decade from 20 to 90
  if(number>=20){
    if(andPhrase ){
      speakPhrase(0x27); // short and
      andPhrase = false;
    }
    b = number/10;  
    speakPhrase(b + (tensOffset));
    number = number - b*10;
  }
  // below  20
  if(number>0){
    if(andPhrase){
      speakPhrase(0x27); // short and
    }
    speakPhrase(number);
  }
}
/*--------------------------------------------------------*/
void speakDecimal(float number){
  // speaks 1 decimal place 
  speakPhrase(0x72); // point
  b = (10 * number) - (10 * int(number));  
  speakPhrase(b);
 // b = (100 * number) - ((int(10 * number)) * 10);
  //if ( b > 0) speakPhrase(b);
}
/*--------------------------------------------------------*/

