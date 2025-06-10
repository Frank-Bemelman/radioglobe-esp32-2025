#pragma once
//Structure example to send data
//Must match the receiver structure

typedef struct struct_message1 {
    int16_t ns_cal;
    int16_t ew_cal;
    int16_t volumevalue;
    int16_t CalibrateZeroPos;
    char RadioUrlRequest[128];
    
} struct_from_display;

typedef struct struct_message2 {
    int16_t ns;
    int16_t ew;
    struct tm timeinfo;
    int16_t FindTimeZone; // tells display that Globe is occupied finding timezone
    int16_t FindNewStation; // tells display a new station is to be suggested for calibrated coordinates
    char Name[64];
    char Title[128];
    char TimeZoneId[32];
    int16_t actualvolume;
} struct_from_globe;

// Create a struct_message called BME280Readings to hold sensor readings
struct_from_display DataFromDisplay;

// Create a struct_message to hold incoming sensor readings
struct_from_globe DataFromGlobe;
struct_from_globe PrevDataFromGlobe;
