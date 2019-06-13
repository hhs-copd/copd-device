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
void SDLoggerWrite(String);
String SDLoggerRead();
bool SDLoggerIsAvailable();
bool SDLoggerSetRead();
bool SDLoggerSetWrite();
bool SDLoggerFileExists();
void SDLoggerFlush();
void SDLoggerDeleteFile();
void SDLoggerCloseFile();


#endif //__SDLOGGER_H__
