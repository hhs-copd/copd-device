/* 
* VEML6075.h
*
* Created: 6/9/2019 3:30:59 PM
* Author: joey
*/


#ifndef __VEML6075_H__
#define __VEML6075_H__

#include <Arduino.h>
#include <SparkFun_VEML6075_Arduino_Library.h>

struct VEML
{
	float uva;
	float uvb;
	float uvindex;
};

void VEML6075Init();
void VEML6075Read();


#endif //__VEML6075_H__
