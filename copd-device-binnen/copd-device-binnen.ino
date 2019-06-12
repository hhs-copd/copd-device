//SD libraries en objecten
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <RTClib.h>
char filename[] = "LOGGER00.CSV";
DateTime now;
String Time;
#define WAIT_TO_START 0
RTC_Millis RTC;
File logfile;
unsigned long lastcheck = 0;

//Bluetooth libraries en objecten
#include <Adafruit_BluefruitLE_UART.h>
#include "BluefruitConfig.h"
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_HWSERIAL_NAME      Serial1
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME , BLUEFRUIT_UART_MODE_PIN);
unsigned long millisconnected = 0;
unsigned long millisdisconnected = 0;

//Stretch sensor libraries en objecten
#include "Stretch.h"
#define REKPIN A8
String inputString = "";
StretchSensor S(REKPIN);

//IMU libraries en objecten
#include "Registers.h"
typedef union accel_t_gyro_union
{
  struct
  {
    uint8_t x_accel_h;uint8_t x_accel_l;uint8_t y_accel_h;uint8_t y_accel_l;uint8_t z_accel_h;uint8_t z_accel_l;uint8_t t_h;
    uint8_t t_l;uint8_t x_gyro_h;uint8_t x_gyro_l;uint8_t y_gyro_h;uint8_t y_gyro_l;uint8_t z_gyro_h;uint8_t z_gyro_l;
    } reg;
  struct
  {
    int16_t x_accel;int16_t y_accel;int16_t z_accel;int16_t temperature;int16_t x_gyro;int16_t y_gyro;int16_t z_gyro;
  } value;
};




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
  Serial.begin(115200);
  //wacht tot seriele port beschikbaar is.
#if WAIT_TO_START
  LOGLN(F("Type any character to start"));
  while (!Serial.available());
#endif

  //Initialiseer SD kaart
  LOGLN(F("Initializing SD card..."));
  if (!SD.begin(10)) error("Card failed, or not present");
  LOGLN(F("card initialized."));
  Wire.begin();
  RTC.begin(DateTime(F(__DATE__), F(__TIME__)));
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

  //Initialiseer Bluetooth
  Serial.println("Initialising Bluetooth module");
  ble.begin(VERBOSE_MODE);
  //if ( !ble.begin(VERBOSE_MODE) ) error("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  if (FACTORYRESET_ENABLE) if ( ! ble.factoryReset() ) error("Couldn't factory reset");
  ble.echo(false);
  ble.info(); //print bluetooth info
  ble.verbose(false); //disable debug
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // Change Mode LED Activity // LED Activity command is only supported from 0.6.6
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode

  //initialiseer stretch sensor
  Serial.println("Initialising Stretch Sensor");
  inputString.reserve(5);
  //S.calibrate();
}

void loop(void)
{
  if (millis() - lastcheck >= 500) {
    lastcheck = millis();
    Time = RTC.now().unixtime();
    CheckForCalibration();
    Serial.println("Bluetooth connected: " + String(ble.isConnected()));
    if (ble.isConnected()) SendDataViaBluetooth();
    else WriteDataToSdCard();
  }
}


void WriteDataToSdCard() {
  LOGLN("Bezig met naar kaart");
  Write(String(analogRead(REKPIN)), "Thorax circumference");
  logfile.flush();
}

void SendDataViaBluetooth() {
  unsigned long BTTimer = millis();
  while ((millis() - BTTimer) < 10000) {
    Time = RTC.now().unixtime();
    WriteDataToSdCard();
    delay(1000);
  }
  while (ble.isConnected()) {
    logfile.close();
    logfile = SD.open(filename);
    while (logfile.available()) {
      String Data = logfile.readStringUntil('\n');
      LOGLN("Bezig met bestand");
      LOGLN(Data);
      ble.print(Data);
      delay(50);
    }
    if (SD.exists(filename)) {
      logfile.close();
      SD.remove(filename);
    }
    LOGLN("Bezig met live");
    Time = RTC.now().unixtime();
    ble.print(Time + ",Thorax circumference," + String(analogRead(REKPIN)) + '\n');
    LOG(Time + ",Thorax circumference," + String(analogRead(REKPIN)) + '\n');
    delay(1000);
  }
  SD.open(filename, FILE_WRITE);
}


void CheckForCalibration() {
  if (inputString == "&c") {
    S.calibrate();
    inputString = "";
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
