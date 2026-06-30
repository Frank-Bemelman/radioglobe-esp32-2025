#pragma once

#define QUEUESIZE 40
#define QUEUEMESSAGELENGTH 128
typedef struct {               
  EXT_RAM_ATTR char     QueueMessage[QUEUESIZE][QUEUEMESSAGELENGTH];  // 50 strings of 128 each (6400 bytes) 
  EXT_RAM_ATTR uint16_t QueueMessageType[QUEUESIZE];
  EXT_RAM_ATTR uint16_t QueueIndexIn;
  EXT_RAM_ATTR uint16_t QueueIndexOut;
  EXT_RAM_ATTR uint16_t QueueCnt;
} Queue;


const char * messagetexts[] = {
   { "UNSUPPORTED MESSAGE"},
   { "SONG_TITLE 1"},
   { "ARTIST 2"},
   { "ALBUM 3"},
   { "GENRE 4"},
   { "STATION_NAME 5"},
   { "DESCRIPTION 6"},
   { "OPEN_WEATHER_MAP_API_KEY 7"},
   { "GOOGLE_API_KEY 8"},
   { "TIMEZONE_ID 9"},
   { "TIMEZONE_NAME 10"},
   { "THIS_IS_HOME 11"},
   { "EX_CHANGE_RATE 12"},
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
   { "MESSAGE_STATION_CONNECTED_IN_MS 37"},
   { "INTERNAL_SPEAKER_TOGGLE 38"},
   { "GET_WEATHER_DATA_BY_GPS 39"},
   { "GET_GEOLOCATION_BY_GPS 40"},
   { "CONNECTTOHOST_FAILURE 41"},
   { "STATION_WEATHER_DATA 42"},
   { "INTERNAL_SPEAKER_ON 43"},
   { "INTERNAL_SPEAKER_OFF 44"},
   { "GLOBE BUILD DATE & TIME 45"},
   { "RADIO PRESET 46"},
   { "WEATHER_API_KEY 47"},
   { "MQTT_STATUS 48"},
   { "DISPLAY_SERIALNUMBER 49"},
   { "DISPLAY_BT_SWITCHABLE_STATE 50"},
   { "OPEN_PORTAL 51"},
   { "GLOBE_CLOSED_PORTAL 52"},
   { "PUCK_ESP_NOW_INIT 53"},
   { "SET_PUCK_WIFI_CHANNEL 54"},
   { "GLOBE_HAS_SD 55"},
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
   { "MESSAGE_MAX 67"}
};

#define MESSAGE_SONG_TITLE 1
#define MESSAGE_ARTIST 2
#define MESSAGE_ALBUM 3
#define MESSAGE_GENRE 4
#define MESSAGE_STATION_NAME 5
#define MESSAGE_DESCRIPTION 6
#define MESSAGE_OPEN_WEATHER_MAP_API_KEY 7
#define MESSAGE_GOOGLE_API_KEY 8
#define MESSAGE_TIMEZONE_ID 9
#define MESSAGE_TIMEZONE_NAME 10
#define MESSAGE_THIS_IS_HOME 11
#define MESSAGE_EX_CHANGE_RATE 12
#define MESSAGE_SSID_FOR_GLOBE 13
#define MESSAGE_PASSWORD_FOR_GLOBE 14
#define MESSAGE_WIFI_STATUS 15
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

#define MESSAGE_GLOBE_WANTS_CURRENT_STATION 26 // when globe reboots

// request from display to get a certain timezone
#define MESSAGE_GET_TIMEZONE_BY_GPS 27

#define MESSAGE_STREAMING_IDLE_MS 28

#define MESSAGE_NEW_LIST_LOADED 29 // after a succesful search

#define MESSAGE_GET_TIMEZONE 30

#define MESSAGE_POWERDOWN 31
#define MESSAGE_POWERUP 32

#define MESSAGE_VOLUME_AND_TONE 33
#define MESSAGE_DISPLAY_WANTS_VOLUME_AND_TONE 34
#define MESSAGE_STORE_VOLUME_AND_TONE 35
#define MESSAGE_AUDIO_EOF_STREAM 36
#define MESSAGE_STATION_CONNECTED_IN_MS 37
#define MESSAGE_INTERNAL_SPEAKER_TOGGLE 38


#define MESSAGE_GET_GEOLOCATION_BY_GPS 40
#define MESSAGE_CONNECTTOHOST_FAILURE 41

#define MESSAGE_STATION_WEATHER_DATA 42
#define MESSAGE_INTERNAL_SPEAKER_ON 43
#define MESSAGE_INTERNAL_SPEAKER_OFF 44
#define MESSAGE_GLOBE_BUILD_DATE_TIME 45
#define MESSAGE_RADIO_PRESET 46
#define MESSAGE_WEATHER_API_KEY 47
#define MESSAGE_MQTT_STATUS 48
#define MESSAGE_DISPLAY_SERIALNUMBER 49
#define MESSAGE_DISPLAY_BT_SWITCHABLE_STATE 50
#define MESSAGE_OPEN_PORTAL 51
#define MESSAGE_GLOBE_CLOSED_PORTAL 52
#define MESSAGE_PUCK_ESP_NOW_INIT 53
#define MESSAGE_SET_PUCK_WIFI_CHANNEL 54
#define MESSAGE_GLOBE_HAS_SD 55
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
#define MESSAGE_MAX 67

typedef struct struct_message1 {
    int16_t  ns_cal;
    int16_t  ew_cal;
    uint16_t volumevalue;
    uint16_t bassvalue;
    int16_t  treblevalue;
    int16_t  CalibrateZeroPos;
    uint16_t D_QueueSerialNumber;
    uint16_t D_QueueMessageType;
    uint16_t D_QueueStationIndex; // echo back unaltered
    char     D_QueueMessage[QUEUEMESSAGELENGTH];
    uint16_t G_QueueSerialNumber; // echo back to globe confirm reception
    float    D_StationGpsNS; // used by globe to get timezone
    float    D_StationGpsEW; // used by globe to get timezone
    uint16_t internalspeakeron;
    uint16_t D_BatteryVoltage;
    uint16_t btmodule_power_on;
    
    
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
    uint16_t D_QueueStationIndex; // station connected -1 if idle
    char     G_QueueMessage[QUEUEMESSAGELENGTH];
    uint16_t D_QueueSerialNumber; // echo back to display confirm reception
    uint16_t D_QueueMessageType;
    uint16_t D_QueueMessageCount; // number of message in queue
    uint16_t G_QueueBytesStreamed; // can be used to monitor actual streaming of data
    uint32_t G_QueueStreamIdleMs; // copier idle time
    uint16_t G_Volume;
    bool     G_EncoderReliable;
    int16_t  G_rssi_globe;


} struct_from_globe;





// Declare a two structures storing/comparing outgoing variables from display
struct_from_display DataFromDisplay;
struct_from_display PrevDataFromDisplay;
struct_from_display DataForLCD;

// Declare two structures for storing/comparing incoming variables from globe
struct_from_globe DataFromGlobe;
struct_from_globe PrevDataFromGlobe;

EXT_RAM_ATTR Queue ToGlobe; // queue with messages for globe
EXT_RAM_ATTR Queue FromGlobe; // queue with messages from globe

// define your home location to set initial time zone
#define HOME_GPS_NS 52.2540183
#define HOME_GPS_EW 4.5419280
