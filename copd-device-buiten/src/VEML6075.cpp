/* 
* VEML6075.cpp
*
* Created: 6/9/2019 3:30:59 PM
* Author: joey
*/


#include "VEML6075.h"

VEML6075 uv; // Create a VEML6075 object
extern struct VEML v1;

void VEML6075Init()
{
	if (uv.begin() == false) {
		Serial.println("Unable to communicate with VEML6075.");
		while (1) ;
	}
	
	Serial.println("VEML6075 started!");
}

void VEML6075Read()
{
	v1.uva = uv.uva();
	v1.uvb = uv.uvb();
	v1.uvindex = uv.index();
}