#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "franks-esp-now.h"



/**
 * CREDITS
 * @file streams-url_mp3-audiokit.ino
 * @author Phil Schatzmann
 * @brief Copy MP3 stream from url and output it on vs1053
 * @version 0.1
 * @date 2021-96-25
 * 
 * @copyright Copyright (c) 2021
 */

// install https://github.com/pschatzmann/arduino-vs1053.git

/*
 * longitude: 0 = Greenwich - loopt van 180 (oosten) tot -180 (westen)
 * latitude: 0 = Equator - loopt van 90 (noordpool) tot -90 (zuidpool)
 */

 

#include "franks-stations.h"
#define COPY_LOG_OFF
#include "AudioTools.h"
#include "AudioTools/AudioLibs/VS1053Stream.h"

//hardware vs1053 board
// SCK pin 18
// (MI)SO pin 19
// (MO)SI pin 23
// (X)CS pin 5
// (X)DCS pin16
// DREQ pin 4
// (X)RST pin 15
#define VS1053_RESET 12

// Orientation of hardware connections ESP32WROOM, USB pointing downwards
// Left row
//   3V3
//    EN 
//   SVP GPIO36 
//   SVN GPIO39
//   D34 GPIO34 
//   D35 GPIO35 
//   D32 BLUE BUTTON ROCKOLA
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


ICYStream url("SSID","PASSWORD");  // or replace with ICYStream to get metadata
VS1053Stream vs1053; // final output
int16_t vs1053Volume = -1;

StreamCopy copier(vs1053, url); // copy url to decoder

// bourns encoders 1024 positions
// #define ENC_DI hardwarematig aan GND
#define ENC_CLK 21
#define ENC_DO 13
#define ENC_CS 22
int AbsEncoderPosition[2]; // 10 bits position data 
int AbsEncoderStatus[2];   // 6 bits status info
void ReadAbsEncoder(void);
void ReadAverageEncoders(void);

#include "AS5600.h"
#include "Wire.h"

const int SCL_1 = 14; 
const int SDA_1 = 27;
const int SCL_2 = 26;
const int SDA_2 = 25; 

AS5600 as5600_0(&Wire);
AS5600 as5600_1(&Wire1);
int16_t ReadEncoderTicker100mS = 0;

bool bUpAndRunning = false;
bool bEncoderNewPosition;
int16_t NSDegLive10; // unfiltered, uncalibrated NS degrees times 10
int16_t EWDegLive10; // unfiltered, uncalibrated EW degrees times 10
int16_t NSDeg10; // uncalibrated NS degrees times 10
int16_t EWDeg10; // uncalibrated EW degrees times 10
int16_t gNSCal10; // calibrated NS degrees times 10
int16_t gEWCal10; // calibrated EW degrees times 10

char ActiveStationUrl[256] = "";
char OldActiveStationUrl[256] = "";
char RequestedStationUrl[256] = "";

#include <EEPROM.h>
#define EEPROM_SIZE 256
struct eepromData {
int16_t Offset0;
int16_t Offset1;
char laststation[128];
};
eepromData GlobeSettings; // 0-31 for calibrations points in degrees * 100, 32 for encoder calibration 0-0 actual encoder positions 0-4095


void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  // my test board is b4:3a:45:a5:03:10
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}  

void setup(){
  pinMode(13, INPUT_PULLUP); 
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);  

  WiFi.mode(WIFI_STA);
  WiFi.begin("SSID", "PASSWORD");
  WiFi.setSleep(false);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
  setup_esp_now();


  // setup vs1053
  auto cfg = vs1053.defaultConfig();
  cfg.is_encoded_data = true; // vs1053 is accepting encoded data
  // Use your custom pins or define in AudioCodnfig.h
  //cfg.cs_pin = VS1053_CS; 
  //cfg.dcs_pin = VS1053_DCS;
  //cfg.dreq_pin = VS1053_DREQ;
  cfg.reset_pin = VS1053_RESET;
  vs1053.begin(cfg);
  DataFromDisplay.volumevalue = 10; // volume to start with

  // mp3 radio
  //url.begin("http://stream.srg-ssr.ch/m/rsj/mp3_128","audio/mp3");

  // Initialise as5600_0 Connection
  Wire.begin(SDA_1, SCL_1);
  Wire1.begin(SDA_2, SCL_2);

  EEPROM.begin(EEPROM_SIZE);

  // calibrate_globe(); // call just once to force during construction, set globe to NS0 and EW0

  EEPROM.get(0x0, GlobeSettings);
  Serial.print("Eeprom stored 0: ");
  Serial.println(GlobeSettings.Offset0);
  Serial.print("Eeprom stored 1: ");
  Serial.println(GlobeSettings.Offset1);

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

  setup_interrupts(); // see task-and-interrupts.ino
  url.setMetadataCallback(printMetaData);
  delay(500);
  bUpAndRunning = true;
  
}

int newstation;
int oldstation;

void loop(){
  copier.copy();
  HandleNewGlobePosition();
  if((ReadEncoderTicker100mS % 2) == 0)
  { if(PrevDataFromDisplay.volumevalue != DataFromDisplay.volumevalue)
    { PrevDataFromDisplay.volumevalue = DataFromDisplay.volumevalue;
      if(DataFromDisplay.volumevalue==0)vs1053.setVolume(0);
      else vs1053.setVolume(map(DataFromDisplay.volumevalue, 0, 100, 75, 100)/100.0);
      DataFromGlobe.actualvolume = vs1053Volume;
    }

    //Serial.print("ActiveStationUrl= "); Serial.println(ActiveStationUrl);
    //Serial.print("DataFromDisplay.RadioUrlRequest= "); Serial.println(DataFromDisplay.RadioUrlRequest);
    if(strcmp(ActiveStationUrl, DataFromDisplay.RadioUrlRequest) != 0)
    { strcpy(PrevDataFromDisplay.RadioUrlRequest, DataFromDisplay.RadioUrlRequest);
      DataFromGlobe.FindNewStation = 0;
      StartNewStation();
    }

    if(CalibrateZeroPos == 1234) // calibration started by display 
    { calibrate_globe();
      CalibrateZeroPos = 0;
    }
  }     
}




// callback for meta data
void printMetaData(MetaDataType type, const char* str, int len){
  Serial.print("==> ");
  Serial.print(MetaDataTypeStr[type]);
  Serial.print(": ");
  Serial.println(str);

  if(type==0) // Title
  { strncpy(DataFromGlobe.Title, str, sizeof(DataFromGlobe.Title)); // safe copy
    DataFromGlobe.Title[sizeof(DataFromGlobe.Title)-1]=0;
  }
  //if(type==1) // Artist
  //if(type==2) // Album
  //if(type==3) // Genre
  if(type==4) // Name
  { strncpy(DataFromGlobe.Name, str, sizeof(DataFromGlobe.Name)); // safe copy
    DataFromGlobe.Name[sizeof(DataFromGlobe.Name)-1]=0;
  }
  //if(type==5) // Description
 
}



void HandleNewGlobePosition(void)
{ if(bEncoderNewPosition)
  { bEncoderNewPosition = false;
    vs1053.setVolume(0);
  }
}

void StartNewStation(void)
{ bool return_result = false;
  strcpy(RequestedStationUrl, DataFromDisplay.RadioUrlRequest);
  if(strncmp(RequestedStationUrl, "http", 4) == 0)
  { Serial.print("RequestedStationUrl from display= "); Serial.println(RequestedStationUrl);
    if(strcmp(RequestedStationUrl, ActiveStationUrl) != 0) // truly another station
    { vs1053.setVolume(0);
      if(strlen(ActiveStationUrl)!=0) // something playing?
      { Serial.print("url.end ->");
        Serial.print(ActiveStationUrl);
        url.end();
        Serial.println(" ended!");
        strcpy(ActiveStationUrl, "");
        strcpy(DataFromGlobe.Name, "");
        strcpy(DataFromGlobe.Title, "");
      }
      Serial.print("url.begin -> ");
      Serial.println(RequestedStationUrl);
      Serial.print("url.begin (call) -> ");
      return_result = url.begin(RequestedStationUrl,"audio/mp3");
      Serial.print(" started! with bool result: ");
      Serial.println(return_result);
      if(return_result==1)
      { vs1053Volume = DataFromDisplay.volumevalue;
        if(DataFromDisplay.volumevalue==0)vs1053.setVolume(0);
        else vs1053.setVolume(map(DataFromDisplay.volumevalue, 0, 100, 75, 100)/100.0);
        strcpy(ActiveStationUrl, RequestedStationUrl);
        strcpy(GlobeSettings.laststation, ActiveStationUrl);
        Serial.print("Save GlobeSettings.laststation = "); Serial.println(GlobeSettings.laststation);
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
      }
    }  
  }
}


