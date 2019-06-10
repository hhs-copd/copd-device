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
RTC_Millis RTC;
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

//Stretch sensor libraries en objecten
#include "Stretch.h"
#define REKPIN A0
String inputString = "";
StretchSensor S(REKPIN);

//Max30105 libraries en objecten
#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
long irValue;

float beatsPerMinute;
int beatAvg;

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
  pinMode(10, OUTPUT);
  if (!SD.begin(10, 11, 12, 13)) error("Card failed, or not present");
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
  if ( !ble.begin(VERBOSE_MODE) ) error("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  if (FACTORYRESET_ENABLE) if ( ! ble.factoryReset() ) error("Couldn't factory reset");
  ble.echo(false);
  ble.info(); //print bluetooth info
  ble.verbose(false); //disable debug
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // Change Mode LED Activity // LED Activity command is only supported from 0.6.6
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode

  //initialiseer stretch sensor
  inputString.reserve(5);
  S.calibrate();

  //initialiseer max30105
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) error("MAX30105 was not found. Please check wiring/power.");//Use default I2C port, 400kHz speed
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

}

void loop(void)
{
  now = RTC.now();
  Time = '"' + String(now.hour(), DEC) + ':' + String(now.minute(), DEC) + ':' + String(now.second(), DEC) + '"';
  irValue = particleSensor.getIR();
  
  CheckForCalibration();
  CheckForHeartBeat(irValue);
  if (ble.isConnected()) SendDataViaBluetooth();
  else WriteDataToSdCard();
}

void WriteDataToSdCard(){
  if (irValue > 50000) Write(String(beatAvg), "Heartrate");
    Write(String(analogRead(REKPIN)), "Thorax circumference");
    if ((millis() - syncTime) < SYNC_INTERVAL)
      return;
    syncTime = millis();
    logfile.flush();
}

void SendDataViaBluetooth(){
  logfile.close();
    logfile = SD.open(filename);
    while (logfile.available()) {
      ble.print(logfile.readStringUntil('\n'));
    }
    logfile.close();
    while (ble.isConnected()) {
      ble.print(Time + ",Thorax circumference," + String(analogRead(REKPIN)) + '\n');
      LOG(Time + ",Thorax circumference," + String(analogRead(REKPIN)) + '\n');
      if (irValue > 50000) ble.print(Time + ",Heartrate," + String(beatAvg) + '\n');
    }
    SD.remove(filename);
    logfile = SD.open(filename, FILE_WRITE);
}


void CheckForCalibration(){
  if (inputString == "&c") {
    S.calibrate();
    inputString = "";
  }
}

void CheckForHeartBeat(long irValue){
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
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
