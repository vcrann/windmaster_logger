#include <Arduino.h>
#include <SafeWrite.h>
#define ANEMOMETER_SERIAL Serial1
#define GPS_SERIAL Serial2
#define PTH_SERIAL Serial3

class WeatherData
{
private:
  float u_ = 0;
  float v_ = 0;
  float w_ = 0;
  float pressure_ = 0;
  float temperature_ = 0;
  float humidity_ = 0;

public:
  void setWind(float u, float v, float w)
  {
    u_ = u;
    v_ = v;
    w_ = w;
  }
  void setU(float u)
  {
    u_ = u;
  }
  void setV(float v)
  {
    v_ = v;
  }
  void setW(float w)
  {
    w_ = w;
  }
  void setPressure(float pressure)
  {
    pressure_ = pressure;
  }
  void setTemperature(float temperature)
  {
    temperature_ = temperature;
  }
  void setHumidity(float humidity)
  {
    humidity_ = humidity;
  }
  float getU() { return u_; }
  float getV() { return v_; }
  float getW() { return w_; }
  void fillWriteBuffer(char *buffer)
  {
    sprintf(buffer, "%f,%f,%f", u_, v_, w_, pressure_, temperature_, humidity_);
  }
};

SafeWrite SW(SafeWrite::TEENSY);
WeatherData last_wind;
char raw_wind_data[32] = "";
int wind_data_iterator = 0;

void parseRawWindData()
{
  int i = 0;
  char *token = strtok(raw_wind_data, ",");
  while (token != NULL)
  {
    if (i == 1)
    {
      last_wind.setU(atof(token));
    }
    else if (i == 2)
    {
      last_wind.setV(atof(token));
    }
    else if (i == 3)
    {
      last_wind.setW(atof(token));
    }
    token = strtok(NULL, ",");
    i++;
  }
}

bool processWindData(char windChar)
{
  if (windChar == 10) // line feed
  {
    // Parse the wind data string
    raw_wind_data[wind_data_iterator] = '\0'; // terminate the string
    wind_data_iterator = 0;
    parseRawWindData();
    return true;
  }
  else
  {
    raw_wind_data[wind_data_iterator] = windChar;
    wind_data_iterator++;
    return false;
  }
}

void setup()
{
  Serial.begin(57600);
  ANEMOMETER_SERIAL.begin(57600);
  SW.init();

  char file_prefix[] = "EGFD";
  char folder_name[] = "anemometer_data";
  SW.createnew(file_prefix, folder_name, true);
  delay(1000);
}

void loop()
{
  char incomingByte = 0;
  bool new_frame = false;

  // Read the wind data
  if (ANEMOMETER_SERIAL.available() > 0)
  {
    incomingByte = ANEMOMETER_SERIAL.read();
    new_frame = processWindData(incomingByte);
  }
  // Read the GPS data

  // Read the PTH data

  // Write the data to the SD card if there is a new wind frame
  if (new_frame)
  {
    char write_buffer[64];
    last_wind.fillWriteBuffer(write_buffer);
    SW.write(write_buffer);
  }
}