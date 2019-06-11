#include "SPS30.h"
#include "VEML6075.h"
#include "si7021.h"
#include "GPSReader.h"
#include "SDLogger.h"
#include "RTC.h"

#include <SoftwareSerial.h>
#include <Adafruit_BluefruitLE_UART.h>
#include "shared/BluefruitConfig.h"


#define GPSBaud 9600
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_HWSERIAL_NAME     Serial1
#define GPS_HWSERIAL_NAME           Serial2 


Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
unsigned long Time;
unsigned long GPSTime;

union LogVars
{
  char VEML6075Log[200];
  char si7021Log[200];
  char SPS30Log[200];
  char GPSLog[200];
} l1;

//#define SetTime

void setup() {
  Serial.begin(115200);

#ifdef SetTime
  RTCSet();
#endif

  SDLoggerInit("LOGGER00.CSV");
  VEML6075Init();
  si7021Init();
  SPS30Init();

  GPS_HWSERIAL_NAME.begin(GPSBaud);

  if ( !ble.begin(VERBOSE_MODE) ) Serial.println("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  if (FACTORYRESET_ENABLE) if ( ! ble.factoryReset() ) Serial.println("Couldn't factory reset");
  ble.echo(false);
  ble.info(); //print bluetooth info
  ble.verbose(false); //disable debug
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // Change Mode LED Activity // LED Activity command is only supported from 0.6.6
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode

  Time = millis();
  GPSTime = millis();
  delay(1000);
}

void loop() {
  if (millis() > Time + 1000) {
    VEML6075Read().toCharArray(l1.VEML6075Log, sizeof(l1.VEML6075Log));
    Serial.println(l1.VEML6075Log);
    SDLoggerWrite(l1.VEML6075Log);

    si7021Read().toCharArray(l1.si7021Log, sizeof(l1.si7021Log));
    Serial.println(l1.si7021Log);
    SDLoggerWrite(l1.si7021Log);

    SPS30Read().toCharArray(l1.SPS30Log, sizeof(l1.SPS30Log));
    Serial.println(l1.SPS30Log);
    SDLoggerWrite(l1.SPS30Log);
    

    //get GPS data
    while(GPS_HWSERIAL_NAME.available() > 0)
    
      if (GPSCheck() && millis() > GPSTime + 1000){
        GPSRead().toCharArray(l1.GPSLog, sizeof(l1.GPSLog));
        Serial.println(l1.GPSLog);
        SDLoggerWrite(l1.GPSLog);
        GPSTime = millis();
      }

    if (millis() > 5000 && gpsChars() < 10){
      Serial.println(F("No GPS detected: check wiring."));
    }
    Time = millis();
  }
}
