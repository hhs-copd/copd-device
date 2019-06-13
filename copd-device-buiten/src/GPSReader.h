/*
 * GPS.h
 * 
 * Created: 6/11/2019 12:43:32 PM
 * Author: job
 */

#ifndef __GPSREADER_H__
#define __GPSREADER_H__
#define GPS_HWSERIAL_NAME           Serial2 
 
#include <TinyGPS++.h>

struct GPS
{
	double Latitude;
	double Longitude;
};

bool GPSCheck();
int gpsChars();
void GPSRead();


#endif
