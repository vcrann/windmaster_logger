#ifndef SafeWrite_h
#define SafeWrite_h
/*
  SafeWrite.h - Library for saving to SD.
  Created by Kieran T. Wood, August 23, 2021.
  Released into the public domain.
*/
#include "SPI.h"
#include "SdFat.h"		//SdFat.h is purported to be much faster than SD.h
#include "sdios.h"
#include "Arduino.h"

#define SdFat_Basic 0

class SafeWrite
{
  public:
  	enum options {
		TEENSY, DRAGON
	};
	
    SafeWrite(options option);
	SafeWrite(options option, Stream* serial);
	
    int init(void);
	
    int createnew(const char *baseName, const char *folderName, bool safeOn=true);
	
    int write(char message[]);
	
  private:
	//Debug things
	bool _serialDebug = false;
	Stream* _serial;
  
	//SD hardware settings
	options _board_type;
	#if SdFat_Basic
		SdFat _sd;
	#else
		SdFs _sd;
	#endif
	uint8_t SD_CS_PIN;

	
	//File names and flags
    char _mainFileName[32];
    char _bkupFileName[32];
	bool _isMainOpen = false;
	bool _isBkupOpen = false;
    bool _isSDInit = false;
	bool _isSafeOn = false;
};

#endif
//eof