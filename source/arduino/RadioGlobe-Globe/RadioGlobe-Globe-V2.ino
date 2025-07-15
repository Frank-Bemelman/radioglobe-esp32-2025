
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClient.h>
#include <VS1053.h>               // https://github.com/baldram/ESP_VS1053_Library
#include <ESP32_VS1053_Stream.h>  // https://github.com/CelliesProjects/ESP32_VS1053_Stream
// changed in ESP32_VS1053_Stream.h around line 15-16
// the original lower values often give connection refused for more remote locations
// #define VS1053_CONNECT_TIMEOUT_MS 10000 // FB was 250 FB
// #define VS1053_CONNECT_TIMEOUT_MS_SSL 10000 // FB was 750
#include "franks-esp-now.h"
#include "mp3_sound_winxpstart.h"
#include "mp3_sound_winxpshutdown.h"
#include "..\secrets.h"
// as defined in ..\secrets.h
// char SSID [32] = "YOUR-SSID";
// char PASSWORD[32] = "YOUR-WIFI-PASSWORD";
// char google_api_key[] = "YOUR-API-KEY"; // free, get your own at google developer platform, used for timezone retrieval
#include "AS5600.h"
#include "Wire.h"

#define COPY_LOG_OFF

//hardware vs1053 board
// SCK pin 18
// (MI)SO pin 19
// (MO)SI pin 23
// (X)CS pin 5
// (X)DCS pin21
// DREQ pin 22
// (X)RST pin 15

// VS1053 sound module esp32-wroom-32 pin arrangement
#define SPI_MOSI_PIN 23 // white
#define SPI_CLK_PIN 18  // green
#define SPI_MISO_PIN 19 // orange

#define  VS1053_CS 5 // grey
#define  VS1053_DREQ 22 // yellow 
#define  VS1053_DCS 21 // blue
#define VS1053_RESET 15 // brown

// VS1053 sound module esp32-S3-N16R8 pin arrangement
//#define SPI_MOSI_PIN 35 // white
//#define SPI_CLK_PIN 36  // green
//#define SPI_MISO_PIN 37 // orange

//#define  VS1053_CS 1 // grey
//#define  VS1053_DREQ 2 // yellow 
//#define  VS1053_DCS 42 // blue
//#define VS1053_RESET 41 // brown

// VS1053 sound module esp32-wrover-8 pin arrangement
//#define SPI_MOSI_PIN 23 // white
//#define SPI_CLK_PIN 18  // green
//#define SPI_MISO_PIN 19 // orange

//#define  VS1053_CS 13 // grey
//#define  VS1053_DREQ 4 // yellow 
//#define  VS1053_DCS 21 // blue
//#define VS1053_RESET 22 // brown



ESP32_VS1053_Stream stream;
VS1053 chunkplayer(VS1053_CS, VS1053_DCS, VS1053_DREQ);

// AS5600 encoders esp32-wroom-32 pin arrangement
const int SCL_1 = 33; // &wire as5600_0 grey
const int SDA_1 = 32; // &wire as5600_0 brown
const int SCL_2 = 26; // &wire as5600_1 grey
const int SDA_2 = 25; // &wire as5600_1 brown

// AS5600 encoders esp32-S3-N16R8 pin arrangement
//const int SCL_1 = 4; // &wire as5600_0 grey
//const int SDA_1 = 5; // &wire as5600_0 brown
//const int SCL_2 = 6; // &wire as5600_1 grey
//const int SDA_2 = 7; // &wire as5600_1 brown

// AS5600 encoders esp32-wrover-8 pin arrangement
//const int SCL_1 = 32; // &wire as5600_0 grey
//const int SDA_1 = 33; // &wire as5600_0 brown
//const int SCL_2 = 25; // &wire as5600_1 grey
//const int SDA_2 = 26; // &wire as5600_1 brown


AS5600 as5600_1(&Wire);
AS5600 as5600_2(&Wire1);

static uint32_t startMillis;
static uint32_t lapMillis;
static uint32_t currentMillis;

int16_t ReadEncoderTicker100mS = 0;
int16_t PrevTick = 0;
bool bPowerStatus = true;
bool bVolumeToneControlsActive = false;
int16_t stream_connecttohost_result; // received from callback 

bool bUpAndRunning = false;
bool bEncoderNewPosition;
int16_t NSDegLive10; // unfiltered, uncalibrated NS degrees times 10
int16_t EWDegLive10; // unfiltered, uncalibrated EW degrees times 10
int16_t NSDeg10; // uncalibrated NS degrees times 10
int16_t EWDeg10; // uncalibrated EW degrees times 10
int16_t gNSCal10; // calibrated NS degrees times 10
int16_t gEWCal10; // calibrated EW degrees times 10


char RequestedUrl[QUEUEMESSAGELENGTH]; // holds the url requested by display
char ActiveUrl[QUEUEMESSAGELENGTH]; // holds the url requested by display
char UnraveledUrl[2048] = ""; // checked for redirects 
char ConnectedUrl[2048] = ""; // actual url that connected with succes

bool Tuning = false;

#include <EEPROM.h>
#define EEPROM_SIZE 256
struct eepromData {
int16_t Offset1;
int16_t Offset2;
uint16_t ee_volume; // 0-100 can be changed by display puck
uint16_t ee_bass; // 0-f can be changed by display puck
uint16_t ee_treble; // 0-f can be changed by display puck
char ssid[32];
char password[32];
char google_api_key[64];
char spare_data[64];
};
eepromData GlobeSettings; 

uint8_t rtone[4] = {0, 3, 14, 0}; // tone control register of VS1053 responding to display controls for bass & treble, values of 0-15
uint8_t mac[6];


void setup()
{ char message[64];
  startMillis = millis();
  Serial.begin(115200);

  //pinMode(VS1053_RESET, OUTPUT);
  //digitalWrite(VS1053_RESET, LOW);  // sets the digital pin 12 off
  delay(100);            // waits for a second
  //digitalWrite(VS1053_RESET, HIGH); // sets the digital pin 12 on
  delay(100);            // waits for a second

  // Initialise as5600_0 Connection
  Wire.begin(SDA_1, SCL_1);
  Wire1.begin(SDA_2, SCL_2);

  EEPROM.begin(EEPROM_SIZE);

  // calibrate_globe(); // call just once to force during construction, set globe to NS0 and EW0 - can also be done through display calibration menu

  EEPROM.get(0x0, GlobeSettings);
  GlobeSettings.ssid[sizeof(GlobeSettings.ssid)-1]=0;
  Serial.printf("Eeprom ssid: %s\n", GlobeSettings.ssid);
  GlobeSettings.password[sizeof(GlobeSettings.password)-1]=0;
  Serial.printf("Eeprom password: %s\n", GlobeSettings.password);

  Serial.printf("Eeprom stored 1: %d\n", GlobeSettings.Offset1);
  Serial.printf("Eeprom stored 2: %d\n", GlobeSettings.Offset2);

  as5600_1.begin();  //  set direction pin.
  as5600_1.setDirection(AS5600_CLOCK_WISE);
  as5600_1.setOffset(GlobeSettings.Offset1 * AS5600_RAW_TO_DEGREES);
  Serial.print("Connect device 0: ");
  Serial.println(as5600_1.isConnected() ? "true" : "false");

  as5600_2.begin();  //  set direction pin.
  as5600_2.setDirection(AS5600_CLOCK_WISE);
  as5600_2.setOffset(GlobeSettings.Offset2 * AS5600_RAW_TO_DEGREES);  
  Serial.print("Connect device 1: ");
  Serial.println(as5600_2.isConnected() ? "true" : "false");

  Serial.print("readangle NS = "); Serial.println(as5600_1.readAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("readangle EW = "); Serial.println(as5600_2.readAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("ReadAGC NS = "); Serial.println(as5600_1.readAGC());
  Serial.print("ReadAGC EW = "); Serial.println(as5600_2.readAGC());

  delay(500);

  DataFromGlobe.D_QueueStationIndex = -1;

  bUpAndRunning = true;
  
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
    
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  //WiFi.setSleep(false);
  
  uint16_t timeout = 100; // 10 seconds max
  Serial.print("Connecting to WiFi ..");
  while (!WiFi.isConnected() && timeout--) 
  { Serial.print('.');
    delay(100);
  }
  
  if(WiFi.isConnected())
  { Serial.println(" wifi connected.");
    AddToQueueForDisplay(WiFi.localIP().toString().c_str(), MESSAGE_GLOBE_IP);
    Serial.println(WiFi.localIP());
    Serial.print("Mac Adress Globe: ");
    WiFi.macAddress(mac);
    sprintf(message, "%2X:%2X:%2X:%2X:%2X:%2X", (uint16_t)mac[0], (uint16_t)mac[1], (uint16_t)mac[2], (uint16_t)mac[3], (uint16_t)mac[4], (uint16_t)mac[5]);
    AddToQueueForDisplay(message, MESSAGE_GLOBE_MAC);
    Serial.print("Mac Adress Globe: ");
    Serial.println(message);
  }
  else
  { Serial.println(" wifi NOT connected.");
    AddToQueueForDisplay(WiFi.localIP().toString().c_str(), MESSAGE_NO_WIFI);
  }

  setup_esp_now();
  setup_tasks(); // see task-and-interrupts.ino, will also indirectly start to use espnow

  //Start SPI bus
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  Serial.println("spi bus started.");

  // Initialize the VS1053 decoder
  if (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !stream.isChipConnected()) 
  { Serial.println("Decoder not running - system halted");
    while (1) delay(100);
  }
  Serial.println("VS1053 up & running");


  // tell display what our stored values are for volume and tone control
  sprintf(message, "%d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble);
  AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);

  // set VS1053 volume and tone values from eeprom
  Serial.printf("Volume from eeprom -> %d\n", GlobeSettings.ee_volume);
  SetVolumeMapped(GlobeSettings.ee_volume); // eeprom can be updated by changing bass or treble
  Serial.printf("Bass value from eeprom -> %d\n", GlobeSettings.ee_bass);
  rtone[2] = (GlobeSettings.ee_bass * 15)/100;
  Serial.printf("Treble value from eeprom -> %d\n", GlobeSettings.ee_treble);
  rtone[0] = (uint8_t) abs(((GlobeSettings.ee_treble-50)*15/100)); 
  //  Serial.printf("Nibble1 %04x\n", (uint16_t) rtone[0] );                           
  if((GlobeSettings.ee_treble-50)<0)rtone[0] |= 0x08;
  //  Serial.printf("Nibble2 %04x\n", (uint16_t) rtone[0] );                           
  stream.setTone(rtone); 

  // windows tune
  //  VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
  if(chunkplayer.getChipVersion() == 4) 
  { // Only perform an update if we really are using a VS1053, not. eg. VS1003
    chunkplayer.loadDefaultVs1053Patches(); 
  }
  chunkplayer.playChunk((uint8_t *)mp3_winxpstart, sizeof(mp3_winxpstart));

  AddToQueueForDisplay("Globe Just Booted", MESSAGE_GLOBE_WANTS_CURRENT_STATION);
  
  
}


static bool once = true;
void loop()
{ char message[QUEUEMESSAGELENGTH];
  float GpsNS;
  float GpsEW;

  stream.loop();

     // process messages from display
  if(DataFromGlobe.D_QueueSerialNumber != DataFromDisplay.D_QueueSerialNumber) // not yet acknowlegded and processed
  {  // quick acknowledge message from displayglobe as received
     DataFromGlobe.D_QueueSerialNumber = DataFromDisplay.D_QueueSerialNumber;
     if((DataFromDisplay.D_QueueMessageType>=0) && (DataFromDisplay.D_QueueMessageType<sizeof(messagetexts)))
     { Serial.printf("DISPLAY SAYS: %s >%s<\n", messagetexts[DataFromDisplay.D_QueueMessageType], DataFromDisplay.D_QueueMessage);  
     }
     // and now take care of it
    switch(DataFromDisplay.D_QueueMessageType)
    { case MESSAGE_GOOGLE_API_KEY:
        Serial.println(DataFromDisplay.D_QueueMessage);
        strcpy(google_api_key, DataFromDisplay.D_QueueMessage);
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        break;
      case MESSAGE_CALIBRATE_ZERO:
        if(strcmp(DataFromDisplay.D_QueueMessage, "1234")==0)
        { calibrate_globe();
        }
        break;  
      case MESSAGE_START_THIS_STATION:
        strcpy(RequestedUrl, DataFromDisplay.D_QueueMessage);

        // will be unraveled later, redirected, derived from pls or m3u file
        strcpy(UnraveledUrl, RequestedUrl);

        if(DataFromGlobe.D_QueueStationIndex == DataFromDisplay.D_QueueStationIndex)
        { // already conmected
          Serial.printf("ALREADY CONNECTED TO STATIONINDEX %d\n", DataFromGlobe.D_QueueStationIndex);  
          AddToQueueForDisplay(RequestedUrl, MESSAGE_STATION_CONNECTED);
          break;
        }
        
        Tuning = true;

        // quick & dirty, but too dirty, as not all https urls are http appraocheable (HTTP error 400)
        // or catch the 400 for a retry on https
        // if(UnraveledUrl[4]=='s')strcpy(&UnraveledUrl[4], &UnraveledUrl[5]);  
        
        if(StartNewStation()==1) // succes
        { // restore volume setting
          SetVolumeMapped(DataFromDisplay.volumevalue);
          strcpy(ActiveUrl, RequestedUrl);
          DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
          AddToQueueForDisplay(ActiveUrl, MESSAGE_STATION_CONNECTED); 
        }
        else
        {  DataFromGlobe.D_QueueStationIndex = -1;
           Serial.printf("FAILED: stream.connecttohost \n");
           sprintf(message, "%d", stream_connecttohost_result);
           AddToQueueForDisplay(message, MESSAGE_CONNECTTOHOST_FAILURE);
//           AddToQueueForDisplay(RequestedUrl, MESSAGE_DEAD_STATION);
           AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
        }

/*
        lapMillis = millis(); 
        DataFromGlobe.Unraveling = 1;
        while(DataFromGlobe.Unraveling)
        { delay(10);
          //Serial.print(".");
          stream.loop();
        }
        Serial.printf("Unraveling finished -> time elapsed = %ld\n", (currentMillis = millis()) - lapMillis);
   

        if(DataFromGlobe.UnravelingResult==1)
        { //Serial.printf("MESSAGE_START_NEW_STATION: >%s<\n", DataFromDisplay.D_QueueMessage);  
          StartNewStation();
        }  
        else 
        { Serial.printf("Url: >%s< rejected by globe.\n", DataFromDisplay.D_QueueMessage);  
          AddToQueueForDisplay("Globe wants next station.", MESSAGE_WANT_NEXT_STATION);
        }
*/
        Tuning = false;

        break;
      case MESSAGE_SSID_FOR_GLOBE:
        if(strcmp(GlobeSettings.ssid , DataFromDisplay.D_QueueMessage)!=0)
        { strcpy(GlobeSettings.ssid , DataFromDisplay.D_QueueMessage);
          EEPROM.put(0x0, GlobeSettings);
          EEPROM.commit();
        }  
        break;
      case MESSAGE_PASSWORD_FOR_GLOBE:
        if(strcmp(GlobeSettings.password , DataFromDisplay.D_QueueMessage)!=0)
        { strcpy(GlobeSettings.password , DataFromDisplay.D_QueueMessage);
          EEPROM.put(0x0, GlobeSettings);
          EEPROM.commit();
        }  
        break;

      // displays wants to know the timezone for a given GPS location as linked to a particular station from the database
      // actual handling moved to esp-now receive, this here is just to inform
      case MESSAGE_GET_TIMEZONE_BY_GPS:
        sscanf(DataFromDisplay.D_QueueMessage, "%f-%f", &GpsNS, &GpsEW);
        //Serial.printf("  (DISPLAY SAYS): NS%f EW%f\n", GpsNS, GpsEW );  
        break;

      // displays wants to know the timezone for a given GPS location from the calibrated globe coordinates
      // actual handling moved to esp-now receive, this here is just to inform
      case MESSAGE_GET_TIMEZONE:
        //Serial.printf("  (DISPLAY SAYS): %s\n", DataFromDisplay.D_QueueMessage);  
        break;

      case MESSAGE_NEW_LIST_LOADED:
        // reset this value because it is not related anymore to the new list of stations found by display
        DataFromGlobe.D_QueueStationIndex = -1;
        break;

      case MESSAGE_POWERDOWN:
        bPowerStatus = false;
        stream.stopSong();
        DataFromGlobe.D_QueueStationIndex = -1;
        while(stream.isRunning())stream.loop(); // empty buffer

        delay(1000);
        // play end tune at normal level
        EEPROM.get(0x0, GlobeSettings);
        Serial.printf("POWERDOWN - volume -> %d \n", GlobeSettings.ee_volume);
        SetVolumeMapped(GlobeSettings.ee_volume); 

        chunkplayer.playChunk((uint8_t *)mp3_winxpshutdown, sizeof(mp3_winxpshutdown));

        // silence now
        SetVolumeMapped(0); 
        Serial.printf("POWERDOWN done\n");
        break;

      case MESSAGE_POWERUP:
        bPowerStatus = true;
        EEPROM.get(0x0, GlobeSettings);
        Serial.printf("POWERUP - volume -> %d \n", GlobeSettings.ee_volume);
         // tell display what our stored values are for volume and tone control
        sprintf(message, "%d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble);
        AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
        SetVolumeMapped(GlobeSettings.ee_volume); 
        AddToQueueForDisplay("Globe Just Booted", MESSAGE_GLOBE_WANTS_CURRENT_STATION);
        // play windows tune 
        chunkplayer.playChunk((uint8_t *)mp3_winxpstart, sizeof(mp3_winxpstart));
        Serial.printf("POWERUP done\n");
        break;

      case MESSAGE_VOLUME_AND_TONE:
        bVolumeToneControlsActive = true;
        break;

      case MESSAGE_DISPLAY_WANTS_VOLUME_AND_TONE:
        EEPROM.get(0x0, GlobeSettings);
        sprintf(message, "%d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble);
        AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
        break;

      case MESSAGE_STORE_VOLUME_AND_TONE:
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        Serial.println("Eeprom saved..");
        break;

      case MESSAGE_GET_GEOLOCATION_BY_GPS:
        sscanf(DataFromDisplay.D_QueueMessage, "%f-%f", &GpsNS, &GpsEW);
        Serial.printf("INCOMING MESSAGE_GET_GEOLOCATION_BY_GPS: NS%f EW%f\n", GpsNS, GpsEW);  
        break;

      default:
        Serial.printf("Unsupported message type %d from display: >%s<\n", DataFromDisplay.D_QueueMessageType, DataFromDisplay.D_QueueMessage);  
        break;
    }
  }

  if(bEncoderNewPosition)
  { bEncoderNewPosition = false;
    AddToQueueForDisplay("?", MESSAGE_FINDNEWSTATION);
  }
  
  // volume and tone levels checking
  if(bVolumeToneControlsActive == true)
  { if(PrevTick != ReadEncoderTicker100mS)
    { PrevTick = ReadEncoderTicker100mS;
      if(PrevDataFromDisplay.volumevalue != DataFromDisplay.volumevalue)
      { PrevDataFromDisplay.volumevalue = DataFromDisplay.volumevalue;
        SetVolumeMapped(DataFromDisplay.volumevalue);
        if(bPowerStatus == true)
        { GlobeSettings.ee_volume = DataFromDisplay.volumevalue;
        }  
      }

      if(PrevDataFromDisplay.bassvalue != DataFromDisplay.bassvalue)
      { PrevDataFromDisplay.bassvalue = DataFromDisplay.bassvalue;
        // bass usuable values is rtone[3] = 14 for lower frequency limit and rtone[2] = 0-15 for level control    
        rtone[3] = 14;
        rtone[2] = (DataFromDisplay.bassvalue * 15)/100;
        stream.setTone(rtone); 
        GlobeSettings.ee_bass = DataFromDisplay.bassvalue;
        GlobeSettings.ee_volume = DataFromDisplay.volumevalue;
        // Serial.printf("Bass=level %x cutoff %x\n", (uint16_t)rtone[2], (uint16_t)rtone[3]);
      }

      if(PrevDataFromDisplay.treblevalue != DataFromDisplay.treblevalue)
      { PrevDataFromDisplay.treblevalue = DataFromDisplay.treblevalue;
        // treble usuable values is rtone[1] = 3 for higher frequency limit and rtone[0] = 0-15 for level control
        // convert from 0-100 to signed nibble f-e-d-c-b-a-9-0-1-2-3-4-5-6-7
        rtone[1] = 3;
        rtone[0] = (uint8_t) abs(((DataFromDisplay.treblevalue-50)*15/100)); 
        //  Serial.printf("Nibble1 %04x\n", (uint16_t) rtone[0] );                           
        if((DataFromDisplay.treblevalue-50)<0)rtone[0] |= 0x08;
        //  Serial.printf("Nibble2 %04x\n", (uint16_t) rtone[0] );                           
        stream.setTone(rtone); 
        GlobeSettings.ee_treble = DataFromDisplay.treblevalue;
        GlobeSettings.ee_volume = DataFromDisplay.volumevalue;
        // Serial.printf("Treble=level %x cutoff %x\n", (uint16_t)rtone[0], (uint16_t)rtone[1]);
      }
    }



    if(CalibrateZeroPos == 1234) // calibration started by display 
    { calibrate_globe();
      CalibrateZeroPos = 0;
    }
  }  

  delay(5);
  
}





bool StartNewStation(void)
{ bool return_result = false;
  char message[QUEUEMESSAGELENGTH];

  Tuning = true;
  Serial.printf("StartNewStation with %s\n", RequestedUrl);
  stream.setVolume(0);
  Serial.printf("First stop this one: %s\n", ConnectedUrl);
  stream.stopSong();
  DataFromGlobe.D_QueueStationIndex = -1;
  strcpy(ActiveUrl, "");
  //AddToQueueForDisplay("", MESSAGE_NAME); // already filled in by display station search, perhaps to be updated by actual station connecting
  AddToQueueForDisplay("", MESSAGE_SONG_TITLE); // remove 'now playing title'

  lapMillis = millis(); 

//  interesting - these url play fine in chrome browser, but not here, unless you change https to http
//  strcpy(UnraveledUrl, "https://cosmo.shoutca.st/proxy/jubileefm/stream"); 
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/89fkq77gb4duv");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/7ns6amt68qruv");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/yerp85sughwtv");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/bux0vqx79zquv");
//  strcpy(UnraveledUrl, "https://www.radioking.com/play/radio-paysan-fm-san/652837");
//  strcpy(UnraveledUrl, "https://betelgeuse.dribbcast.com/proxy/diaremefm?mp=/stream");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/ElBahdja_64K.mp3");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/Illizi_64K.mp3");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/Ouargla_64K.mp3");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/hfbgmx6rwrhvv");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/Chaine3_64K.mp3");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/Jeunesse_64K.mp3");
//  strcpy(UnraveledUrl, "https://radio-dzair.net/proxy/chaabia/chaabia");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/ElBahdja_64K.mp3");
//  strcpy(UnraveledUrl, "https://webradio.tda.dz/Coran_64K.mp3");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/1dkectoxpqgvv");
//  strcpy(UnraveledUrl, "https://betelgeuse.dribbcast.com/proxy/mamediarra?mp=/stream");
//  strcpy(UnraveledUrl, "https://playerservices.streamtheworld.com/api/livestream-redirect/SP_R2982692.aac");
//  strcpy(UnraveledUrl, "https://live.paineldj.com.br/proxy/radio1ago?mp=/stream");
//  strcpy(UnraveledUrl, "https://sc1.xdevel.com/ribeirabrava");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/85whutype7duv");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/e59pwkvm3reuv");
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/2ee8m52mb"); 
//  strcpy(UnraveledUrl, "https://stream.zeno.fm/8pbaase2w2quv");

  Serial.printf("Now connect: %s\n", UnraveledUrl);
  stream.connecttohost(UnraveledUrl);  

  if(stream.isRunning()) 
  { Serial.printf("Succesfully connected: %s -> time elapsed = %ld\n", UnraveledUrl, (currentMillis = millis()) - lapMillis);
    Serial.print("Codec: ");
    Serial.println(stream.currentCodec());
    Serial.print("Bitrate: ");
    Serial.print(stream.bitrate());
    Serial.println(" kbps");
    return_result = 1;
    strcpy(ConnectedUrl, UnraveledUrl);
  }
  else
  { Serial.printf("Could not connect: %s -> time elapsed = %ld\n", UnraveledUrl, (currentMillis = millis()) - lapMillis);
  }  
 
  Tuning = false;
  return return_result;
}

void SetVolumeMapped(uint16_t volume)
{ if(volume==0)stream.setVolume(0);
  else stream.setVolume(map(volume, 0, 100, 75, 100)); // useable volume range of VS1053 is 75-100, 75 being very quiet
}




void audio_showstation(const char* info) 
{ char *p;
  // Serial.printf("Station: %s\n", info);
  // filter crap messages
  if((p=strchr(info, '-')) !=0) *p=0; // split idotic long names that combine station & content 
  if(strcmp(info, "no name")==0)return; // ignore meaningless names
  if(strcmp(info, "No Name")==0)return; // ignore meaningless names
  if(strcmp(info, "NO NAME")==0)return; // ignore meaningless names
  if(strcmp(info, "My Station name")==0)return; // ignore meaningless names
  if(strcmp(info, "This is my server name")==0)return; // ignore meaningless names
  if(strcmp(info, "Untitled")==0)return; // ignore meaningless names
  if(strlen(info)<2)return; // ignore meaningless names
  AddToQueueForDisplay(info, MESSAGE_STATION_NAME);
}

void audio_showstreamtitle(const char* info) {
    // Serial.printf("Stream title: %s\n", info);
    // filter crap messages
    if(strcmp(info, "Now Playing info goes here")==0)return;
    if(strcmp(info, " - ")==0)return;
    AddToQueueForDisplay(info, MESSAGE_SONG_TITLE);
}


// results/problems as text
const char * fail_texts[] = {
   { "FAIL_INVALID_URL"},
   { "FAIL_HTTP_CLIENT"},
   { "FAIL_CONNECTING"},
   { "FAIL_PLAYLIST_CANT_REDIRECT"},
   { "FAIL_NO_STREAM_HANDLE"},
   { "FAIL_PLAYLIST_NO_DATA"},
   { "FAIL_PLAYLIST_NO_URL"},
   { "FAIL_UNSUPPORTED_MIME"},
   { "FAIL_CANT_REDIRECT"},
   { "FAIL_LOOP_NO_HTTP_CLIENT"},
   { "FAIL_LOOP_HTTP_DISCONNECT"},
   { "FAIL_LOOP_CONNECTION_LOST"},
   { "FAIL_LOOP_STREAM_TIMEOUT"},
   { "FAIL_LOOP_EOF_NO_REMAINING_BYTES"},
   { "FAIL_LOOP_EOF_NO_REMAINING_BYTES_HTTPS"},
};

void audio_connect_result(const int16_t result) {
  stream_connecttohost_result = result;
  if(result>=FAIL_ENUM_START)
  { Serial.printf("ESP32_VS1053_Stream (Failure) %s : %d\n", fail_texts[result-FAIL_ENUM_START], result);
  }
  else
  { Serial.printf("ESP32_VS1053_Stream (HTTP Return Code) : %d\n", result);
  }
}

void audio_eof_stream(const char* info) 
{ char message[QUEUEMESSAGELENGTH];
  Serial.printf("End of stream: %s\n", info);
  if(bPowerStatus == true)
  { DataFromGlobe.D_QueueStationIndex = -1;
    sprintf(message, "%d", stream_connecttohost_result); // could  be http return code or failure enum
    AddToQueueForDisplay(message, MESSAGE_CONNECTTOHOST_FAILURE); // let display store this number for log report
    AddToQueueForDisplay(RequestedUrl, MESSAGE_AUDIO_EOF_STREAM); // let display store this url in text file

    if(stream_connecttohost_result == FAIL_LOOP_EOF_NO_REMAINING_BYTES_HTTPS)
    { // try again, with http, remove 's' from url
      if(UnraveledUrl[4]=='s')
      { strcpy(&UnraveledUrl[4], &UnraveledUrl[5]);  
        Serial.printf("Try again with: %s\n", UnraveledUrl);
        if(StartNewStation()==1) // succes
        { // restore volume setting
          SetVolumeMapped(DataFromDisplay.volumevalue);
          DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
          AddToQueueForDisplay(ActiveUrl, MESSAGE_STATION_CONNECTED); 
          return;
        }
      }
    }  

//    AddToQueueForDisplay(RequestedUrl, MESSAGE_DEAD_STATION);
    AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
  }    
}



// EOF