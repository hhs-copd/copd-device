/*
 * GPS.cpp
 * 
 * Created: 6/11/2019 12:43:32 PM
 * Author: job
*/

#include "GPSReader.h"
TinyGPSPlus gps;

bool GPSCheck(){
  return gps.encode(GPS_HWSERIAL_NAME.read());
}

int gpsChars(){
  return gps.charsProcessed();
}

String GPSRead(){
  String gpsInfo = "Location: ";
  if (gps.location.isValid())
  {
    gpsInfo += String(gps.location.lat(), 6);
    gpsInfo += ",";
    gpsInfo += String(gps.location.lng(), 6);
  }
  else
  {
    gpsInfo += String("INVALID");
  }

  gpsInfo += "  Date/Time: ";
  if (gps.date.isValid())
  {
    gpsInfo += String(gps.date.month());
    gpsInfo += "/";
    gpsInfo += String(gps.date.day());
    gpsInfo += "/";
    gpsInfo += String(gps.date.year());
  }
  else
  {
    gpsInfo += "INVALID";
  }

  gpsInfo += " ";
  if (gps.time.isValid())
  {
    if (gps.time.hour() + 2 < 10) Serial.print(F("0"));
    gpsInfo += String(gps.time.hour() + 2);
    gpsInfo += ":";
    if (gps.time.minute() < 10) Serial.print(F("0"));
    gpsInfo += String(gps.time.minute());
    gpsInfo += ":";
    if (gps.time.second() < 10) Serial.print(F("0"));
    gpsInfo += String(gps.time.second());
    gpsInfo +=".";
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    gpsInfo += String(gps.time.centisecond());
  }
  else
  {
    gpsInfo += "INVALID";
  }

  return gpsInfo;
 }
