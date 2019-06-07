#include <sps30.h>
//#include <SoftwareSerial.h>
#include <Adafruit_BluefruitLE_UART.h>
#include <Adafruit_Si7021.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "BluefruitConfig.h"
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

#define debug

#ifdef debug
#define LOG(...) Serial.print(__VA_ARGS__)
#define LOGLN(...) Serial.println(__VA_ARGS__)
#else
#define LOG(...)
#define LOGLN(...)
#endif

Adafruit_Si7021 sensor = Adafruit_Si7021();

#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0;     // time of last sync()

#define WAIT_TO_START 0 // Wait for serial input in setup()

RTC_DS1307 RTC; // define the Real Time Clock object

// the logging file
File logfile;
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                              BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);

char filename[] = "LOGGER00.CSV";
DateTime now;
String Time;
struct sps30_measurement measurement;

void error(String str)
{
  LOG(F("error: "));
  LOGLN(str);
  while (1);
}

void setup(void)
{
  Serial.begin(115200);
#if WAIT_TO_START
  LOG(F("Type any character to start\n"));
  while (!Serial.available());
#endif
  LOG(F("Initializing SD card..."));
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) error(F("Card failed, or not present"));
  LOG(F("card initialized.\n"));
  if (!sensor.begin()) error(F("Did not find Si7021 sensor!"));
  while  (sps30_probe() != 0) LOG(F("Could not probe sps30!\n"));
  if  (sps30_start_measurement() != 0) error(F("Could not start sps30 measurement!"));
  delay(10000);
  
  Wire.begin();
  if (!RTC.begin()) error(F("RTC failed"));
  for (uint8_t i = 0; i < 100; i++)
  {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (!SD.exists(filename))
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break; // leave the loop!
    }
  }
  if (!logfile) error(F("couldnt create file"));
  LOG(F("Logging to: "));
  LOGLN(filename);
  analogReference(EXTERNAL);

  if ( !ble.begin(VERBOSE_MODE) ) error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  LOGLN( F("OK!") );

  if (FACTORYRESET_ENABLE)
  {
    /* Perform a factory reset to make sure everything is in a known state */
    LOGLN(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  LOGLN(F("Requesting Bluefruit info:"));
  /* Print Bluefruit information */
  ble.info();
  ble.verbose(false);  // debug info is a little annoying after this point!

  LOGLN(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    LOGLN(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  LOGLN( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);
  
}






void loop(void)
{
  now = RTC.now();
  Time = '"' + String(now.hour(), DEC) + ':' + String(now.minute(), DEC) + ':' + String(now.second(), DEC) + '"';
  int16_t ret = sps30_read_measurement(&measurement);
  if (ble.isConnected()) {
    logfile.close();
    logfile = SD.open(filename);
    while (logfile.available()) {
      ble.print(logfile.readStringUntil('\n'));
    }
    logfile.close();
    while (ble.isConnected()) {
      ble.print(Time + ',' + "Temperature," + sensor.readTemperature() + '\n');
      LOG(Time + ',' + "Temperature," + sensor.readTemperature() + '\n');
      ble.print(Time + ',' + "Humidity," + sensor.readHumidity() + '\n');
      LOG(Time + ',' + "Humidity," + sensor.readHumidity() + '\n');
      ble.print(Time + ',' + "Particulate matter," + (String(measurement.mc_1p0, 4) + ";" + String(measurement.mc_2p5, 4) + ";" + String(measurement.mc_4p0, 4) + ";" + String(measurement.mc_10p0, 4)));
    }
    SD.remove(filename);
    logfile = SD.open(filename, FILE_WRITE);
  }

  else
  {
    Write(String(sensor.readTemperature()).c_str(), "Temperature");
    Write(String(sensor.readHumidity()).c_str(), "Humidity");
    //if(ret >= 0) Write(String(measurement.mc_1p0, 4) + ";" + String(measurement.mc_2p5, 4) + ";" + String(measurement.mc_4p0, 4) + ";" + String(measurement.mc_10p0, 4),F("Particulate matter"));
    // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
    // which uses a bunch of power and takes time
    if ((millis() - syncTime) < SYNC_INTERVAL)
      return;
    syncTime = millis();
    logfile.flush();
  }
}

void Write(const char* toWrite, const char* sensor)
{
  LOG(Time);
  LOG(',' + sensor + ',');
  LOG(toWrite);
  LOG(F("\n"));
  logfile.print(Time);
  logfile.print(',' + sensor + ',');
  logfile.print(toWrite);
  logfile.println();
}
