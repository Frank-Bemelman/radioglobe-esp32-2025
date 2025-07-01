
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <WiFiClient.h>
#include <VS1053.h>               // https://github.com/baldram/ESP_VS1053_Library
#include <ESP32_VS1053_Stream.h>  // https://github.com/CelliesProjects/ESP32_VS1053_Stream

// changed in ESP32_VS1053_Stream.h around line 15-16
// the original lower values often give connection refused for more remote locations
// #define VS1053_CONNECT_TIMEOUT_MS 2000 // FB was 250 FB
// #define VS1053_CONNECT_TIMEOUT_MS_SSL 2500 // FB was 750


#include "franks-esp-now.h"
#include "mp3_sound_winxpstart.h"
#include "..\secrets.h"

// as defined in ..\secrets.h
// char SSID [32] = "YOUR-SSID";
// char PASSWORD[32] = "YOUR-WIFI-PASSWORD";


#include "franks-stations.h"
#define COPY_LOG_OFF

//hardware vs1053 board
// SCK pin 18
// (MI)SO pin 19
// (MO)SI pin 23
// (X)CS pin 5
// (X)DCS pin21
// DREQ pin 22
// (X)RST pin 15

#define SPI_CLK_PIN 18
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 23



// grey
#define  VS1053_CS 5
// orange
#define  VS1053_DCS 21
// yellow (was 4 first, moved to 22)
#define  VS1053_DREQ 22
// brown
#define VS1053_RESET 12

ESP32_VS1053_Stream stream;

// Orientation of hardware connections ESP32WROOM, USB pointing downwards
// Left row
//   3V3
//    EN 
//   SVP GPIO36 
//   SVN GPIO39
//   D34 GPIO34 
//   D35 GPIO35 
//   D32 
//   D33 
//   D25 
//   D26 
//   D27 
//   D14 
//   D12 
//   GND
//   D13 
//   SD2
//   SD3
//   GND
//   VIN 5V
//
// Right row
//   GND
//   D23 
//   D22 
//   TXO (GPIO1)
//   RXO (GPIO3) 
//   D21 (GPIO21)
//   GND
//   D19 (GPIO19)
//   D18 (GPIO18)
//   D5  (GPIO5) 
//   TX2 (GPIO17)
//   RX2 (GPIO16)
//   D4  (GPIO4) 
//   D2  (GPIO2) 
//   D15 (GPIO15)
//   SD1
//   SD0
//   GND
//   CLK

static uint32_t startMillis;
static uint32_t lapMillis;
static uint32_t currentMillis;


// URL_CLIENT_TIMEOUT set to 5000ms, line 330 in audiotoolsconfig.h

// another player for mp3 in header file with windows sound
// MemoryStream music(mp3_winxpstart, sizeof(mp3_winxpstart));
  


#include "AS5600.h"
#include "Wire.h"

// AS5600 encoders
const int SCL_1 = 14; 
const int SDA_1 = 27;
const int SCL_2 = 26;
const int SDA_2 = 25; 

AS5600 as5600_0(&Wire);
AS5600 as5600_1(&Wire1);
int16_t ReadEncoderTicker100mS = 0;
int16_t PrevTick = 0;

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
char UnraveledUrl[2560] = ""; // checked for redirects 

bool Tuning = false;

#include <EEPROM.h>
#define EEPROM_SIZE 256
struct eepromData {
int16_t Offset0;
int16_t Offset1;
uint16_t ee_volume; // 0-100 can be changed by display puck
uint16_t ee_bass; // 0-f can be changed by display puck
uint16_t ee_treble; // 0-f can be changed by display puck
char ssid[32];
char password[32];
char moredata[128];
};
eepromData GlobeSettings; 

uint8_t rtone[4] = {0, 3, 14, 0}; // tone control register of VS1053 responding to display controls for bass & treble, values of 0-15
uint8_t mac[6];

void setup()
{ char message[64];
  startMillis = millis();
  Serial.begin(115200);

  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);  // sets the digital pin 12 off
  delay(100);            // waits for a second
  digitalWrite(VS1053_RESET, HIGH); // sets the digital pin 12 on
  delay(100);            // waits for a second

  // Initialise as5600_0 Connection
  Wire.begin(SDA_1, SCL_1);
  Wire1.begin(SDA_2, SCL_2);

  EEPROM.begin(EEPROM_SIZE);

  // calibrate_globe(); // call just once to force during construction, set globe to NS0 and EW0

  EEPROM.get(0x0, GlobeSettings);
  GlobeSettings.ssid[sizeof(GlobeSettings.ssid)-1]=0;
  Serial.printf("Eeprom ssid: %s\n", GlobeSettings.ssid);
  GlobeSettings.password[sizeof(GlobeSettings.password)-1]=0;
  Serial.printf("Eeprom password: %s\n", GlobeSettings.password);

  Serial.printf("Eeprom stored 0: %d\n", GlobeSettings.Offset0);
  Serial.printf("Eeprom stored 1: %d\n", GlobeSettings.Offset1);

  as5600_0.begin(33);  //  set direction pin.
  as5600_0.setDirection(AS5600_COUNTERCLOCK_WISE);
  as5600_0.setOffset(GlobeSettings.Offset0 * AS5600_RAW_TO_DEGREES);
  Serial.print("Connect device 0: ");
  Serial.println(as5600_0.isConnected() ? "true" : "false");

  as5600_1.begin(32);  //  set direction pin.
  as5600_1.setDirection(AS5600_COUNTERCLOCK_WISE);
  as5600_1.setOffset(GlobeSettings.Offset1 * AS5600_RAW_TO_DEGREES);  
  Serial.print("Connect device 1: ");
  Serial.println(as5600_1.isConnected() ? "true" : "false");

  Serial.print("readangle NS = "); Serial.println(as5600_0.readAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("readangle EW = "); Serial.println(as5600_1.readAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("ReadAGC NS = "); Serial.println(as5600_0.readAGC());
  Serial.print("ReadAGC EW = "); Serial.println(as5600_1.readAGC());

  delay(500);

  DataFromGlobe.G_actualvolume = -1;
  DataFromGlobe.D_QueueStationIndex = -1;

  bUpAndRunning = true;
  
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
    
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  WiFi.setSleep(false);
  
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

  // Start SPI bus
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  Serial.println("spi bus started.");

  // Initialize the VS1053 decoder
  if (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !stream.isChipConnected()) 
  { Serial.println("Decoder not running - system halted");
    while (1) delay(100);
  }
  Serial.println("VS1053 up & running");

  // set VS1053 tone values
  rtone[0] = GlobeSettings.ee_treble;
  rtone[2] = GlobeSettings.ee_bass;
  Serial.printf("Volume from eeprom -> %d", GlobeSettings.ee_volume);
  SetVolumeMapped(GlobeSettings.ee_volume); // eeprom can be updated by changing bass or treble
  
  // windows tune
  VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
  player.playChunk((uint8_t *)mp3_winxpstart, sizeof(mp3_winxpstart));

  AddToQueueForDisplay("Globe Just Booted", MESSAGE_GLOBE_WANTS_CURRENT_STATION);
  
}


static bool once = true;
void loop()
{ static uint16_t update_eeprom_after_100mS = 0;

  stream.loop();

     // process messages from display
  if(DataFromGlobe.D_QueueSerialNumber != DataFromDisplay.D_QueueSerialNumber) // not yet acknowlegded and processed
  {  // quick acknowledge message from displayglobe as received
     DataFromGlobe.D_QueueSerialNumber = DataFromDisplay.D_QueueSerialNumber;
     // and now take care of it
    switch(DataFromDisplay.D_QueueMessageType)
    { case MESSAGE_GOOGLE_API_KEY:
        Serial.println("INCOMING MESSAGE_GOOGLE_API_KEY");
        Serial.println(DataFromDisplay.D_QueueMessage);
        break;
      case MESSAGE_START_THIS_STATION:
        strcpy(RequestedUrl, DataFromDisplay.D_QueueMessage);

        // will be unraveled later, redirected, derived from pls or m3u file
        strcpy(UnraveledUrl, RequestedUrl);

        Serial.printf("INCOMING MESSAGE_START_THIS_STATION: >%s<\n", DataFromDisplay.D_QueueMessage);  
        if(DataFromGlobe.D_QueueStationIndex == DataFromDisplay.D_QueueStationIndex)
        { // already conmected
          Serial.printf("ALREADY CONNECTED TO STATIONINDEX %d\n", DataFromGlobe.D_QueueStationIndex);  
          AddToQueueForDisplay(RequestedUrl, MESSAGE_STATION_CONNECTED);
          break;
        }
        
        Tuning = true;
        StartNewStation();
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

      // displays wants to know the timezone for a given location
      case MESSAGE_GET_TIMEZONE:
        Serial.printf("INCOMING MESSAGE_GET_TIMEZONE: >NS%f EW%f<\n", DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW );  
        Serial.printf("FOR STATION: %s\n", DataFromDisplay.D_QueueMessage);  
        DataFromGlobe.FindTimeZone = 1; // makes the CallGetTimeZone task actually do it
        break;

      case MESSAGE_NEW_LIST_LOADED:
        // reset this value because it is not related anymore to the new list of stations found by display
        DataFromGlobe.D_QueueStationIndex = -1;
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
  if(PrevTick != ReadEncoderTicker100mS)
  { PrevTick = ReadEncoderTicker100mS;
    if(PrevDataFromDisplay.volumevalue != DataFromDisplay.volumevalue)
    { PrevDataFromDisplay.volumevalue = DataFromDisplay.volumevalue;
      GlobeSettings.ee_volume = DataFromDisplay.volumevalue;
      SetVolumeMapped(DataFromDisplay.volumevalue);
      update_eeprom_after_100mS = 36000; // 3600 - sec 
    }

    if(PrevDataFromDisplay.bassvalue != DataFromDisplay.bassvalue)
    { PrevDataFromDisplay.bassvalue = DataFromDisplay.bassvalue;
      // bass usuable values is rtone[3] = 14 for lower frequency limit and rtone[2] = 0-15 for level control    
      rtone[3] = 14;
      rtone[2] = (DataFromDisplay.bassvalue * 15)/100;
      stream.setTone(rtone); 
      GlobeSettings.ee_bass = rtone[2];
      update_eeprom_after_100mS = 100; 
//      Serial.printf("Bass=level %x cutoff %x\n", (uint16_t)rtone[2], (uint16_t)rtone[3]);
    }

    if(PrevDataFromDisplay.treblevalue != DataFromDisplay.treblevalue)
    { PrevDataFromDisplay.treblevalue = DataFromDisplay.treblevalue;
      // treble usuable values is rtone[1] = 3 for higher frequency limit and rtone[0] = 0-15 for level control
      // convert from 0-100 to signed nibble f-e-d-c-b-a-9-0-1-2-3-4-5-6-7
      rtone[1] = 3;
      rtone[0] = (uint8_t) abs(((DataFromDisplay.treblevalue-50)*15/100)); 
//      Serial.printf("Nibble1 %04x\n", (uint16_t) rtone[0] );                           
      if((DataFromDisplay.treblevalue-50)<0)rtone[0] |= 0x08;
//      Serial.printf("Nibble2 %04x\n", (uint16_t) rtone[0] );                           
      stream.setTone(rtone); 
      GlobeSettings.ee_treble = rtone[0];
      update_eeprom_after_100mS = 100; 
//      Serial.printf("Treble=level %x cutoff %x\n", (uint16_t)rtone[0], (uint16_t)rtone[1]);
    }

    if(update_eeprom_after_100mS)
    { update_eeprom_after_100mS--;
      if(update_eeprom_after_100mS==0) 
      { EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        Serial.println("Eeprom saved..");
      }
    }  


    if(CalibrateZeroPos == 1234) // calibration started by display 
    { calibrate_globe();
      CalibrateZeroPos = 0;
    }
  }  

  delay(5);
  
}





void StartNewStation(void)
{ bool return_result = false;

  Tuning = true;
  Serial.printf("StartNewStation with >%s<\n", RequestedUrl);
  stream.setVolume(0);
  Serial.printf("stream.connecttohost %s stopped\n", ActiveUrl);
  stream.stopSong();
  DataFromGlobe.D_QueueStationIndex = -1;
  strcpy(ActiveUrl, "");
  //AddToQueueForDisplay("", MESSAGE_NAME); // already filled in by display station search, perhaps to be updated by actual station connecting
   AddToQueueForDisplay("", MESSAGE_SONG_TITLE); // remove 'now playing title'

  Serial.printf("stream.connecttohost ->%s\n", UnraveledUrl);
  lapMillis = millis(); 
  stream.connecttohost(UnraveledUrl);  
  if(stream.isRunning()) 
  { Serial.printf("stream.connecttohost succes -> time elapsed = %ld\n", (currentMillis = millis()) - lapMillis);
    return_result = 1;
  }
  else
  { Serial.printf("stream.connecttohost failed -> time elapsed = %ld\n", (currentMillis = millis()) - lapMillis);
  }  

  if(return_result==1)
  { // restore volume setting
    SetVolumeMapped(DataFromDisplay.volumevalue);

    strcpy(ActiveUrl, RequestedUrl);
    DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
    AddToQueueForDisplay("ActiveUrl", MESSAGE_STATION_CONNECTED); 
    //EEPROM.put(0x0, GlobeSettings);
    //EEPROM.commit();
  }
  else
  { Serial.printf("FAILED: stream.connecttohost \n");
    AddToQueueForDisplay("RequestedUrl", MESSAGE_DEAD_STATION);
    DataFromGlobe.D_QueueStationIndex = -1;
    AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
  }
  
  Tuning = false;
}

void SetVolumeMapped(uint16_t volume)
{ if(volume==0)stream.setVolume(0);
  else stream.setVolume(map(volume, 0, 100, 75, 100)); // useable volume range of VS1053 is 75-100, 75 being very quiet
}




void audio_showstation(const char* info) {
    Serial.printf("Station: %s\n", info);
    AddToQueueForDisplay(info, MESSAGE_STATION_NAME);
}

void audio_showstreamtitle(const char* info) {
    Serial.printf("Stream title: %s\n", info);
    AddToQueueForDisplay(info, MESSAGE_SONG_TITLE);
}

void audio_eof_stream(const char* info) {
    Serial.printf("End of stream: %s\n", info);
    AddToQueueForDisplay("RequestedUrl", MESSAGE_DEAD_STATION);
    DataFromGlobe.D_QueueStationIndex = -1;
    AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
}

// EOF

