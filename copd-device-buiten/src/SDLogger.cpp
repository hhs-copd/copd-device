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

	if (!SD.begin(10)) {
		Serial.println("Card failed, or not present");
		// don't do anything more:
		while (1) ;
	}
	Serial.println("card initialized.");
	
	fileName = file;
}


void SDLoggerWrite(String string)
{	
	dataFile.print(string.c_str());
	//dataFile.close();
}



String SDLoggerRead()
{
	return dataFile.readStringUntil('\n');
}

bool SDLoggerIsAvailable()
{
	return dataFile.available();
}

bool SDLoggerSetRead()
{
	dataFile = SD.open(fileName.c_str(), FILE_READ);
	if (! dataFile) {
		Serial.println("error opening " + fileName);
		// Wait forever since we cant write data
		return false;
	}
	else return true;
}

bool SDLoggerSetWrite()
{
	dataFile = SD.open(fileName.c_str(), FILE_WRITE);
	if (! dataFile) {
		Serial.println("error opening " + fileName);
		// Wait forever since we cant write data
		return false;
	}
	else return true;
}

bool SDLoggerFileExists()
{
	return SD.exists(fileName.c_str());
}

void SDLoggerFlush()
{
	dataFile.flush();
}

void SDLoggerDeleteFile()
{	
	SD.remove(fileName.c_str());
}

void SDLoggerCloseFile()
{
	dataFile.close();
}