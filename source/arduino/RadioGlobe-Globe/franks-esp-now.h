#pragma once

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message1 {
    int16_t ns_cal;
    int16_t ew_cal;
    int16_t volumevalue; // new desirend volume value from display
    int16_t CalibrateZeroPos; // if set to 1234 by display, current globe position will be used as NS0 & EW0
    char RadioUrlRequest[128]; // tune to this radio station
} struct_from_display;

typedef struct struct_message2 {
    int16_t ns;
    int16_t ew;
    struct tm timeinfo;
    int16_t FindTimeZone;
    int16_t FindNewStation;
    char Name[64];
    char Title[128];
    char TimeZoneId[32];
    int16_t actualvolume;
    
} struct_from_globe;


// Create a struct_message to hold user actions from display/controller
struct_from_display DataFromDisplay;
struct_from_display PrevDataFromDisplay;

// Create a struct_message to hold data for display/controller 
struct_from_globe DataFromGlobe;

int16_t CalibrateZeroPos = 0;
