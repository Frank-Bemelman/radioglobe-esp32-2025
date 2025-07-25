#pragma once

#define QUEUESIZE 50
#define QUEUEMESSAGELENGTH 128
typedef struct {               
  char     QueueMessage[QUEUESIZE][QUEUEMESSAGELENGTH];  // 50 strings of 128 each (6400 bytes) 
  uint16_t QueueMessageType[QUEUESIZE];
  uint16_t QueueIndexIn;
  uint16_t QueueIndexOut;
  uint16_t QueueCnt;
} QueueOut;

const char * messagetexts[] = {
   { "UNSUPPORTED MESSAGE"},
   { "SONG_TITLE 1"},
   { "ARTIST 2"},
   { "ALBUM 3"},
   { "GENRE 4"},
   { "STATION_NAME 5"},
   { "DESCRIPTION 6"},
   { "GET_GOOGLE_API_KEY 7"},
   { "GOOGLE_API_KEY 8"},
   { "TIMEZONE_ID 9"},
   { "TIMEZONE_NAME 10"},
   { "GET_WIFI_CREDENTIALS 11"},
   { "---------- 12"},
   { "SSID_FOR_GLOBE 13"},
   { "PASSWORD_FOR_GLOBE 14"},
   { "WIFI_STATUS 15"},
   { "CALIBRATE_ZERO 16"},
   { "FINDNEWSTATION 17"},
   { "GLOBE_MAC 18"},
   { "GLOBE_IP 19"},
   { "DEAD_STATION 20"},
   { "START_THIS_STATION 21"},
   { "PAUSE_RADIO 22"},
   { "RESUME_RADIO 23"},
   { "WANT_NEXT_STATION 24"},
   { "STATION_CONNECTED 25"},
   { "GLOBE_WANTS_CURRENT_STATION 26"}, // when globe reboots
   { "GET_TIMEZONE_BY_GPS 27"},
   { "STREAMING_IDLE_MS 28"},
   { "NEW_LIST_LOADED 29"}, // after a succesful search
   { "GET_TIMEZONE 30"},
   { "POWERDOWN 31"},
   { "POWERUP 32"},
   { "VOLUME_AND_TONE 33"},
   { "DISPLAY_WANTS_VOLUME_AND_TONE 34"},
   { "STORE_VOLUME_AND_TONE 35"},
   { "AUDIO_EOF_STREAM 36"},
   { "---------- 37"},
   { "---------- 38"},
   { "---------- 39"},
   { "GET_GEOLOCATION_BY_GPS 40"},
   { "CONNECTTOHOST_FAILURE 41"}
 };


#define MESSAGE_SONG_TITLE 1
#define MESSAGE_ARTIST 2
#define MESSAGE_ALBUM 3
#define MESSAGE_GENRE 4
#define MESSAGE_STATION_NAME 5
#define MESSAGE_DESCRIPTION 6
#define MESSAGE_GET_GOOGLE_API_KEY 7
#define MESSAGE_GOOGLE_API_KEY 8
#define MESSAGE_TIMEZONE_ID 9
#define MESSAGE_TIMEZONE_NAME 10
#define MESSAGE_GET_WIFI_CREDENTIALS 11
///
#define MESSAGE_SSID_FOR_GLOBE 13
#define MESSAGE_PASSWORD_FOR_GLOBE 14
#define MESSAGE_NO_WIFI 15
#define MESSAGE_CALIBRATE_ZERO 16
#define MESSAGE_FINDNEWSTATION 17
#define MESSAGE_GLOBE_MAC 18
#define MESSAGE_GLOBE_IP 19
#define MESSAGE_DEAD_STATION 20
// as a result from a roller selection made
#define MESSAGE_START_THIS_STATION 21
#define MESSAGE_PAUSE_RADIO 22
#define MESSAGE_RESUME_RADIO 23
#define MESSAGE_WANT_NEXT_STATION 24
#define MESSAGE_STATION_CONNECTED 25
// when globe reboots
#define MESSAGE_GLOBE_WANTS_CURRENT_STATION 26
// request from display to get a certain timezone
#define MESSAGE_GET_TIMEZONE_BY_GPS 27
#define MESSAGE_STREAMING_IDLE_MS 28
#define MESSAGE_NEW_LIST_LOADED 29 // after a succesful search by display
#define MESSAGE_GET_TIMEZONE 30
#define MESSAGE_POWERDOWN 31
#define MESSAGE_POWERUP 32
#define MESSAGE_VOLUME_AND_TONE 33 
#define MESSAGE_DISPLAY_WANTS_VOLUME_AND_TONE 34
#define MESSAGE_STORE_VOLUME_AND_TONE 35
#define MESSAGE_AUDIO_EOF_STREAM 36
///
////
///
#define MESSAGE_GET_GEOLOCATION_BY_GPS 40
#define MESSAGE_CONNECTTOHOST_FAILURE 41

#define MESSAGE_URL_HTTP_400_BAD_REQUEST 400
#define MESSAGE_URL_HTTP_403_REFUSED 403
#define MESSAGE_URL_HTTP_404_NOTFOUND 404





typedef struct struct_message1 {
    int16_t  ns_cal;
    int16_t  ew_cal;
    uint16_t volumevalue;
    uint16_t bassvalue;
    int16_t  treblevalue;
    int16_t  CalibrateZeroPos;
    uint16_t D_QueueSerialNumber;
    uint16_t D_QueueMessageType;
    uint16_t D_QueueStationIndex; // station connected -1 if idle
    char     D_QueueMessage[QUEUEMESSAGELENGTH];
    uint16_t G_QueueSerialNumber; // echo back to globe confirm reception
    float    D_StationGpsNS; // used by globe to get timezone
    float    D_StationGpsEW; // used by globe to get timezone
} struct_from_display;

typedef struct struct_message2 {
    int16_t  ns;
    int16_t  ew;
    struct   tm timeinfo;
    int16_t  FindTimeZone; // tells display that Globe is occupied finding timezone
    int16_t  FindGeoLocationData; // tells display that Globe is occupied finding geolocation data
    int16_t  Unraveling; // tells display that Globe is occupied unraveling .m3u or .pls url to get the actual streaming url
    int16_t  UnravelingResult; // tells display that Globe is occupied unraveling .m3u or .pls url to get the actual streaming url
    uint16_t G_QueueSerialNumber;
    uint16_t G_QueueMessageType;
    uint16_t D_QueueStationIndex; // echo back unaltered
    char     G_QueueMessage[QUEUEMESSAGELENGTH];
    uint16_t D_QueueSerialNumber; // echo back to display confirm reception
    uint16_t D_QueueMessageType;
    uint16_t G_QueueBytesStreamed; // can be used to monitor actual streaming of data
    uint32_t G_QueueStreamIdleMs; // copier idle time
    bool     G_EncoderReliable;


} struct_from_globe;




// Declare a structurs named DataFromGlobe to hold various variables
struct_from_globe DataFromGlobe;


// Declare two structures named DataFromDisplay for storing/comparing
struct_from_display DataFromDisplay;
struct_from_display PrevDataFromDisplay;


int16_t CalibrateZeroPos = 0;

int16_t ns_cal_received;
int16_t ew_cal_received;
float D_StationGpsNS;
float D_StationGpsEW;
float D_GeoLocationGpsNS;
float D_GeoLocationGpsEW;

