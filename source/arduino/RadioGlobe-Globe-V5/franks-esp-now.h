#pragma once

#define MAX_STATIONS 50
#define MAX_FAVORITES 4

#define QUEUESIZE 50
#define QUEUEMESSAGELENGTH 128
typedef struct {               
  char     QueueMessage[QUEUESIZE][QUEUEMESSAGELENGTH];  // 50 strings of 128 each (6400 bytes) 
  uint16_t QueueMessageType[QUEUESIZE];
  uint16_t QueueIndexIn;
  uint16_t QueueIndexOut;
  uint16_t QueueCnt;
} Queue;

#define APIQUEUESIZE 25
typedef struct {               
  uint16_t ApiType[APIQUEUESIZE];
  float ApiParameterNS[APIQUEUESIZE];
  float ApiParameterEW[APIQUEUESIZE];
  uint16_t ApiRequestedStation[APIQUEUESIZE];
  uint16_t ApiQueueIndexIn;
  uint16_t ApiQueueIndexOut;
  uint16_t ApiQueueCnt;
} ApiQueue;

ApiQueue ApiCallsToDo; // queue with apis to call

const char * messagetexts[] = {
   { "UNSUPPORTED MESSAGE"},
   { "SONG_TITLE 1"},
   { "STATUS_LINE 2"},
   { "GLOBE UPDATE AVAILABLE 3"},
   { "NOP"},
   { "STATION_NAME 5"},
   { "DESCRIPTION 6"},
   { "OPEN_WEATHER_MAP_API_KEY 7"},
   { "GOOGLE_API_KEY 8"},
   { "TIMEZONE_ID 9"},
   { "FREE_TO_USE2 10"},
   { "THIS_IS_HOME 11"},
   { "EX_CHANGE_RATE 12"},
   { "SSID_FOR_GLOBE 13"},
   { "PASSWORD_FOR_GLOBE 14"},
   { "WIFI_STATUS 15"},
   { "CALIBRATE_ZERO 16"},
   { "FINDNEWSTATION 17"},
   { "GLOBE_MAC 18"},
   { "GLOBE_IP 19"},
   { "MESSAGE_FREE_TO_USE 20"},
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
   { "STATION_CONNECTED_IN_MS 37"},
   { "INTERNAL_SPEAKER_TOGGLE 38"},
   { "GET_WEATHER_DATA_BY_GPS 39"},
   { "GET_GEOLOCATION_BY_GPS 40"},
   { "CONNECTTOHOST_FAILURE 41"},
   { "STATION_WEATHER_DATA 42"},
   { "INTERNAL_SPEAKER_ON 43"},
   { "INTERNAL_SPEAKER_OFF 44"},
   { "GLOBE_BUILD_DATE_TIME 45"},
   { "RADIO PRESET 46"},
   { "WEATHER_API_KEY 47"},
   { "MQTT_STATUS 48"},
   { "DISPLAY_SERIALNUMBER 49"},
   { "MESSAGE_DISPLAY_BT_SWITCHABLE_STATE 50"},
   { "OPEN PORTAL 51"},
   { "GLOBE CLOSED PORTAL 52"},
   { "PUCK_ESP_NOW_INIT 53"},
   { "SET_PUCK_WIFI_CHANNEL 54"},
   { "GLOBE_SD_GB 55"},
   { "GLOBE_PLAY_SD 56"},
   { "GET_GEOLOCATION 57"},
   { "GET_WEATHER_DATA 58"},
   { "MUSIC_MODE 59"},
   { "PLAYLIST_SONG_ARTIST 60"},
   { "START_THIS_FILE 61"},
   { "RELOAD_SD_WITH_COUNTRY 62"},
   { "MESSAGE_START_FILE_BY_INDEX 63"},
   { "MESSAGE_SET_ROLLER_INDEX 64"},
   { "MESSAGE_UPDATE_GLOBE 65"},
   { "MESSAGE_UPDATE_PUCK 66"},
   { "MESSAGE_GLOBE_HOSTNAME 67"},
   { "MESSAGE_SILENT_POWER_DOWN 68"}, 
   { "MESSAGE_MAX 69"}
 };


#define MESSAGE_SONG_TITLE 1
#define MESSAGE_STATUS_LINE 2
#define MESSAGE_GLOBE_UPDATE_AVAILABLE 3
#define MESSAGE_NOP 4
#define MESSAGE_STATION_NAME 5
#define MESSAGE_DESCRIPTION 6
#define MESSAGE_OPEN_WEATHER_MAP_API_KEY 7
#define MESSAGE_GOOGLE_API_KEY 8
#define MESSAGE_TIMEZONE_ID 9
#define MESSAGE_FREE_TO_USE2 10
#define MESSAGE_THIS_IS_HOME 11
#define MESSAGE_EX_CHANGE_RATE 12
#define MESSAGE_SSID_FOR_GLOBE 13
#define MESSAGE_PASSWORD_FOR_GLOBE 14
#define MESSAGE_NO_WIFI 15
#define MESSAGE_CALIBRATE_ZERO 16
#define MESSAGE_FINDNEWSTATION 17
#define MESSAGE_GLOBE_MAC 18
#define MESSAGE_GLOBE_IP 19
#define MESSAGE_FREE_TO_USE 20 // can be used when needed
#define MESSAGE_START_THIS_STATION 21 // as a result from a roller selection made
#define MESSAGE_PAUSE_RADIO 22
#define MESSAGE_RESUME_RADIO 23
#define MESSAGE_WANT_NEXT_STATION 24
#define MESSAGE_STATION_CONNECTED 25
// when globe powers up by remote control
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
#define MESSAGE_STATION_CONNECTED_IN_MS 37
#define MESSAGE_INTERNAL_SPEAKER_TOGGLE 38
#define MESSAGE_GET_WEATHER_DATA_BY_GPS 39
#define MESSAGE_GET_GEOLOCATION_BY_GPS 40
#define MESSAGE_CONNECTTOHOST_FAILURE 41

#define MESSAGE_STATION_WEATHER_DATA 42
#define MESSAGE_INTERNAL_SPEAKER_ON 43
#define MESSAGE_INTERNAL_SPEAKER_OFF 44
#define MESSAGE_GLOBE_BUILD_DATE_TIME 45
#define MESSAGE_STATION_PRESET 46
#define MESSAGE_WEATHER_API_KEY 47
#define MESSAGE_MQTT_STATUS 48
#define MESSAGE_DISPLAY_SERIALNUMBER 49
#define MESSAGE_DISPLAY_BT_SWITCHABLE_STATE 50
#define MESSAGE_OPEN_PORTAL 51
#define MESSAGE_GLOBE_CLOSED_PORTAL 52
#define MESSAGE_PUCK_ESP_NOW_INIT 53
#define MESSAGE_SET_PUCK_WIFI_CHANNEL 54
#define MESSAGE_GLOBE_SD_GB 55
#define MESSAGE_GLOBE_PLAY_SD 56
#define MESSAGE_GET_GEOLOCATION 57
#define MESSAGE_GET_WEATHER_DATA 58
#define MESSAGE_MUSIC_MODE 59
#define MESSAGE_PLAYLIST_SONG_ARTIST 60
#define MESSAGE_START_THIS_FILE 61
#define MESSAGE_RELOAD_SD_WITH_COUNTRY 62
#define MESSAGE_START_FILE_BY_INDEX 63
#define MESSAGE_SET_ROLLER_INDEX 64
#define MESSAGE_UPDATE_GLOBE 65
#define MESSAGE_UPDATE_PUCK 66
#define MESSAGE_GLOBE_HOSTNAME 67
#define MESSAGE_SILENT_POWER_DOWN 68
#define MESSAGE_MAX 69

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
    uint16_t internalspeakeron; // speakers on or off
    uint16_t D_BatteryVoltage;
    uint16_t btmodule_power_on; // bluetooth on or off
    int16_t  D_RequestedStation;   
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
    int16_t  D_QueueStationIndex; // echo back unaltered
    char     G_QueueMessage[QUEUEMESSAGELENGTH];
    uint16_t D_QueueSerialNumber; // echo back to display confirm reception
    uint16_t D_QueueMessageType;
    uint16_t D_QueueMessageCount; // number of message in queue
    uint16_t G_QueueBytesStreamed; // can be used to monitor actual streaming of data
    uint32_t G_QueueStreamIdleMs; // copier idle time
    uint16_t G_Volume;
    bool     G_EncoderReliable;
    int16_t  G_rssi_globe;
    int16_t  D_ApisFetchedForStation;   


} struct_from_globe;




// Declare a structurs named DataFromGlobe to hold various variables
struct_from_globe DataFromGlobe;
struct_from_globe PrevDataFromGlobe;


// Declare two structures named DataFromDisplay for storing/comparing
struct_from_display DataFromDisplay;
struct_from_display PrevDataFromDisplay;

Queue ToDisplay; // queue with messages for display
Queue FromDisplay; // queue with messages from display


int16_t CalibrateZeroPos = 0;

int16_t TZ_NS;
int16_t TZ_EW;
int16_t TZ_RequestedStation = 9999;

float D_GeoStationNS;
float D_GeoStationEW;
float D_GeoLocationNS;
float D_GeoLocationEW;
int16_t D_RequestedStation = 9999;


