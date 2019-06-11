/* 
* SDLogger.h
*
* Created: 6/9/2019 7:53:34 PM
* Author: joey
*/


#ifndef __SDLOGGER_H__
#define __SDLOGGER_H__

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

void SDLoggerInit(String);
void SDLoggerWrite(const char*);
void SDLoggerRead();
void SDLoggerGetString();


#endif //__SDLOGGER_H__
