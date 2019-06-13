#include "src/SPS30.h"
#include "src/VEML6075.h"
#include "src/si7021.h"
#include "src/GPSReader.h"
#include "src/SDLogger.h"
#include "src/RTC.h"

#include <Adafruit_BluefruitLE_UART.h>
#include "shared/BluefruitConfig.h"

#define GPSBaud 9600
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_HWSERIAL_NAME      Serial1
#define GPS_HWSERIAL_NAME           Serial2

Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
unsigned long Time;
unsigned long GPSTime;
String RTCTime;
bool StateChanged;

fijnstof f1;
VEML v1;
si7021 s1;
GPS g1;
unsigned long buzz = 0;

enum SaveSendState
{
  SDCARD_STATE,
  BLUETOOTH_STATE,
};

SaveSendState State = SDCARD_STATE;

void setup() {
  Serial.begin(115200);

  RTCSet();

  SDLoggerInit("LOGGER01.CSV");

  VEML6075Init();
  si7021Init();
  SPS30Init();

  GPS_HWSERIAL_NAME.begin(GPSBaud);

  ble.begin(VERBOSE_MODE);
  if (FACTORYRESET_ENABLE) if ( ! ble.factoryReset() ) Serial.println("Couldn't factory reset");
  ble.echo(false);
  ble.info(); //print bluetooth info
  ble.verbose(false); //disable debug
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // Change Mode LED Activity // LED Activity command is only supported from 0.6.6
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode

  SDLoggerSetWrite();
  Time = millis();
  GPSTime = millis();
  delay(1000);
}

void loop() {
  if (ble.isConnected()) {
    if (State == SDCARD_STATE) State = BLUETOOTH_STATE;
  }
  else {
    if (State == BLUETOOTH_STATE) State = SDCARD_STATE;
  }

  if (millis() > Time + 1000) {
    switch (State)
    {
      case SDCARD_STATE:
        WriteDataToSdCard();
        break;
      case BLUETOOTH_STATE:
        SendDataViaBluetooth();
        break;
    }
    Time = millis();
  }
  CheckPMValues();
}

void WriteDataToSdCard() {
  Serial.println("Bezig met naar kaart");

  RTCTime = String(RTCRead().unixtime());
  if (SPS30Read() == 0) {
    Serial.print(RTCTime + ",PM1p0," + String(f1.PM1p0) + '\n');
    Serial.print(RTCTime + ",PM2p5," + String(f1.PM2p5) + '\n');
    Serial.print(RTCTime + ",PM4p0," + String(f1.PM4p0) + '\n');
    Serial.print(RTCTime + ",PM10p0," + String(f1.PM10p0) + '\n');
    SDLoggerWrite(RTCTime + ",PM1p0," + String(f1.PM1p0) + '\n');
    SDLoggerWrite(RTCTime + ",PM2p5," + String(f1.PM2p5) + '\n');
    SDLoggerWrite(RTCTime + ",PM4p0," + String(f1.PM4p0) + '\n');
    SDLoggerWrite(RTCTime + ",PM10p0," + String(f1.PM10p0) + '\n');
  }
  SDLoggerFlush();

  RTCTime = String(RTCRead().unixtime());
  VEML6075Read();
  Serial.print(RTCTime + ",UVA," + String(v1.uva) + '\n');
  Serial.print(RTCTime + ",UVB," + String(v1.uvb) + '\n');
  Serial.print(RTCTime + ",UVIndex," + String(v1.uvindex) + '\n');
  SDLoggerWrite(RTCTime + ",UVA," + String(v1.uva) + '\n');
  SDLoggerWrite(RTCTime + ",UVB," + String(v1.uvb) + '\n');
  SDLoggerWrite(RTCTime + ",UVIndex," + String(v1.uvindex) + '\n');
  SDLoggerFlush();

  RTCTime = String(RTCRead().unixtime());
  si7021Read();
  Serial.print(RTCTime + ",Humidity," + s1.humidity + '\n');
  Serial.print(RTCTime + ",Temperature," + s1.Temperature + '\n');
  SDLoggerWrite(RTCTime + ",Humidity," + s1.humidity + '\n');
  SDLoggerWrite(RTCTime + ",Temperature," + s1.Temperature + '\n');
  SDLoggerFlush();

  while (GPS_HWSERIAL_NAME.available() > 0)
    if (GPSCheck() && millis() > GPSTime + 1000) {
      GPSRead();
      RTCTime = String(RTCRead().unixtime());
      Serial.print(RTCTime + ",GPS Latitude," + g1.Latitude + '\n');
      Serial.print(RTCTime + ",GPS Longitude," + g1.Longitude + '\n');
      SDLoggerWrite(RTCTime + ",GPS Latitude," + g1.Latitude + '\n');
      SDLoggerWrite(RTCTime + ",GPS Longitude," + g1.Longitude + '\n');
      SDLoggerFlush();
      GPSTime = millis();
    }

  if (millis() > 5000 && gpsChars() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
  }
}

void SendData() {
  Serial.println("Bezig met live");

  RTCTime = String(RTCRead().unixtime());
  if (SPS30Read() == 0) {
    Serial.print(RTCTime + ",PM1p0," + String(f1.PM1p0) + '\n');
    Serial.print(RTCTime + ",PM2p5," + String(f1.PM2p5) + '\n');
    Serial.print(RTCTime + ",PM4p0," + String(f1.PM4p0) + '\n');
    Serial.print(RTCTime + ",PM10p0," + String(f1.PM10p0) + '\n');
    ble.print(RTCTime + ",PM1p0," + String(f1.PM1p0) + '\n');
    delay(50);
    ble.print(RTCTime + ",PM2p5," + String(f1.PM2p5) + '\n');
    delay(50);
    ble.print(RTCTime + ",PM4p0," + String(f1.PM4p0) + '\n');
    delay(50);
    ble.print(RTCTime + ",PM10p0," + String(f1.PM10p0) + '\n');
    delay(50);
  }

  RTCTime = String(RTCRead().unixtime());
  VEML6075Read();
  Serial.print(RTCTime + ",UVA," + String(v1.uva) + '\n');
  Serial.print(RTCTime + ",UVB," + String(v1.uvb) + '\n');
  Serial.print(RTCTime + ",UVIndex," + String(v1.uvindex) + '\n');
  ble.print(RTCTime + ",UVA," + String(v1.uva) + '\n');
  delay(50);
  ble.print(RTCTime + ",UVB," + String(v1.uvb) + '\n');
  delay(50);
  ble.print(RTCTime + ",UVIndex," + String(v1.uvindex) + '\n');
  delay(50);

  RTCTime = String(RTCRead().unixtime());
  si7021Read();
  Serial.print(RTCTime + ",Humidity," + s1.humidity + '\n');
  Serial.print(RTCTime + ",Temperature," + s1.Temperature + '\n');
  ble.print(RTCTime + ",Humidity," + s1.humidity + '\n');
  delay(50);
  ble.print(RTCTime + ",Temperature," + s1.Temperature + '\n');
  delay(50);

  while (GPS_HWSERIAL_NAME.available() > 0)
    if (GPSCheck() && millis() > GPSTime + 1000) {
      GPSRead();
      RTCTime = String(RTCRead().unixtime());
      Serial.print(RTCTime + ",GPS Latitude," + g1.Latitude + '\n');
      Serial.print(RTCTime + ",GPS Longitude," + g1.Longitude + '\n');
      ble.print(RTCTime + ",GPS Latitude," + g1.Latitude + '\n');
      delay(50);
      ble.print(RTCTime + ",GPS Longitude," + g1.Longitude + '\n');
      delay(50);
      GPSTime = millis();
    }

  if (millis() > 5000 && gpsChars() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
  }
}

void SendDataViaBluetooth() {
  unsigned long BTTimer = millis();
  while ((millis() - BTTimer) < 10000) {
    WriteDataToSdCard();
    delay(1000);
  }
  while (ble.isConnected()) {
    if (SDLoggerFileExists()) {
      SDLoggerCloseFile();
      SDLoggerSetRead();
    }
    while (SDLoggerIsAvailable()) {
      String Data = SDLoggerRead();
      Serial.println("Bezig met bestand");
      Serial.println(Data);
      ble.println(Data);
      delay(50);
    }
    if (SDLoggerFileExists()) {
      SDLoggerCloseFile();
      SDLoggerDeleteFile();
    }
    SendData();
    CheckPMValues();
  }
  SDLoggerSetWrite();
}

void CheckPMValues() {
  if (f1.PM1p0 >= 10 || f1.PM2p5 >= 10 || f1.PM4p0 >= 10 || f1.PM10p0 >= 10) {
    if (buzz == 0 || millis() - buzz >=30000) {
      tone(31, 440, 1000);
      delay(1500);
      tone(31, 440, 1000);
      buzz = millis();
    }
  }
}
