/*
 * GPS.cpp
 * 
 * Created: 6/11/2019 12:43:32 PM
 * Author: job
*/

#include "GPSReader.h"
TinyGPSPlus gps;
extern struct GPS g1;

bool GPSCheck(){
  return gps.encode(GPS_HWSERIAL_NAME.read());
}

int gpsChars(){
  return gps.charsProcessed();
}

void GPSRead(){
  g1.Latitude = gps.location.lat();
  g1.Longitude = gps.location.lng();
}
