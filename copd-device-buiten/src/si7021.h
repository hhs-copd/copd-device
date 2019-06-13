/* 
* si7021.h
*
* Created: 6/9/2019 3:42:15 PM
* Author: joey
*/


#ifndef __SI7021_H__
#define __SI7021_H__

#include <Arduino.h>
#include <Adafruit_Si7021.h>

struct si7021
{
	double humidity;
	double Temperature;
};

void si7021Init();
void si7021Read();

#endif //__SI7021_H__
