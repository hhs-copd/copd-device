/* 
* si7021.cpp
*
* Created: 6/9/2019 3:42:15 PM
* Author: joey
*/


#include "si7021.h"

Adafruit_Si7021 sensor = Adafruit_Si7021();
extern struct si7021 s1;

void si7021Init()
{
	Serial.println("Si7021 test!");

	if (!sensor.begin()) {
		Serial.println("Did not find Si7021 sensor!");
		while (true)
		;
	}
	Serial.println("Si7021 started!");
}

void si7021Read()
{
	s1.humidity = sensor.readHumidity();
	s1.Temperature = sensor.readTemperature();
}