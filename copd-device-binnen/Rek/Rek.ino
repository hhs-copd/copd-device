#include "Stretch.h"

#define REKPIN A8

String inputString = "";
StretchSensor S(REKPIN);

void setup() {
  Serial.begin(115200);
  inputString.reserve(5);
}

void loop() {

  S.printValue();
  if (inputString == "&c") {
    S.calibrate();
    inputString = "";
  }
  delay(2000);
}

void serialEvent() {
  if (Serial.available() > 0) {
    char inChar = (char)Serial.read();

    inputString += inChar;
  }
}
