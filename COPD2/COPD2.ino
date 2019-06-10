#include "src/SPS30.h"
#include "src/VEML6075.h"
#include "src/si7021.h"
#include "src/SDLogger.h"
#include "src/RTC.h"

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Adafruit_BluefruitLE_UART.h>
#include "shared/BluefruitConfig.h"

#define RXPin 4
#define TXPin 3
#define GPSBaud 9600
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_HWSERIAL_NAME      Serial1

TinyGPSPlus gps;
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
unsigned long Time;

union LogVars
{
  char VEML6075Log[200];
  char si7021Log[200];
  char SPS30Log[200];
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

  Serial2.begin(GPSBaud);
  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();

  if ( !ble.begin(VERBOSE_MODE) ) Serial.println("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  if (FACTORYRESET_ENABLE) if ( ! ble.factoryReset() ) Serial.println("Couldn't factory reset");
  ble.echo(false);
  ble.info(); //print bluetooth info
  ble.verbose(false); //disable debug
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // Change Mode LED Activity // LED Activity command is only supported from 0.6.6
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode

  Time = millis();
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

    Time = millis();
  }

  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() + 2 < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour() + 2);
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
