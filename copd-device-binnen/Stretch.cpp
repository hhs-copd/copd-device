#include "Stretch.h"

StretchSensor::StretchSensor(int pin) : m_Pin(pin), m_Low(0), m_High(0) {}

void StretchSensor::calibrate() {
  Serial.println("Calibration Started!");
  Serial.println("Keep the band in it's resting position for 5 seconds");
  setLow();

}


void StretchSensor::setLow() {
  int lowp = analogRead(m_Pin);
  unsigned long Start = 5000 + millis();
  
  while ((Start - millis()) > 0) {
    if ((analogRead(m_Pin) < lowp - 5) || (analogRead(m_Pin) > lowp + 5)) {
      Start = millis();
      lowp = analogRead(m_Pin);
    }
    else if (((Start - millis()) % 1000) == 0) {
      Serial.print(((Start - millis()) / 1000), 0);
      Serial.println(" Seconds!");
    }
  }

  m_Low = lowp;
  Serial.println("Low done!");

}

void StretchSensor::setHigh() {

}
