#include <Arduino.h>
#include <SafeWrite.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#define ANEMOMETER_SERIAL Serial1
#define GPS_SERIAL Serial2
#define PTH_SERIAL Serial3

class WeatherData
{
private:
    uint32_t timestamp_s_ = 0;
    uint32_t timestamp_us_ = 0;
    float u_ = 0;
    float v_ = 0;
    float w_ = 0;
    float pressure_ = 0;
    float temperature_ = 0;
    float humidity_ = 0;

public:
    void setTimestamp(uint32_t timestamp_s, uint32_t timestamp_us)
    {
        timestamp_s_ = timestamp_s;
        timestamp_us_ = timestamp_us;
    }
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
        sprintf(buffer, "%lu,%lu,%f,%f,%f,%f,%f,%f", timestamp_s_, timestamp_us_, u_, v_, w_, pressure_, temperature_, humidity_);
    }
};

SafeWrite SW(SafeWrite::TEENSY);
SFE_UBLOX_GNSS_SERIAL GPS;
WeatherData last_weather;
char raw_wind_data[32] = "";
int wind_data_iterator = 0;

uint32_t epoch;
uint32_t epoch_us; // microseconds returned by getUnixEpoch()
uint32_t time_last_gps_us;

void parseRawWindData()
{
    int i = 0;
    char *token = strtok(raw_wind_data, ",");
    while (token != NULL)
    {
        if (i == 1)
        {
            last_weather.setU(atof(token));
        }
        else if (i == 2)
        {
            last_weather.setV(atof(token));
        }
        else if (i == 3)
        {
            last_weather.setW(atof(token));
        }
        token = strtok(NULL, ",");
        i++;
    }
}

bool processWindData(char wind_char)
{
    // if (wind_char == 2) // Start of string character
    // {
    //     uint32_t elapsed_time_us = micros() - time_last_gps_us;
    //     last_weather.setTimestamp(epoch, epoch_us + elapsed_time_us);
    //     raw_wind_data[wind_data_iterator] = wind_char; // maybe remove
    //     wind_data_iterator++;
    //     return false;
    // }
    if (wind_char == 10) // line feed
    {
        // Parse the wind data string
        uint32_t elapsed_time_us = micros() - time_last_gps_us;
        last_weather.setTimestamp(epoch, epoch_us + elapsed_time_us);
        raw_wind_data[wind_data_iterator] = '\0'; // terminate the string
        wind_data_iterator = 0;
        parseRawWindData();
        return true;
    }
    else
    {
        raw_wind_data[wind_data_iterator] = wind_char;
        wind_data_iterator++;
        return false;
    }
}

void setupGPS()
{
    do
    {
        Serial.println("GNSS: trying 38400 baud");
        GPS_SERIAL.begin(38400);
        if (GPS.begin(GPS_SERIAL) == true)
            break;

        delay(100);
        Serial.println("GNSS: trying 9600 baud");
        GPS_SERIAL.begin(9600);
        if (GPS.begin(GPS_SERIAL) == true)
        {
            Serial.println("GNSS: connected at 9600 baud, switching to 38400");
            GPS.setSerialRate(38400);
            delay(100);
        }
        else
        {
            // GPS.factoryDefault();
            delay(2000); // Wait a bit before trying again to limit the Serial output
        }
    } while (1);
    Serial.println("GNSS serial connected");

    GPS.setUART1Output(COM_TYPE_UBX); // Set the UART port to output UBX only
    GPS.setNavigationFrequency(5);    // Produce 5 solutions per second
    GPS.setAutoPVT(true);             // Tell the GNSS to output each solution periodically
    GPS.saveConfiguration();          // Save the current settings to flash and BBR
}

void getGPSTime()
{
    if (GPS.getPVT() == true)
    {
        epoch = GPS.getUnixEpoch(epoch_us);
        time_last_gps_us = micros();
    }
}

void setup()
{
    Serial.begin(57600);
    while (!Serial)
        ; // Wait for user to open terminal
    Serial.println("Starting up...");
    ANEMOMETER_SERIAL.begin(57600);
    delay(1000);
    setupGPS();
    delay(1000);
    SW.init();

    char file_prefix[] = "EGFD";
    char folder_name[] = "anemometer_data";
    SW.createnew(file_prefix, folder_name, true);
    delay(1000);
}

void loop()
{
    char incoming_char = 0;
    bool new_frame = false;

    // Read the wind data
    if (ANEMOMETER_SERIAL.available() > 0)
    {
        incoming_char = ANEMOMETER_SERIAL.read();
        new_frame = processWindData(incoming_char);
    }
    // Read the GPS data
    // if (micros() - time_last_gps_us > 1000000) // 1 second has passed since the last GPS time update
    getGPSTime();
    // Read the PTH data

    // Write the data to the SD card if there is a new wind frame
    if (new_frame)
    {
        char write_buffer[128];
        last_weather.fillWriteBuffer(write_buffer);
        SW.write(write_buffer);
        Serial.println(write_buffer);
    }
}