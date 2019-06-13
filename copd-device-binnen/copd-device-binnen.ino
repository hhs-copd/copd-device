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
int low;
int high;
int scale;

//IMU libraries en objecten
#include "Registers.h"
typedef union accel_t_gyro_union
{
  struct
  {
    uint8_t x_accel_h; uint8_t x_accel_l; uint8_t y_accel_h; uint8_t y_accel_l; uint8_t z_accel_h; uint8_t z_accel_l; uint8_t t_h;
    uint8_t t_l; uint8_t x_gyro_h; uint8_t x_gyro_l; uint8_t y_gyro_h; uint8_t y_gyro_l; uint8_t z_gyro_h; uint8_t z_gyro_l;
  } reg;
  struct
  {
    int16_t x_accel; int16_t y_accel; int16_t z_accel; int16_t temperature; int16_t x_gyro; int16_t y_gyro; int16_t z_gyro;
  } value;
};
accel_t_gyro_union accel_t_gyro;
int loopcount = 0;



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
  //analogReference(EXTERNAL);

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


  //Initialiseer IMU
  Init_imu();


  //initialiseer stretch sensor
  Serial.println("Initialising Stretch Sensor");
  inputString.reserve(5);
  pinMode(REKPIN, INPUT);
  Calibrate();
}

void loop(void)
{
  if (millis() - lastcheck >= 1000) {
    lastcheck = millis();
    Time = RTC.now().unixtime();
    CheckForCalibration();
    ReadMPU();
    Serial.println("Bluetooth connected: " + String(ble.isConnected()));
    if (ble.isConnected()) SendDataViaBluetooth();
    else WriteDataToSdCard();
    loopcount++;
  }
  if(loopcount >= 60){
    Init_imu(); //imu opnieuw initialiseren om verkeerde waardes te voorkomen.
  }
}


void WriteDataToSdCard() {
  LOGLN("Bezig met naar kaart");
  Write(String(ReadStretchSensor()), "Thorax circumference");
  Write(String(accel_t_gyro.value.x_gyro), "GyroX");
  Write(String(accel_t_gyro.value.y_gyro), "GyroY");
  Write(String(accel_t_gyro.value.z_gyro), "GyroZ");
  Write(String(accel_t_gyro.value.x_accel), "AccelerometerX");
  Write(String(accel_t_gyro.value.y_accel), "AccelerometerY");
  Write(String(accel_t_gyro.value.z_accel), "AccelerometerZ");
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
    ble.print(Time + ",GyroX," + String(accel_t_gyro.value.x_gyro) + '\n');
    ble.print(Time + ",GyroY," + String(accel_t_gyro.value.y_gyro) + '\n');
    ble.print(Time + ",GyroZ," + String(accel_t_gyro.value.z_gyro) + '\n');
    LOG(Time + ",GyroX," + String(accel_t_gyro.value.x_gyro) + '\n');
    LOG(Time + ",GyroY," + String(accel_t_gyro.value.y_gyro) + '\n');
    LOG(Time + ",GyroZ," + String(accel_t_gyro.value.z_gyro) + '\n');
    ble.print(Time + ",AccelerometerX," + String(accel_t_gyro.value.x_accel) + '\n');
    ble.print(Time + ",AccelerometerY," + String(accel_t_gyro.value.y_accel) + '\n');
    ble.print(Time + ",AccelerometerZ," + String(accel_t_gyro.value.z_accel) + '\n');
    LOG(Time + ",AccelerometerX," + String(accel_t_gyro.value.x_accel) + '\n');
    LOG(Time + ",AccelerometerY," + String(accel_t_gyro.value.y_accel) + '\n');
    LOG(Time + ",AccelerometerZ," + String(accel_t_gyro.value.z_accel) + '\n');
    String thorax = String(ReadStretchSensor());
    ble.print(Time + ",Thorax circumference," + thorax + '\n');
    LOG(Time + ",Thorax circumference," + thorax + '\n');
    delay(1000);
  }
  SD.open(filename, FILE_WRITE);
}


void ReadMPU() {
  int error;
  double dT;
  // Read the raw values.
  // Read 14 bytes at once,
  // containing acceleration, temperature and gyro.
  // With the default settings of the MPU-6050,
  // there is no filter enabled, and the values
  // are not very stable.
  error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));
  // Swap all high and low bytes.
  // After this, the registers values are swapped,
  // so the structure name like x_accel_l does no
  // longer contain the lower byte.
  uint8_t swap;
#define SWAP(x,y) swap = x; x = y; y = swap

  SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
  SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
  SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
  SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
  SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
  SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);
}



void CheckForCalibration() {
  if (inputString == "&c") {
    //S.calibrate();
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

int MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);

  n = Wire.endTransmission(false);    // hold the I2C-bus
  if (n != 0)
    return (n);

  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while (Wire.available() && i < size)
  {
    buffer[i++] = Wire.read();
  }
  if ( i != size)
    return (-11);

  return (0);  // return : no error
}

int MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
    return (-20);

  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
    return (-21);

  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);

  return (0);         // return : no error
}

int MPU6050_write_reg(int reg, uint8_t data)
{
  int error;

  error = MPU6050_write(reg, &data, 1);

  return (error);
}

void Calibrate() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  low = analogRead(REKPIN);
}

int ReadStretchSensor(){
  int val = analogRead(REKPIN) - low; 
  if(val <0) return 0;
  else return val;  
}

void Init_imu(){
  int error;
  uint8_t c;
  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read (MPU6050_PWR_MGMT_1, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}
