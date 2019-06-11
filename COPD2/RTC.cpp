/* 
* RTC.cpp
*
* Created: 6/9/2019 7:53:06 PM
* Author: joey
*/

#include "RTC.h"

RTC_Millis rtc;

void RTCSet()
{
	// following line sets the RTC to the date & time this sketch was compiled
	rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
	// This line sets the RTC with an explicit date & time, for example to set
	// January 21, 2014 at 3am you would call:
	// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

void RTCRead()
{

}
