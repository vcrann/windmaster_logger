#include "SafeWrite.h"

SafeWrite SW(SafeWrite::DRAGON,&Serial);
//SafeWrite SW(SafeWrite::TEENSY,&Serial);

int count = 0;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println("Test SD SafeWrite Library");
  Serial.println("University of Manchester");
  Serial.println("kieran.wood@manchester.ac.uk");
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

  //If the SPI is shared by multiple devices (with dedicated CS pins for each),
  //then care must be taken to ensure the additional devices are initialised or
  //their CS pin is high to avoid conflicts.
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);
  SW.init();

  //Provide the file prefix (max. 4 characters), and folder name.
  //The third argument controls if two files are created and written sequentially.
  char filePrefix[] = { 'T', 'E', 'S', 'T', 0 };
  char folderName[] = { 'D', 'E', 'F', 'A', 'U', 'L', 'T', 0 };
  SW.createnew(filePrefix,folderName,true);

  delay(1000);
}

void loop() {
  count+=1;

  //The write function takes a string buffer (C-style) and writes it as a new line
  char writeBuf[64];
  sprintf(writeBuf,"Count=%d",count);
  SW.write(writeBuf);

  Serial.println("Write Complete");
  delay(1000);
}
//eof
