//SD libraries en objecten
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <RTClib.h>
char filename[] = "LOGGER00.CSV";
DateTime now;
String Time;
#define SYNC_INTERVAL 1000
#define WAIT_TO_START 0
RTC_DS1307 RTC;
File logfile;
uint32_t syncTime = 0;

//Bluetooth libraries en objecten
#include <Adafruit_BluefruitLE_UART.h>
#include "BluefruitConfig.h"
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);
Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN, BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);

//fijnstof libraries en objecten
#include <sps30.h>
struct sps30_measurement measurement;

//temp&humidity  libraries en objecten
#include <Adafruit_Si7021.h>
Adafruit_Si7021 sensor = Adafruit_Si7021();

//UV libraries en objecten
#include <SparkFun_VEML6075_Arduino_Library.h>
VEML6075 uv; 

#define debug
#ifdef debug
#define LOG(...) Serial.print(__VA_ARGS__)
#define LOGLN(...) Serial.println(__VA_ARGS__)
#else
#define LOG(...)
#define LOGLN(...)
#endif

void error(const char* str)
{
  LOGLN(str);
  while (1);
}

void setup(void)
{
  Serial.begin(9600);
  //wacht tot seriele port beschikbaar is.
  #if WAIT_TO_START
  LOGLN(F("Type any character to start"));
  while (!Serial.available());
  #endif

  //Initialiseer SD kaart
  LOGLN(F("Initializing SD card..."));
  pinMode(10, OUTPUT);
  if (!SD.begin(10/*, 11, 12, 13*/)) error("Card failed, or not present");
  LOGLN(F("card initialized."));
  Wire.begin();
  if (!RTC.begin()) error("RTC failed");
  for (uint8_t i = 0; i < 100; i++)
  {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (!SD.exists(filename))
    {
      logfile = SD.open(filename, FILE_WRITE);
      break;
    }
  }
  if (!logfile) error("couldnt create file");
  LOG(F("Logging to: "));
  LOGLN(filename);
  analogReference(EXTERNAL);

  //Initialiseer SI7021 sensor
  if (!sensor.begin()) error("Did not find Si7021 sensor!");

  //Initialiseer Uv sensor
  //if (uv.begin() == false) error("Unable to communicate with VEML6075.");
    
  Serial1.begin(115200);
  //Initialiseer sps30 sensor
  while  (sps30_probe() != 0) LOGLN(F("Could not probe sps30!"));
  if  (sps30_start_measurement() != 0) error("Could not start sps30 measurement!");
  delay(10000);

  
  
  //Initialiseer Bluetooth
  if ( !ble.begin(VERBOSE_MODE) ) error("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  if (FACTORYRESET_ENABLE) if ( ! ble.factoryReset() ) error("Couldn't factory reset");
  ble.echo(false);
  ble.info(); //print bluetooth info
  ble.verbose(false); //disable debug
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // Change Mode LED Activity // LED Activity command is only supported from 0.6.6
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode

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
      ble.print(Time + ",Temperature," + sensor.readTemperature() + '\n');
      LOG(Time + ",Temperature," + sensor.readTemperature() + '\n');
      ble.print(Time + ",Humidity," + sensor.readHumidity() + '\n');
      LOG(Time +",Humidity," + sensor.readHumidity() + '\n');
      //ble.print(Time + ",UV-B," + String(uv.uvb()) + '\n');
      //LOG(Time + ",UV-B," + String(uv.uvb()) + '\n');
      ble.print(Time + ",Particulate matter," + (String(measurement.mc_1p0, 4) + ";" + String(measurement.mc_2p5, 4) + ";" + String(measurement.mc_4p0, 4) + ";" + String(measurement.mc_10p0, 4)));
    }
    SD.remove(filename);
    logfile = SD.open(filename, FILE_WRITE);
  }

  else
  {
    //Write(String(sensor.readTemperature()), "Temperature");
    //Write(String(sensor.readHumidity()), "Humidity");
    Write(String(uv.uvb()), "UV-B");
    if (ret >= 0) Write((String(measurement.mc_1p0, 4) + ";" + String(measurement.mc_2p5, 4) + ";" + String(measurement.mc_4p0, 4) + ";" + String(measurement.mc_10p0, 4)).c_str(), "Particulate matter");
    // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
    // which uses a bunch of power and takes time
    if ((millis() - syncTime) < SYNC_INTERVAL)
      return;
    syncTime = millis();
    logfile.flush();
  }
}

void Write(String toWrite, String sensor)
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
