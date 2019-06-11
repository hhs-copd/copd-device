/* 
* SDLogger.cpp
*
* Created: 6/9/2019 7:53:34 PM
* Author: joey
*/


#include "SDLogger.h"

File dataFile;
String fileName = "";

void SDLoggerInit(String file)
{
	Serial.print("Initializing SD card...");
	// make sure that the default chip select pin is set to
	// output, even if you don't use it:
	pinMode(SS, OUTPUT);

	if (!SD.begin(10, 11, 12, 13)) {
		Serial.println("Card failed, or not present");
		// don't do anything more:
		while (1) ;
	}
	Serial.println("card initialized.");
	
	fileName = file;
}


void SDLoggerWrite(const char* string)
{
	dataFile = SD.open(fileName.c_str(), FILE_WRITE);
	if (! dataFile) {
		Serial.println("error opening " + fileName);
		// Wait forever since we cant write data
		while (1) ;
	}
	
	dataFile.println(string);
	dataFile.close();
}

void SDLoggerRead(const char* string)
{
	dataFile = SD.open(fileName.c_str(), FILE_READ);
	if (! dataFile) {
		Serial.println("error opening datalog.txt");
		// Wait forever since we cant write data
		while (1) ;
	}
	while (dataFile.available()) //ble.print(dataFile.readBytesUntil('\n'));
	dataFile.close();
}

void SDLoggerGetString()
{
	
}