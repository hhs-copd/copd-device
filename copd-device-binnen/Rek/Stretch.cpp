#include "Stretch.h"

StretchSensor::StretchSensor(int pin) : m_Pin(pin), m_Low(0), m_High(0) {}

void StretchSensor::printValue() {
  Serial.println(map(analogRead(m_Pin), m_Low, m_High, 0, 100));
}

void StretchSensor::calibrate() {
  Serial.println("Calibration Started!");
  Serial.println("Keep the band in it's resting position for 5 seconds");
  setLow();
  Serial.println("Low done!");
  Serial.println("Stretch the band as far as you can and hold it for 5 seconds");
  setHigh();
  Serial.println("High done!");
  Serial.println("Calibrated succesfully!");
}


void StretchSensor::setLow() {
  int lowp = analogRead(m_Pin);
  int counter = 5;
  unsigned long Start = 5000 + millis();
  

  while ((Start - millis()) > 0) {
    int x = analogRead(m_Pin);
    if ((x < lowp - 5) || (x > lowp + 5)) {
      Start = 5000 + millis();
      lowp = x;
      counter = 5;
    }
    else if (((float)(Start - millis()) / 1000) == counter) {
      Serial.print(((Start - millis()) / 1000), DEC);
      Serial.println(" Seconds!");
      counter--;
    }
  }
  m_Low = lowp;
}

void StretchSensor::setHigh() {
  int highp = analogRead(m_Pin);
  int counter = 5;
  unsigned long Start = 5000 + millis();

  while ((Start - millis()) > 0) {
    int x = analogRead(m_Pin);
    if (x > (m_Low - 50)) {
      Serial.println("Not stretched far enough!");
      Start = 5000 + millis();
      counter = 5;
    }
    else if ((x < highp - 5) || (x > highp + 5)) {
      Start = 5000 + millis();
      highp = x;
      counter = 5;
    }
    else if (((float)(Start - millis()) / 1000) == counter) {
      Serial.print(((Start - millis()) / 1000), DEC);
      Serial.println(" Seconds!");
      counter--;
    }
  }
  m_High = highp;
}
