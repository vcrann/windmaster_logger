#include "SafeWrite.h"


//Empty constructor. Setup is all done in the init() function.
SafeWrite::SafeWrite(options option)
{
	this->_board_type = option;
}


//Constructor to enable serial debug messages.
SafeWrite::SafeWrite(options option, Stream* serial) : _serial(serial)
{
	//Store the pointer to the serial output stream and enable the debug messages
	this->_serialDebug = true;
	this->_board_type = option;
}


//Init. the sd card hardware.
int SafeWrite::init(void)
{
	//Configure options depending upon board type
	if (this->_board_type==DRAGON) {
		this->SD_CS_PIN = 19;
		int SD_FREQ = 5;
		
		if (!this->_sd.begin(SD_CS_PIN, SD_SCK_MHZ(SD_FREQ))) {
			this->_isSDInit = false;
			if (_serialDebug) {
				this->_serial->println("Could not initialize DragonSpike SD card");
			}
			return 1;
		}
		else {
			this->_isSDInit = true;
		}
	}
	
	if (this->_board_type==TEENSY) {
		#ifndef SDCARD_SS_PIN
		this->SD_CS_PIN = SS;
		#else
		this->SD_CS_PIN = SDCARD_SS_PIN;
		#endif
		
		if (!this->_sd.begin(SdioConfig(FIFO_SDIO))) {
			this->_isSDInit = false;
			if (_serialDebug) {
				this->_serial->println("Could not initialize Teensy SD card");
			}
			return 1;
		}
		else {
			this->_isSDInit = true;
		}
	}
	
	return 0;
}


//Create a new unique filename in the given directory.
//The baseName has a incrementing counter added to ensure every new file is unique.
//The folder name is also set using a string.
//If safeOn==true, then also open a backup file for dual, non-simultaneous writing.
int SafeWrite::createnew(const char *baseName, const char *folderName, bool safeOn) 
{
	if (safeOn) {
		this->_isSafeOn = safeOn;
	}
	
	if (this->_isSDInit) {
		
		//Check/create the folder
		if (!this->_sd.exists(folderName)) {
			if (!this->_sd.mkdir(folderName)) {
				if (_serialDebug) {
					this->_serial->println("Failed to make new folder");
				}
				return 1;
			}
		}
		//Change the working directory
		if (!this->_sd.chdir(folderName)) {
			if (_serialDebug) {
				this->_serial->println("Failed to change directory");
			}
			return 1;
		}
		
		if (_serialDebug) {
			this->_serial->print("Working in folder: '");
			this->_serial->print(folderName);
			this->_serial->println("'");
		}
		
  
		//Iterate file names until a unique one is found
		int _fileN = 0;
		sprintf(this->_mainFileName,"%s_%03dm.dat",baseName,_fileN);
		while(this->_sd.exists(this->_mainFileName)) {
			_fileN++;
			sprintf(this->_mainFileName,"%s_%03dm.dat",baseName,_fileN);
		}
		
		//Try and open the file
		FsFile mainFile = this->_sd.open(this->_mainFileName,FILE_WRITE);
		if (!mainFile) {
			if (_serialDebug) {
				this->_serial->println("Failed to open main file");
			}
			return 1;
		}
		
		if (_serialDebug) {
			this->_serial->print("Created main file: '");
			this->_serial->print(this->_mainFileName);
			this->_serial->println("'");
		}
		
		//Write a header line
		mainFile.println("SafeWrite v0.1 Main File");
		mainFile.close();
		
		if (this->_isSafeOn) {
			//Make a backup file using the same numbering.
			//Assumes there will be no conflict if the main file was unique.
			sprintf(this->_bkupFileName,"%s_%03db.dat",baseName,_fileN);
			//Try and open the file
			FsFile backupFile = this->_sd.open(this->_bkupFileName,FILE_WRITE);
			if (!backupFile) {
				if (_serialDebug) {
					this->_serial->println("Failed to open backup file");
				}
				return 1;
			}
			
			if (_serialDebug) {
				this->_serial->print("Created backup file: '");
				this->_serial->print(this->_bkupFileName);
				this->_serial->println("'");
			}
			
			//Write a header line
			backupFile.println("SafeWrite v0.1 Backup File");
			backupFile.close();
		}
		
		//All succeded, so return 0
		return 0;
	}
	
	//The SD card was not initilaised.
	if (_serialDebug) {
		this->_serial->println("SD card not initialized");
	}
	return 1;
}


//Write the char message to the mainFile. If safeOn=true, then
//also write to an identical backup file. Always appends.
//Write to file and a backup. Only one file is open at once, hence 
//file corruption due to power loss cannot affect both files.
int SafeWrite::write(char message[]) 
{
	if (this->_isSDInit) {
		
		//Try and open the file
		FsFile mainFile = this->_sd.open(this->_mainFileName,FILE_WRITE);
		if (!mainFile) {
			if (_serialDebug) {
				this->_serial->println("Failed to open main file");
			}
			return 1;
		}
		mainFile.println(message);
		mainFile.close();
		
		//Try to write to the backup file
		if (this->_isSafeOn) {
			FsFile backupFile = this->_sd.open(this->_bkupFileName,FILE_WRITE);
			if (!backupFile) {
				if (_serialDebug) {
					this->_serial->println("Failed to open backup file");
				}
				return 1;
			}
			backupFile.println(message);
			backupFile.close();
		}
	
		//All succeded, so return 0
		return 0;
	}
	
	//The SD card was not initilaised.
	if (_serialDebug) {
		this->_serial->println("SD card not initialized");
	}
	return 1;
}
//eof