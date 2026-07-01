//#include <ESP32FtpServer.h>


// in arduino library manager..
// install libs as shown here https://randomnerdtutorials.com/esp32-wi-fi-manager-asyncwebserver/
// this is for the wifi manager portal

// arduino ide settings 
// esp32 3.1.2 board library
// esp32s3 dev module
// tools -> flash size 16MB
// tools -> partition 16MB 3MB APP 9.9MB FATFS
// tools -> PSRAM OPI PSRAM 

// TODO - turns off BT module when power off - or better not - perhaps better not - yes, better not

// 30 JUN 26 - auto update refinements
// 27 JUN 26 - added auto update from github
// 25 MAY 26 - in setup_esp_now() make it just a station, not an acces point
// 24 MAY 26 Added BT switch to mqtt for HA
// 21 MAY 26 ESP32_VS1053_Stream updated
// 19 MRT 26 FEATURE Sd card connections added and tested, acces to SD card works, no functionality added though
// 17 MRT 26 IMPROVED Turn of amplifiers when stream stops, or after playing a soundbite
// 9 MRT 26 - GetGeolocationData() local arrys enlarged, was crashing on weird thai town names
// other libraries and updates
// 25feb2026 -> AS5600@0.6.6 (was AS5600@0.6.5)
// reminder features 
// 14FEB26 -> has wifi channel updated by globe wifi connection
// 18APR26 -> added ftp server for SD card acces, see https://github.com/Annabel369/ESP32FTPServer (works but is very slow, practically not very useful) user guest, password guest
// 25JUN26 -> added update routines 

#include <Arduino.h>

#define BUILD_LABEL "Globe Build"
// make this -> Globe Build Jun 23 2026 - 21:08:50
#define BUILD_TIMESTAMP  BUILD_LABEL " " __DATE__ " - " __TIME__
#define BUILD_TIMESTAMP_ONLY __DATE__ " - " __TIME__
const char build_label[] =  BUILD_LABEL;
const char build_timestamp[] =  BUILD_TIMESTAMP;
const char build_timestamp_only[] =  BUILD_TIMESTAMP_ONLY;
extern uint8_t UpdateFirmware(uint8_t state); // start with 1 for a full date/time check and update

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include "ESP32FtpServer.h"
#include <esp_wifi.h>
#include <WiFiClient.h>
//#include <ESP32_VS1053_Stream_V4.h>  // https://github.com/CelliesProjects/ESP32_VS1053_Stream
#include "ESP32_VS1053_Stream.h"  // https://github.com/CelliesProjects/ESP32_VS1053_Stream
// keeping the ESP32_VS1053_Stream in my sketch folder now, to make it easier ot mess around with
// changed in ESP32_VS1053_Stream.h around line 15-16
// the original lower values often give connection refused for more remote locations
// #define VS1053_CONNECT_TIMEOUT_MS 5000 // FB was 500 FB
// #define VS1053_CONNECT_TIMEOUT_MS_SSL 5000 // FB was 1000
#include "franks-esp-now.h"
#include "countrycodes.h" // in this project

//#include "mp3_sound_winxpstart.h"
//#include "mp3_sound_winxpshutdown.h"
#include "mp3_startup.h"
#include "mp3_shutdown.h"
#include "mp3_voice_prompts.h"
#include "mp3_radio_tuning.h"
#include "mp3_happy_ping.h"

#include "..\secrets.h"
// as defined in ..\secrets.h
// char SSID [32] = "YOUR-SSID";
// char PASSWORD[32] = "YOUR-WIFI-PASSWORD";
// char google_api_key[] = "YOUR-API-KEY"; // free, get your own at google developer platform, used for timezone retrieval, replace first 4 letters AIza in !!!!
// to avoid a verbatim copy of api key in compiled binary 
#include "AS5600.h"
#include "Wire.h"


#define COPY_LOG_OFF

// esp32s3-16r8 has a nice pixel led which we will use to indicate Wifi portal active and more
// note -> there is a solder bridge next to the pixel led, you have to put a small solder blob on that to make it work
#include <Adafruit_NeoPixel.h> 
#define PIN 48
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide/
// VS1053 sound module esp32-S3-N16R8 pin arrangement HSPI (SPI2)
#define SPI_MOSI_PIN 11 // white
#define SPI_CLK_PIN 12  // green
#define SPI_MISO_PIN 13 // orange

#define VS1053_CS 16 // grey
#define VS1053_DREQ 9 // yellow 
#define VS1053_DCS 10 // blue
#define VS1053_RESET 15 // brown

//#include "FS.H"
#include "SD.h"
#include "SPI.H"

// SD card module, simplest model, sharing same pins as VS1053 except CS is different -> works but stutters when playing from SD
//#define SPI_MOSI_PIN 11 // green (pin 3 SD module)
//#define SPI_CLK_PIN 12  // yellow (pin 4 SD module)
//#define SPI_MISO_PIN 13 // orange (pin 5 SD module)

// Tried, but cam't make it work with 2nd SPI/VSI/HSPI -> keep rebooting
//#define SD_MOSI 35 // green (pin 3 SD module)
//#define SD_CLK 36  // yellow (pin 4 SD module)
//#define SD_MISO 37 // orange (pin 5 SD module)

#define SD_MOSI 41 // green (pin 3 SD module)
#define SD_CLK 40  // yellow (pin 4 SD module)
#define SD_MISO 38 // orange (pin 5 SD module)
#define SD_CS 39 // blue (pin 2 SD module)
// brown GND (pin 6 SD module)
// red +3V3 (pin 1 SD module)

SPIClass *hspi = NULL;



ESP32_VS1053_Stream stream;
//VS1053 chunkplayer(VS1053_CS, VS1053_DCS, VS1053_DREQ);

// AS5600 encoders esp32-wroom-32 pin arrangement
//const int SCL_1 = 33; // &wire as5600_0 grey
//const int SDA_1 = 32; // &wire as5600_0 brown
//const int SCL_2 = 26; // &wire as5600_1 grey
//const int SDA_2 = 25; // &wire as5600_1 brown

// AS5600 encoders esp32-S3-N16R8 pin arrangement
const int SCL_1 = 4; // &wire as5600_1 grey NS LAT
const int SDA_1 = 5; // &wire as5600_1 brown NS LAT
const int SCL_2 = 6; // &wire as5600_2 grey EW LON
const int SDA_2 = 7; // &wire as5600_2 brown EW LON

#define MUTE_AMPLIFIERS 14 // pin number
#define SPEAKERS_OFF 0
#define SPEAKERS_ON 1
#define SPEAKERS_DELAYED_OFF 3

uint16_t VolumeSoundBiteValue;


AS5600 as5600_1(&Wire); // NS LAT Encoder
AS5600 as5600_2(&Wire1); // EW LON Encoder

static uint32_t startMillis;
static uint32_t lapMillis;
static uint32_t currentMillis;
static uint32_t ConnectedInMillis;

int16_t ReadEncoderTicker100mS = 0;
int16_t PrevTick = 0;
bool bPowerStatus = true;
bool bVolumeToneControlsActive = false;

bool bUpAndRunning = false;
bool bSetupCompleted = false;
bool bEncoderNewPosition = false;
bool bEncoderKillStation = false;
uint16_t Timer100msSerialCanBeOpened = 0; // set to 5 seconds after 5 taps on touch
uint16_t Timer100msSerialIsOpen = 0; // set to 30 seconds after speaker switch pressed
uint16_t SpeakerOffAfter25mS = 0;


char TargetUrl[QUEUEMESSAGELENGTH] = ""; // most recent url requested by display
char ActiveUrl[QUEUEMESSAGELENGTH] = ""; // most recent url connected to

bool Tuning = false;

#define SPEAKER_TOGGLE_PIN 21 // pushbutton behind globe support for speaker toggle and bleutooth connect
#define BT_CONNECT_PIN 17 // output connected to BT module to force a BT connect
#define BT_POWER_PIN 2
#define PORTALSWITCH_PIN 20 // pushbutton located on the back, short press  opens portal to connect to smartphone and configure wifi and puck mac, long press > 3 seconds is resets,all settings

#include <EEPROM.h>
#define EEPROM_SIZE 512
struct eepromData 
{ int16_t Offset1;
  int16_t Offset2;
  uint16_t ee_volume; // 0-100 can be changed by display puck
  uint16_t ee_bass; // 0-f can be changed by display puck
  uint16_t ee_treble; // 0-f can be changed by display puck
  uint16_t ee_internal_speakers; // 0-f can be changed by display puck
  char HomeCountryCode[3]; // used as base for exchange rate 
  char ssid[32];
  char password[32];
  char google_api_key[64];
  uint8_t ee_puckmac[6];
  uint8_t ee_mqttserver_ip[4];
  uint32_t magicnumber;
  uint32_t serialnumber;
  uint16_t btmodule_installed;
  uint16_t btmodule_switchable;
  uint16_t btmodule_power_on;
  char open_weather_map_api_key[64];
  uint16_t sdcard_present;
  char spare_data[];
};
eepromData GlobeSettings; 

uint8_t rtone[4] = {0, 3, 14, 0}; // tone control register of VS1053 responding to display controls for bass & treble, values of 0-15

bool GetRatesNow = false;
uint16_t IgnoreMqqtUpdates = 0;

uint8_t GlobeMac[6];
uint8_t PuckMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int16_t bMqttActivated = 4567;

unsigned int PresetRequestFromHA = 0;
char ActiveStationTitle[256] = "";
char ActiveSongTitle[256] = "";

char MetaDataSongTitle[256] = "";
char MetaDataRadioStation[256] = "";


void TaskTouch(void * pvParameters);
void PixelUpdate(uint16_t mode, uint32_t color1, uint32_t color2, uint16_t msek);

char HostName[32];

bool bMusicMode = false;
bool bMusicModePrev = false;

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

FtpServer ftp;

extern uint16_t UpdateState;

const char* VS1053_connectResult;

void readMacAddress()
{ esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, GlobeMac);
  if (ret == ESP_OK) 
  { Serial.println(BUILD_TIMESTAMP);
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  GlobeMac[0], GlobeMac[1], GlobeMac[2],
                  GlobeMac[3], GlobeMac[4], GlobeMac[5]);
  } 
  else 
  { Serial.println("Failed to read Globe MAC address!");
  }
}


void setup()
{ char message[64];

  // replace !!!! characters in fetched api key (secrets.h)
  google_api_key[0]='A';
  google_api_key[1]='I';
  google_api_key[2]='z';
  google_api_key[3]='a';

  startMillis = millis();

  xTaskCreatePinnedToCore(
                    TaskTouch,   /* Task function. */
                    "TaskTouch",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core */   
  
  PixelUpdate(0, 0xFFFFFF, 0x000000, 10000); // solid white                     

  
  pinMode(BT_CONNECT_PIN, OUTPUT);
  digitalWrite(BT_CONNECT_PIN, HIGH);  
  
  pinMode(MUTE_AMPLIFIERS, OUTPUT);
  Speakers(SPEAKERS_OFF);
  pinMode(SPEAKER_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(PORTALSWITCH_PIN, INPUT_PULLUP); // input to button for opening portal

  Serial.begin(115200);

  Serial.printf("GlobeSettings size = %d bytes\n", sizeof(GlobeSettings));

  //Start SPI bus
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  Serial.println("vspi bus started.");

  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);  
  delay(50);   
  digitalWrite(VS1053_RESET, HIGH); 
  delay(50);          

  // Initialize the VS1053 decoder
  if (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !stream.isChipConnected()) 
  { Serial.println("Decoder not running - this is bad");
  }
  else Serial.println("VS1053 up & running");
  // Set the codec callback
  stream.setCodecCB(codecCallback);
  // Set the bitrate callback
  // stream.setBitrateCB(bitrateCallback);   
  // Set the station name callback
  stream.setStationCB(audio_showstation);
  // Set the stream metadata callback
  stream.setInfoCB(audio_showstreamtitle);
  // Set the EOF callback
  stream.setEofCB(audio_eof_stream);    

  // Set the fail callback
  //stream.setFailCB(audio_fail);    

  // Set the error callback
  stream.setErrorCB(audio_error);    


  delay(50);

 

  // Initialise as5600_0 Connection
  Wire.begin(SDA_1, SCL_1);
  Wire1.begin(SDA_2, SCL_2);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0x0, GlobeSettings);

  if(CheckIfBTSwitchable())GlobeSettings.btmodule_switchable = 1;
  else GlobeSettings.btmodule_switchable = 0;

  InitializeGlobeSettings(); // at first run, initialize with some reasonable values

  // start SD card
  hspi = new SPIClass (HSPI);
  hspi->begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  if(CheckSD())GlobeSettings.sdcard_present = 1;
  else GlobeSettings.sdcard_present = 0;

  if(GlobeSettings.btmodule_power_on) digitalWrite(BT_POWER_PIN, HIGH);  // turn on
  else digitalWrite(BT_POWER_PIN, LOW);  // turn off

  
  GlobeSettings.ssid[sizeof(GlobeSettings.ssid)-1]=0;
  //Serial.printf("Eeprom ssid: %s\n", GlobeSettings.ssid);
  GlobeSettings.password[sizeof(GlobeSettings.password)-1]=0;
  //Serial.printf("Eeprom password: %s\n", GlobeSettings.password);

  Serial.printf("Eeprom stored Offset1: %d\n", GlobeSettings.Offset1);
  Serial.printf("Eeprom stored Offset2: %d\n", GlobeSettings.Offset2);

  Serial.printf("Mqtt Server set at %d.%d.%d.%d\n", (uint16_t)GlobeSettings.ee_mqttserver_ip[0], (uint16_t)GlobeSettings.ee_mqttserver_ip[1], (uint16_t)GlobeSettings.ee_mqttserver_ip[2], (uint16_t)GlobeSettings.ee_mqttserver_ip[3]);

    // NS encoder
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

// A0:85:E3:E1:50:4C
//GlobeSettings.ee_puckmac[0] = 0xA0; 
//GlobeSettings.ee_puckmac[1] = 0x85; 
//GlobeSettings.ee_puckmac[2] = 0xE3;
//GlobeSettings.ee_puckmac[3] = 0xE1;
//GlobeSettings.ee_puckmac[4] = 0x50; 
//GlobeSettings.ee_puckmac[5] = 0x4C;

  Serial.printf("Puck Mac %02X:%02X:%02X:%02X:%02X:%02X as stored in eeprom\n", GlobeSettings.ee_puckmac[0], GlobeSettings.ee_puckmac[1], GlobeSettings.ee_puckmac[2], GlobeSettings.ee_puckmac[3], GlobeSettings.ee_puckmac[4], GlobeSettings.ee_puckmac[5]);
  
  
  
  
  memcpy(PuckMac, GlobeSettings.ee_puckmac, 6);

  PlaySoundBite((uint8_t *)mp3_happy_ping, sizeof(mp3_happy_ping), 0); 

  DataFromGlobe.D_QueueStationIndex = -1;

  bUpAndRunning = true;
  
  Serial.printf("Rotate right blue\n");                           
  PixelUpdate(4, 0x0000FF, 0x000000, 15000); // rotate right, solid blue  

  AutoConnectSetup();
  WiFi.setHostname(HostName);

  WiFi.begin(GlobeSettings.ssid, GlobeSettings.password); 
  btStop(); // as recommended by Cellie
  WiFi.setSleep(false); // as recommended by Cellie
  Serial.print("Connecting to WiFi");
  delay(1000);
  uint16_t timeout = 150; // 15 seconds max
  while (!WiFi.isConnected() && timeout--) 
  { Serial.print('.');
    delay(100);
  }
  Serial.println("");

  if(WiFi.isConnected())
  { PlaySoundBite((uint8_t *)mp3_wificonnected, sizeof(mp3_wificonnected), 0); 
    Serial.printf("Connected with hostname %s\n", WiFi.getHostname());
    Serial.println("Connected with EEprom stored credentials");
  }
  else 
  { Serial.println("Try Connecting with WifiManager");
    PlaySoundBite((uint8_t *)mp3_smartphone_portal, sizeof(mp3_smartphone_portal), 50); 
    AutoConnect(); // wifi auto connect with portal if need be
    if(WiFi.isConnected())
    { Serial.println("Connected with WifiManager stored credentials");
    }
  }
  
  if(WiFi.isConnected())
  { Serial.print("Wifi connected to IP: ");
    Serial.println(WiFi.localIP());
    PixelUpdate(0, 0x00FF00, 0x000000, 5000); // solid green
    delay(2000);
  }
  else
  { Serial.println(" wifi NOT connected.");
    PlaySoundBite((uint8_t *)mp3_wifidisconnected, sizeof(mp3_wifidisconnected), 0); 
    while(1)
    { PixelUpdate(6, 0xFF0000, 0x000000, 5000); // flash, solid red   
      delay(5000);
    }
  }

  Serial.printf("Wifi connected to channel: %d\n", WiFi.channel());
  Serial.printf("Wifi-strenght = %d dBm\n", WiFi.RSSI());

  setup_esp_now();
  
  // setup MQTT for Home Assitant if configured in portal settings
  if(GlobeSettings.ee_mqttserver_ip[0] || GlobeSettings.ee_mqttserver_ip[1] || GlobeSettings.ee_mqttserver_ip[2] || GlobeSettings.ee_mqttserver_ip[3])
  { setupMQTT();
  }

  readMacAddress();
  sprintf(message, "%02X:%02X:%02X:%02X:%02X:%02X", GlobeMac[0], GlobeMac[1], GlobeMac[2], GlobeMac[3], GlobeMac[4], GlobeMac[5]);
  AddToQueueForDisplay(message, MESSAGE_GLOBE_MAC);
  sprintf(message, "%d", WiFi.channel());

  if(GlobeSettings.btmodule_installed == 0)Serial.println("No Bluetooth module installed");
  else Serial.println("Has Bluetooth module installed");
  if(GlobeSettings.btmodule_switchable == 0)Serial.println("Bluetooth is NOT switchable");
  else Serial.println("Bluetooth is switchable");

  
  sprintf(message, "%d-%d-%d", GlobeSettings.btmodule_switchable, GlobeSettings.btmodule_power_on, GlobeSettings.btmodule_installed);
  AddToQueueForDisplay(message, MESSAGE_DISPLAY_BT_SWITCHABLE_STATE);

  sprintf(message, "%d", GlobeSettings.sdcard_present);
  AddToQueueForDisplay(message, MESSAGE_GLOBE_HAS_SD);

  AddToQueueForDisplay(WiFi.localIP().toString().c_str(), MESSAGE_GLOBE_IP);


  setup_tasks(); // see task-and-interrupts.ino, will also indirectly start to use espnow

  

  // tell display what our stored values are for volume and tone control
  sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
  DataFromGlobe.G_Volume = GlobeSettings.ee_volume;
  AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);

   
  AddToQueueForDisplay(build_timestamp, MESSAGE_GLOBE_BUILD_DATE_TIME);
  
  PlaySoundBite((uint8_t *)mp3_startup, sizeof(mp3_startup), 0); 

  sprintf(message, "%d", GlobeSettings.serialnumber);
  AddToQueueForDisplay(message, MESSAGE_DISPLAY_SERIALNUMBER); // set puck to same serial number
  // end of setup
  GetRatesNow = true;
  bSetupCompleted = true;
  
  //if(GlobeSettings.sdcard_present)
  //{ ftp.begin("guest", "guest");
  //  Serial.println("Servidor FTP ativo!");
  //}  

}


static bool once = true;

void loop()
{ char message[QUEUEMESSAGELENGTH];
  float GpsNS;
  float GpsEW;
  static String receivedMessage = "";
  static char incomingChar;
  static char text[64];
  static char *p;

  static uint16_t QueueMessageType;
  static char QueueMessage[QUEUEMESSAGELENGTH];
  static uint32_t value_int32t = 0;
  static uint16_t connection_lost_counter;
  static char PreviousUrl[QUEUEMESSAGELENGTH] =""; // holds the url requested by display
  static bool bFirst = true;
  
   //if(GlobeSettings.sdcard_present)ftp.handleFTP();
  loop2(); // checks portal button
  stream.loop();
  if(bMqttActivated==1234)loopMQTT();
  //else Serial.printf("bMqttActivated ??? = %d\n", bMqttActivated);

  //Serial.println(touchRead(8));
  if(UpdateState)
  { UpdateState = UpdateFirmware(UpdateState);
  } 

  if((ReadEncoderTicker100mS % 50)==0) // every 5 seconds or so
  { if(WiFi.status() != WL_CONNECTED) 
    { Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      connection_lost_counter++;
      if(connection_lost_counter>500)
      { Serial.println("WiFi connection lost for 2500 seconds...");
        delay(100);
        ESP.restart(); 
      }
    }
    else
    { connection_lost_counter = 0;
    }
  }

  if(Timer100msSerialIsOpen) // only when enabled, as this works when serial port is actually connected, but hangs when there is no usb cable to PC
  { 
  while(Serial.available()) 
  { Serial.print(".");
    incomingChar = Serial.read();
    if(incomingChar == '\n')
    { Serial.print("You typed this: ");
      Serial.println(receivedMessage);
      Serial.printf("S-GlobeSettings.btmodule_installed = %d\n", GlobeSettings.btmodule_installed);
      Serial.printf("S-GlobeSettings.btmodule_switchable = %d\n", GlobeSettings.btmodule_switchable);
      strcpy(text, receivedMessage.c_str());
      if((p = strstr(text, "BI")) != NULL) 
      { if(text[2]=='0')
        { GlobeSettings.btmodule_installed = 0;
          GlobeSettings.btmodule_switchable = 0;
          digitalWrite(BT_POWER_PIN, LOW);
        }  
        else 
        { GlobeSettings.btmodule_installed = 1;
          digitalWrite(BT_POWER_PIN, HIGH);
        } 
        Serial.printf("BI-GlobeSettings.btmodule_installed = %d\n", GlobeSettings.btmodule_installed);
        Serial.printf("BI-GlobeSettings.btmodule_switchable = %d\n", GlobeSettings.btmodule_switchable);
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        Serial.println("Eeprom initialized & saved..");
        sprintf(message, "%d-%d-%d", GlobeSettings.btmodule_switchable, GlobeSettings.btmodule_power_on, GlobeSettings.btmodule_installed);
        AddToQueueForDisplay(message, MESSAGE_DISPLAY_BT_SWITCHABLE_STATE);
      }
      if((p = strstr(text, "BS")) != NULL) 
      { Serial.printf("BS-GlobeSettings.btmodule_installed = %d\n", GlobeSettings.btmodule_installed);
        Serial.printf("BS-GlobeSettings.btmodule_switchable = %d\n", GlobeSettings.btmodule_switchable);
        if(text[2]=='0')GlobeSettings.btmodule_switchable = 0;
        else GlobeSettings.btmodule_switchable = 1;
        if(GlobeSettings.btmodule_installed==0)GlobeSettings.btmodule_switchable = 0;
        Serial.printf("BS-GlobeSettings.btmodule_installed = %d\n", GlobeSettings.btmodule_installed);
        Serial.printf("BS-GlobeSettings.btmodule_switchable = %d\n", GlobeSettings.btmodule_switchable);
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        Serial.println("Eeprom initialized & saved..");
        sprintf(message, "%d-%d-%d", GlobeSettings.btmodule_switchable, GlobeSettings.btmodule_power_on, GlobeSettings.btmodule_installed);
        AddToQueueForDisplay(message, MESSAGE_DISPLAY_BT_SWITCHABLE_STATE);
      }
      if((p = strstr(text, "SN")) != NULL) 
      { Serial.printf("SN-GlobeSettings.serialnumber was = %d\n", GlobeSettings.serialnumber);
        if(strlen(text)>3)
        { sscanf(text,"SN%d", &value_int32t);
          if((value_int32t>50) && (value_int32t<10000)) // valis serial number
          { if(GlobeSettings.serialnumber != value_int32t) // only take action if actually changed
            { GlobeSettings.serialnumber = value_int32t;
              Serial.printf("SN-GlobeSettings.serialnumber stored = %d\n", GlobeSettings.serialnumber);
              EEPROM.put(0x0, GlobeSettings);
              EEPROM.commit();
              Serial.printf("SN-GlobeSettings.serialnumber stored in EEprom = %d\n", GlobeSettings.serialnumber);
              sprintf(text, "%d", GlobeSettings.serialnumber);
              AddToQueueForDisplay(text, MESSAGE_DISPLAY_SERIALNUMBER); // set puck to same serial number
            }
          }
        }  
      }

      receivedMessage = "";
      Serial.printf("->GlobeSettings.btmodule_installed = %d\n", GlobeSettings.btmodule_installed);
      Serial.printf("->GlobeSettings.btmodule_switchable = %d\n", GlobeSettings.btmodule_switchable);
    }
    else
    { receivedMessage += incomingChar;
    }
  }
  }


  
  // process messages from display
  // if(DataFromGlobe.D_QueueSerialNumber != DataFromDisplay.D_QueueSerialNumber) // not yet acknowlegded and processed
  while(FromDisplay.QueueIndexIn != FromDisplay.QueueIndexOut) // we have to catch up with new messages
  {  Serial.printf("FromDisplay.QueueIndexIn = %d FromDisplay.QueueIndexOut = %d\n", FromDisplay.QueueIndexIn, FromDisplay.QueueIndexOut);
     Serial.printf("Messages from display pending %d\n", FromDisplay.QueueCnt);

     // copy the essential message info into a more readable variable
     QueueMessageType = FromDisplay.QueueMessageType[FromDisplay.QueueIndexOut];
     memcpy(QueueMessage, FromDisplay.QueueMessage[FromDisplay.QueueIndexOut], sizeof(QueueMessage));

     if((QueueMessageType>=0) && (QueueMessageType<=MESSAGE_MAX)) 
     { Serial.printf("DISPLAY SAYS: %s >%s<\n", messagetexts[QueueMessageType], QueueMessage);  
     }


     // and now take care of it
     //switch(DataFromDisplay.D_QueueMessageType)
     switch(QueueMessageType)
     { case MESSAGE_OPEN_WEATHER_MAP_API_KEY:
         Serial.println(QueueMessage);
         if(strcmp(GlobeSettings.open_weather_map_api_key, QueueMessage)!=NULL)
         { strcpy(GlobeSettings.open_weather_map_api_key, QueueMessage);
           Serial.println("Eeprom saved -> GlobeSettings.open_weather_map_api_key");
           EEPROM.put(0x0, GlobeSettings);
           EEPROM.commit();
         }
         break;
      
       case MESSAGE_GOOGLE_API_KEY:
         Serial.println(QueueMessage);
         if(strcmp(GlobeSettings.google_api_key, QueueMessage)!=NULL)
         { strcpy(GlobeSettings.google_api_key, QueueMessage);
           Serial.println("Eeprom saved -> GlobeSettings.google_api_key");
           EEPROM.put(0x0, GlobeSettings);
           EEPROM.commit();
         }
         break;

       case MESSAGE_THIS_IS_HOME:
         QueueMessage[2]=0; // keep it short
         strcpy(GlobeSettings.HomeCountryCode, QueueMessage);
         EEPROM.put(0x0, GlobeSettings);
         EEPROM.commit();
         Serial.println("Eeprom saved -> GlobeSettings.HomeCountryCode");
         GetRatesNow = true;
         break;  

       // display want exchange rate, for a country code supplied
       case MESSAGE_EX_CHANGE_RATE:
         extern char exchangepayload[]; 
         if(exchangepayload[0]==0)
         { GetRatesNow = true;
         }
         else 
         { int16_t fromindex = IsCountryCodeValid(GlobeSettings.HomeCountryCode);
           int16_t toindex = IsCountryCodeValid(QueueMessage);
           Serial.printf("-------------------> valuta from %d to %d\n", fromindex, toindex);
           if(((fromindex>=0) && (toindex>=0)) && (fromindex!=toindex)) // valid it is and sense it makes
           { // we are not in our home location
             // create text like 1 euro = 1.13 US Dollar
             // check if these countries actually use different valuta
             if(CountryList[fromindex].valutanumber != CountryList[toindex].valutanumber)
             { if((CountryList[toindex].exchangerate)>=100.0)sprintf(message, "1 %s = %.0f %s", CountryList[fromindex].valutaname, CountryList[toindex].exchangerate, CountryList[toindex].valutaname);
               else if((CountryList[toindex].exchangerate)>=10.0)sprintf(message, "1 %s = %.1f %s", CountryList[fromindex].valutaname, CountryList[toindex].exchangerate, CountryList[toindex].valutaname);
               else if((CountryList[toindex].exchangerate)>=1.0)sprintf(message, "1 %s = %.2f %s", CountryList[fromindex].valutaname, CountryList[toindex].exchangerate, CountryList[toindex].valutaname);
               else if((CountryList[toindex].exchangerate)>=0.1)sprintf(message, "1 %s = %.3f %s", CountryList[fromindex].valutaname, CountryList[toindex].exchangerate, CountryList[toindex].valutaname);
               else if((CountryList[toindex].exchangerate)>=0.01)sprintf(message, "1 %s = %.4f %s", CountryList[fromindex].valutaname, CountryList[toindex].exchangerate, CountryList[toindex].valutaname);
               else sprintf(message, "1 %s = %.5f %s", CountryList[fromindex].valutaname, CountryList[toindex].exchangerate, CountryList[toindex].valutaname);
               AddToQueueForDisplay(message, MESSAGE_EX_CHANGE_RATE);
             }
             else 
             { // valuta's are the same 
               sprintf(message, "Pay With %s Here", CountryList[toindex].valutaname); 
               AddToQueueForDisplay(message, MESSAGE_EX_CHANGE_RATE); // valuta's are the same
             }
           }
           else 
           { AddToQueueForDisplay("Pay With Your Own Currency", MESSAGE_EX_CHANGE_RATE); // countrycode is homelocation
           }
         }  
         break;

       case MESSAGE_CALIBRATE_ZERO:
         if(strcmp(QueueMessage, "1234")==0)
         { calibrate_globe();
         }
         break;  

       case MESSAGE_START_THIS_STATION:
         if(!bPowerStatus)
         { Serial.printf("MESSAGE_START_THIS_STATION: IGNORED (Power Down)\n");
           break;
         }

         if((strcmp(PreviousUrl, QueueMessage) == NULL) && stream.isRunning())
         { // already conmected PLAYING
           Serial.printf("ALREADY CONNECTED TO STATION: %s\n", QueueMessage);  
           AddToQueueForDisplay(TargetUrl, MESSAGE_STATION_CONNECTED);
           break;
         }
             
         strcpy(PreviousUrl, QueueMessage);
         strcpy(TargetUrl, QueueMessage);

         // DataFromDisplay.D_QueueStationIndex range = 0-49 for found stations and 50-53 for presets, -1 if idle
         Serial.printf("DataFromDisplay.D_QueueStationIndex = %d\n", DataFromDisplay.D_QueueStationIndex);
         
         
         if(DataFromDisplay.D_QueueStationIndex>=MAX_STATIONS) // it's a preset
         { PresetRequestFromHA = DataFromDisplay.D_QueueStationIndex-MAX_STATIONS+1;
           // will be picked up by mqtt loop to inform HA
         }  
         else PresetRequestFromHA = 0;       
        

         // quick & dirty, but maybe too dirty, as not all https urls are http appraocheable (HTTP error 400)
         // but we also catch the 400 and do a retry on https in that case 
         // so, always try http first, remove the 's' from https
         if(TargetUrl[4]=='s')strcpy(&TargetUrl[4], &TargetUrl[5]);  
        
         Tuning = true;
         if(StartNewStation()==1) // succes
         { DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
           //Serial.printf("SUCCES: stream.connecttohost HTTP returncode %d\n", stream.connectResult()); 
           Serial.printf("SUCCES: stream.connecttohost\n"); 
           
           AddToQueueForDisplay(QueueMessage, MESSAGE_STATION_CONNECTED); 
           sprintf(message, "%ld,\"%s\"", ConnectedInMillis, QueueMessage); 
           AddToQueueForDisplay(message, MESSAGE_STATION_CONNECTED_IN_MS); 
         }
         else
         {  DataFromGlobe.D_QueueStationIndex = -1;
            //Serial.printf("FAILED: stream.connecttohost HTTP code %d\n", stream.connectResult()); 
            Serial.printf("FAILED: stream.connecttohost ERROR code %s\n", VS1053_connectResult); 

            if(strcmp(VS1053_connectResult, "Http create error")==0) // DNS fail??
            { sprintf(message, "%s -> %s", VS1053_connectResult, QueueMessage); 
              AddToQueueForDisplay(message, MESSAGE_CONNECTTOHOST_FAILURE); // let display store this number for log report
              AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
            }
            else
            { // try again, toggle https<>http, insert 's' to url
              if(TargetUrl[4]==':') // try again, with https, insert 's' to url
              { int16_t len = strlen(TargetUrl);
                while(len>3)
                { TargetUrl[len+1]=TargetUrl[len]; // start with 0 termination
                  len--; // and work backwards
                }
                TargetUrl[4]='s';
              }
              if(StartNewStation()==1) // succes
              { DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
                Serial.printf("SUCCES: stream.connecttohost\n"); 
                AddToQueueForDisplay(QueueMessage, MESSAGE_STATION_CONNECTED); 
                sprintf(message, "%ld,\"%s\"", ConnectedInMillis, QueueMessage); 
                AddToQueueForDisplay(message, MESSAGE_STATION_CONNECTED_IN_MS); 
              }
              else
              { Serial.printf("FAILED: stream.connecttohost ERROR code %s\n", VS1053_connectResult); 

                sprintf(message, "%s -> %s", VS1053_connectResult, QueueMessage); 
                AddToQueueForDisplay(message, MESSAGE_CONNECTTOHOST_FAILURE); // let display store this number for log report
                AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);        //    audio_eof_stream("Retry from main message loop for HTTP errors < 0", 0);
              }
            }  
         }
         Tuning = false;

         break;

      case MESSAGE_NEW_LIST_LOADED:
        // reset this value because it is not related anymore to the new list of stations found by display
        DataFromGlobe.D_QueueStationIndex = -1;
        //PlaylistTracks=0;
        break;

      case MESSAGE_POWERDOWN:
        GlobePowerDown();
        break;

      case MESSAGE_POWERUP:
        GlobePowerUp();
        break;

      case MESSAGE_VOLUME_AND_TONE:
        bVolumeToneControlsActive = true;
        break;

      case MESSAGE_DISPLAY_WANTS_VOLUME_AND_TONE:
        EEPROM.get(0x0, GlobeSettings);
        sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
        AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);

        
        AddToQueueForDisplay(build_timestamp, MESSAGE_GLOBE_BUILD_DATE_TIME);

        break;

      case MESSAGE_STORE_VOLUME_AND_TONE:
        if(GlobeSettings.ee_volume < 10)GlobeSettings.ee_volume = 10;
        if(GlobeSettings.ee_volume > 70)GlobeSettings.ee_volume = 70;
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        Serial.println("Eeprom saved..");
        break;

      case MESSAGE_MQTT_STATUS:
        // display wants status at boot, to show a led in home icon on tone control screen
        if(bMqttActivated==1234)AddToQueueForDisplay("ON", MESSAGE_MQTT_STATUS);
        else  AddToQueueForDisplay("OFF", MESSAGE_MQTT_STATUS);
        break;

      case MESSAGE_OPEN_PORTAL:  
        checkButton(true); // force open portal by puck
        break;

      // let's play from SD card now, triggered by music button on preset screen puck
      case MESSAGE_GLOBE_PLAY_SD:
        DataFromGlobe.D_QueueStationIndex = -1; 
        strcpy(PreviousUrl, "");
        StartPlayFromSD();
        break;

      case  MESSAGE_START_THIS_FILE: 
        DataFromGlobe.D_QueueStationIndex = -1; 
        strcpy(PreviousUrl, QueueMessage);
        stream.stopSong(); // stop whatever stream or file was playing
         // delay(1000); check to see if click/
        Speakers(SPEAKERS_ON);
        SetVolumeMapped(DataFromDisplay.volumevalue); 
        stream.connectToFile(SD, QueueMessage); // play it 
//        stream.connectToFile(SD, "/GLOBEMUSIC/JUKEBOX/D04-Gloria Estefan-Higher.mp3"); // play it 
        break;
      
      case MESSAGE_RELOAD_SD_WITH_COUNTRY:
        extern char CountryCodeSelectorSD[3];
        strcpy(CountryCodeSelectorSD, QueueMessage); // will trigger StartPlayFromSD(void)
        strcpy(PreviousUrl, "");
        break;

      case MESSAGE_START_FILE_BY_INDEX:
        uint16_t idx;
        sscanf(QueueMessage, "%d", &idx);
        PlayFromPlaylistByIndex(idx);
        break;

      case MESSAGE_UPDATE_GLOBE:
        // in puck setup menu, long press serialnumber to trigger
        if(UpdateState==0)UpdateState = 1; // triggers the update procedure, if procedure does not run yet
        break;

      default:
        Serial.printf("Unsupported message type %d from display: >%s<\n", QueueMessageType, QueueMessage);  
        AddToQueueForDisplay("Unsupported", MESSAGE_MAX); 
        break;
    }

    FromDisplay.QueueIndexOut++;
    FromDisplay.QueueIndexOut %= QUEUESIZE;
    FromDisplay.QueueCnt--;

  }

  if(bEncoderNewPosition)
  { bEncoderNewPosition = false;
    DataFromGlobe.D_QueueStationIndex = -1; // not meaningfull anymore, current list will be replaced
    AddToQueueForDisplay("?", MESSAGE_FINDNEWSTATION);
  }
  
  if(bEncoderKillStation)
  { bEncoderKillStation = false;
    stream.stopSong();
    //chunkplayer.switchToMp3Mode(); // or Playwhile (radio-tuning-effect-sound) goes chit-chit-chit if this interrupted a file playing
    Speakers(SPEAKERS_DELAYED_OFF);
    DataFromGlobe.D_QueueStationIndex = -1;
    strcpy(ActiveUrl, "");
    
  }
  

  // volume and tone levels checking
  //if(bVolumeToneControlsActive == true)
  { if(PrevTick != ReadEncoderTicker100mS)
    { PrevTick = ReadEncoderTicker100mS;
      if((PrevDataFromDisplay.volumevalue != DataFromDisplay.volumevalue) || bFirst)
      { PrevDataFromDisplay.volumevalue = DataFromDisplay.volumevalue;
        SetVolumeMapped(DataFromDisplay.volumevalue);
        if(bPowerStatus == true) // when user swtiches off, volume is zero but meaning less, just stick with current value
        { GlobeSettings.ee_volume = DataFromDisplay.volumevalue;
        }  
      }

      if((PrevDataFromDisplay.bassvalue != DataFromDisplay.bassvalue) || bFirst)
      { PrevDataFromDisplay.bassvalue = DataFromDisplay.bassvalue;
        // bass usuable values is rtone[3] = 14 for lower frequency limit and rtone[2] = 0-15 for level control    
        rtone[3] = 14;
        rtone[2] = (DataFromDisplay.bassvalue * 15)/100;
        stream.setTone(rtone); 
        GlobeSettings.ee_bass = DataFromDisplay.bassvalue;
        GlobeSettings.ee_volume = DataFromDisplay.volumevalue;
        // Serial.printf("Bass=level %x cutoff %x\n", (uint16_t)rtone[2], (uint16_t)rtone[3]);
      }

      if((PrevDataFromDisplay.treblevalue != DataFromDisplay.treblevalue) || bFirst)
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
      if((PrevDataFromDisplay.internalspeakeron != DataFromDisplay.internalspeakeron) || bFirst)
      { PrevDataFromDisplay.internalspeakeron = DataFromDisplay.internalspeakeron;
        GlobeSettings.ee_internal_speakers = DataFromDisplay.internalspeakeron;
        if(GlobeSettings.ee_internal_speakers == 1)
        { if(stream.isRunning())
          { Speakers(SPEAKERS_ON);
            Serial.println("SPEAKERS_ON by DataFromDisplay.internalspeakeron");
          }  
        }
        else 
        { Speakers(SPEAKERS_OFF);
          Serial.println("SPEAKERS_OFF by DataFromDisplay.internalspeakeron");
        }           
      }
      if((PrevDataFromDisplay.btmodule_power_on != DataFromDisplay.btmodule_power_on) || bFirst)
      { PrevDataFromDisplay.btmodule_power_on = DataFromDisplay.btmodule_power_on;
        GlobeSettings.btmodule_power_on = DataFromDisplay.btmodule_power_on;
        if(GlobeSettings.btmodule_power_on == 1)
        { digitalWrite(BT_POWER_PIN, HIGH); 
          Serial.println("BT POWERED ON by DataFromDisplay.btmodule_power_on");
        
        }
        else 
        { digitalWrite(BT_POWER_PIN, LOW); 
          Serial.println("BT POWERED OFF by DataFromDisplay.btmodule_power_on");
        }          
      } 

      if(CalibrateZeroPos == 1234) // calibration started by display 
      { calibrate_globe();
        CalibrateZeroPos = 0;
      }
      bFirst = false;
    }
  }  
  //delay(5);
}





bool StartNewStation(void)
{ bool return_result = false;
  char message[QUEUEMESSAGELENGTH];

  bMusicMode = false;
  if(bMusicModePrev != bMusicMode)
  { bMusicModePrev = bMusicMode;
    AddToQueueForDisplay("0", MESSAGE_MUSIC_MODE);
  }  

  PixelUpdate(0, 0xFF00FF, 0x000000, 5000); // solid purple

  Serial.printf("StartNewStation with %s\n", TargetUrl);
  SetVolumeMapped(0);
  if(stream.isRunning())
  { Serial.printf("First stop this one: %s\n", ActiveUrl);
    stream.stopSong();
//    chunkplayer.switchToMp3Mode();
  }
  
  Speakers(SPEAKERS_DELAYED_OFF);
  DataFromGlobe.D_QueueStationIndex = -1;
  strcpy(ActiveUrl, "");
  strcpy(ActiveStationTitle, "");
  strcpy(ActiveSongTitle, "");
  AddToQueueForDisplay("", MESSAGE_SONG_TITLE); // remove 'now playing title'

  
//  interesting - these url play fine in chrome browser, but not here, unless you change https to http
//  strcpy(TargetUrl, "https://cosmo.shoutca.st/proxy/jubileefm/stream"); 
//  strcpy(TargetUrl, "https://stream.zeno.fm/89fkq77gb4duv");
//  strcpy(TargetUrl, "https://stream.zeno.fm/7ns6amt68qruv");
//  strcpy(TargetUrl, "https://stream.zeno.fm/yerp85sughwtv");
//  strcpy(TargetUrl, "https://stream.zeno.fm/bux0vqx79zquv");
//  strcpy(TargetUrl, "https://www.radioking.com/play/radio-paysan-fm-san/652837");
//  strcpy(TargetUrl, "https://betelgeuse.dribbcast.com/proxy/diaremefm?mp=/stream");
//  strcpy(TargetUrl, "https://webradio.tda.dz/ElBahdja_64K.mp3");
//  strcpy(TargetUrl, "https://webradio.tda.dz/Illizi_64K.mp3");
//  strcpy(TargetUrl, "https://webradio.tda.dz/Ouargla_64K.mp3");
//  strcpy(TargetUrl, "https://stream.zeno.fm/hfbgmx6rwrhvv");
//  strcpy(TargetUrl, "https://webradio.tda.dz/Chaine3_64K.mp3");
//  strcpy(TargetUrl, "https://webradio.tda.dz/Jeunesse_64K.mp3");
//  strcpy(TargetUrl, "https://radio-dzair.net/proxy/chaabia/chaabia");
//  strcpy(TargetUrl, "https://webradio.tda.dz/ElBahdja_64K.mp3");
//  strcpy(TargetUrl, "https://webradio.tda.dz/Coran_64K.mp3");
//  strcpy(TargetUrl, "https://stream.zeno.fm/1dkectoxpqgvv");
//  strcpy(TargetUrl, "https://betelgeuse.dribbcast.com/proxy/mamediarra?mp=/stream");
//  strcpy(TargetUrl, "https://playerservices.streamtheworld.com/api/livestream-redirect/SP_R2982692.aac");
//  strcpy(TargetUrl, "https://live.paineldj.com.br/proxy/radio1ago?mp=/stream");
//  strcpy(TargetUrl, "https://sc1.xdevel.com/ribeirabrava");
//  strcpy(TargetUrl, "https://stream.zeno.fm/85whutype7duv");
//  strcpy(TargetUrl, "https://stream.zeno.fm/e59pwkvm3reuv");
//  strcpy(TargetUrl, "https://stream.zeno.fm/2ee8m52mb"); 
//  strcpy(TargetUrl, "https://stream.zeno.fm/8pbaase2w2quv");
//  strcpy(TargetUrl, "http://178.19.58.119:1818/;"); // mime ="" ???? but plays when just assume MP3


  lapMillis = millis(); 
  Serial.printf("Now connect: %s\n", TargetUrl);
  stream.connectToHost(TargetUrl);  

  //  test - playing from SD card
  //  stream.connecttofile(SD, "/A01-Cor Bakker-Feel My Love.mp3"); // works, at SPI frequency 4000000 (stutters at default speed)
  //  stream.connecttofile(SD, "/B02-Raymond Groenewoud-Total Loss.mp3"); // works, but stutters 
  //  stream.connecttofile(SD, "/G03 Master KG-Jerusalema.mp3"); // works
  //  stream.connecttofile(SD, "/A09-Caro Emerald-Stuck.wav"); // adding WAV to enum codec_t in ESP32_VS1053_Strem.h made it play, but at half speed/stuttering
  //  stream.connecttofile(SD, "/A10-Caro Emerald-A Night Like This.mp3"); // 
  //  stream.connecttofile(SD, "/GLOBEMUSIC/JUKEBOX/A09-Caro Emerald-Stuck.wav"); // adding WAV to enum codec_t in ESP32_VS1053_Strem.h made it play, but at half speed/stuttering
  //  stream.connecttofile(SD, "/GLOBEMUSIC/JUKEBOX//G03 Master KG-Jerusalema.mp3"); // works also 

  if(stream.isRunning()) 
  { 

    // wait short time with volume 0 to avoid audible clicks/snippets between station switching
//    currentMillis = millis();
//    while (millis() - currentMillis < 250)
//    { stream.loop();
//    }

//    SetVolumeMapped(DataFromDisplay.volumevalue); // will also enable amplifiers

    currentMillis = millis();
    ConnectedInMillis = currentMillis - lapMillis;
    Serial.printf("Succesfully connected: %s -> time elapsed = %ld\n", TargetUrl, ConnectedInMillis);
    return_result = 1;
    strcpy(ActiveUrl, TargetUrl);
    PixelUpdate(0, 0x000000, 0x000000, 5000); // all off
  }
  else
  { Serial.printf("Could not connect: %s -> time elapsed = %ld\n", TargetUrl, (currentMillis = millis()) - lapMillis);
  }  
 
  return return_result;
}



// would be nice if it went gradually
void SetVolumeMapped(uint16_t volume)
{ static uint16_t prevvol;
  static uint8_t Actual_vs1053vol=0;
  static uint8_t New_vs1053vol;

  New_vs1053vol = map(volume, 0, 100, 60, 100); // range is 128db - more practical useable volume range of VS1053 is 68-100, 100-> 0dB and 68 being very quiet -40db
//  if(volume==0)New_vs1053vol=0;
  
  Serial.printf("New_vs1053vol (mapped parameter) = %d - Actual_vs1053vol = %d\n", New_vs1053vol, Actual_vs1053vol);

  if(bSetupCompleted)
  { if(volume>prevvol)
    { PixelUpdate(4, 0xFFFF00, 0x000000, 1000); // right turn yellow
    }  
    if(volume<prevvol)
    { PixelUpdate(3, 0xFFFF00, 0x000000, 1000); // left turn yellow
    }
  }  
  prevvol = volume;

  if(volume==0)Speakers(SPEAKERS_DELAYED_OFF);
  else // we have a volume
  { if(GlobeSettings.ee_internal_speakers == 1)
    { if(stream.isRunning())Speakers(SPEAKERS_ON); 
    }
    else Speakers(SPEAKERS_DELAYED_OFF); // no sound from speakers wanted
  } 


  //if(Actual_vs1053vol!=New_vs1053vol)
  { Serial.printf("New Volume adjusted to %d\n", New_vs1053vol);
    if(volume==0)New_vs1053vol = 0;
    Actual_vs1053vol=New_vs1053vol;
    stream.setVolume(Actual_vs1053vol);
  }  
}



void audio_showstation(const char* info) 
{ char *p;
  char stationname[256];
  // Serial.printf("Station: %s\n", info);
  // filter crap station naming, stick with our own name from the datebase
  if((p=strchr(info, '-')) !=0) *p=0; // split idotic long names that combine station & content 
  if(strlen(info)>30)return; // ignore ridiculous long names 
  if(strcasecmp(info, "no name")==0)return; // ignore meaningless names
  if(strcasecmp(info, "my station name")==0)return; // ignore meaningless names
  if(strcasecmp(info, "this is my server name")==0)return; // ignore meaningless names
  if(strcasecmp(info, "untitled")==0)return; // ignore meaningless names
  if(strcasecmp(info, "unnamed Server")==0)return; // ignore meaningless names
  if(strcasecmp(info, "offline")==0)return; // ignore meaningless names
  if(strcasecmp(info, "stream")==0)return; // ignore meaningless names
  if(strlen(info)<2)return; // ignore meaningless names
  
  strcpy(stationname, info); // keep a local copy to mess around with
  ReplaceHtmlEntities(stationname);
  //convertToExtAscii(stationname);

  strcpy(ActiveStationTitle, stationname);
  AddToQueueForDisplay(ActiveStationTitle, MESSAGE_STATION_NAME);
  
}

void audio_showstreamtitle(const char* info) 
{ char *p;

  strcpy(MetaDataSongTitle, info); // keep a local copy to mess around with

  //strcpy(MetaDataSongTitle, "VĚRA MARTINOVÁ - JEZDEC Z NEZNÁMA"); // test
  //strcpy(MetaDataSongTitle, "218.ÃÑ¡àÃÒäÁèà¡èÒàÅÂ - äÁèÁÕàËµØ¼Å - ã¤ÃÊÑ¡¤¹"); // test
  //strcpy(MetaDataSongTitle, "358. L'Heure de la délivrance du 06-08-21 L'offrande sacrificielle. PES. mp3"); //é = 65-cc-81 (65-e combined with cc-81 accent)
  //strcpy(MetaDataSongTitle, "358. L'Heure de la délivrance du 06-08-21 L'offrande sacrificielle. PES. mp3"); // é = c3-ag -> e9
  
  // Serial.printf("Stream title: %s\n", ReceivedSongTitle);
  // filter known crap messages
  if(strcasecmp(MetaDataSongTitle, "Now Playing info goes here")==0)return;
  if(strcmp(MetaDataSongTitle, " - ")==0)return;
  if(strcasecmp(MetaDataSongTitle, "adbreak")==0)return; // ignore meaningless names


  ReplaceHtmlEntities(MetaDataSongTitle);
  //convertToExtAscii(MetaDataSongTitle);

  //UTF8ToExtAscii(MetaDataSongTitle);
  if(strcmp(ActiveSongTitle, MetaDataSongTitle)!=0) // new one, send it to puck
  { strcpy(ActiveSongTitle, MetaDataSongTitle);
    AddToQueueForDisplay(ActiveSongTitle, MESSAGE_SONG_TITLE);
  }  
}


#define FAIL_INVALID_URL 1000
#define FAIL_INVALID_URL_LENGTH 1001
#define FAIL_HTTP_CLIENT_CREATE 1002
#define FAIL_HTTP_ESC_URL_BUFFER 1003
#define FAIL_CONNECT_FAILED 1004
#define FAIL_PLAYLIST_CANT_REDIRECT 1005
#define FAIL_NO_STREAM_HANDLE 1006
#define FAIL_PLAYLIST_NO_DATA 1007
#define FAIL_PLAYLIST_NO_URL 1008
#define FAIL_CANT_REDIRECT 1009
#define FAIL_NO_LOCATION_HEADER 1010
#define FAIL_LOOP_NO_HTTP_CLIENT 1011
#define FAIL_LOOP_HTTP_DISCONNECT 1012
#define FAIL_LOOP_CONNECTION_LOST 1013
#define FAIL_LOOP_STREAM_TIMEOUT 1014
#define FAIL_LOOP_EOF_NO_REMAINING_BYTES 1015
#define FAIL_LOOP_NO_FILE 1016
#define FAIL_LOOP_END_OF_FILE 1017
#define FAIL_LOOP_FILE_READ_FAIL 1018
#define FAIL_LOOP_BUFFER_EMPTY 1019


// results/problems as text
const char * fail_texts[] = {
   { "FAIL_INVALID_URL 1000"},
   { "FAIL_INVALID_URL_LENGTH 1001"},
   { "FAIL_HTTP_CLIENT_CREATE 1002"},
   { "FAIL_HTTP_ESC_URL_BUFFER 1003"},
   { "FAIL_CONNECT_FAILED 1004"},
   { "FAIL_PLAYLIST_CANT_REDIRECT 1005"},
   { "FAIL_NO_STREAM_HANDLE 1006"},
   { "FAIL_PLAYLIST_NO_DATA 1007"},
   { "FAIL_PLAYLIST_NO_URL 1008"},
   { "FAIL_CANT_REDIRECT 1009"},
   { "FAIL_NO_LOCATION_HEADER 1010"},
   { "FAIL_LOOP_NO_HTTP_CLIENT 1011"},
   { "FAIL_LOOP_HTTP_DISCONNECT 1012"},
   { "FAIL_LOOP_CONNECTION_LOST 1013"},
   { "FAIL_LOOP_STREAM_TIMEOUT 1014"},
   { "FAIL_LOOP_EOF_NO_REMAINING_BYTES 1015"},
   { "FAIL_LOOP_NO_FILE 1016"},
   { "FAIL_LOOP_END_OF_FILE 1017"},
   { "FAIL_LOOP_FILE_READ_FAIL 1018"},
   { "FAIL_LOOP_BUFFER_EMPTY 1019"}
};


// called from VS1053 driver
void audio_eof_stream(const char* info) 
{ char message[QUEUEMESSAGELENGTH];

  Speakers(SPEAKERS_DELAYED_OFF);
  Serial.printf("End of stream  -> %s\n", info);


  DataFromGlobe.D_QueueStationIndex = -1;

  // only next station or file if power is on
  if(bPowerStatus == true)
  { if(!bMusicMode)
    {   
      AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
      strcpy(ActiveUrl, "");
    }
    else
    { StartPlayFromSD(); // plays next
    }
  }    
}

void ReplaceHtmlEntities(char *name)
{ char *p, *s;
  int escapeArrayLen = 0;
  
  if((p=strchr(name, '&')) ==0) return; // no business here
  if((p=strstr(name, " & ")) != NULL) return; // no business here

  static const struct {
    const char* encodedEntity;
    const char decodedChar;
  } entityToChars[] = {
        {"&lt;", '<'},
        {"&gt;", '>'},
        {"&amp;", '&'},
        {"&quot;", '"'},
        {"&#", '*'},
  };

  escapeArrayLen = sizeof(entityToChars) / sizeof(entityToChars[0]);

  for(int j = 0; j < escapeArrayLen; j++)
  { if((p=strstr(name, entityToChars[j].encodedEntity))!=NULL)
    { // got one
      if(*(p+1) == '#') // ascii value follows
      { *p = ((*(p+2)-'0')*10) + (*(p+3)-'0'); // get the decimal 2-digit value after &#
        s = p + 5;
        while(*s)*++p=*s++;
        return;
      }
      else // one of the descriptional ones
      { *p = entityToChars[j].decodedChar;
        s = p + strlen(entityToChars[j].encodedEntity);
        while(*s)*++p=*s++;
        return;
      }  
    }
  }
  Serial.printf("Unsupported html character name %s\n", name);
}

void convertToExtAscii(char *target)
{ char *p;
  char *p2;
  p = target;
  p2 = target;
  while (*p)
  { if (*p == 0xc2)
    { p++; *p2++ = *p;
    }
    else if (*p == 0xc3)
    { p++; *p2++ = *p + (0xC0 - 0x80);
    }
    else *p2++ = *p;
    p++;
  }
  *p2 = 0;
}

//https://nl.wikipedia.org/wiki/UTF-8

void UTF8ToExtAscii(char *target)
{ char *p;
  char *p2;
  char c;
  uint16_t len;
  p = target;

  //strcpy(p, "VĚRA MARTINOVÁ - JEZDEC Z NEZNÁMA");
  //strcpy(p, "218.ÃÑ¡àÃÒäÁèà¡èÒàÅÂ - äÁèÁÕàËµØ¼Å - ã¤ÃÊÑ¡¤¹");

  len = strlen(p);
  Serial.printf("UTF8ToExtAscii() %s -> %d bytes\n", p, len);
  while(*p!=0)
  { again:
    //while(1)
    len = strlen(p);
    //Serial.printf("len=%d\n", len);
    if(len==0)break;

    { if(len>3)
      { if( ((*(p+0) & 0b11111000) == 0b11110000) && ((*(p+1) & 0b11000000) == 0b10000000) && ((*(p+2) & 0b11000000) == 0b10000000) && ((*(p+3) & 0b11000000) == 0b10000000) )
        { // U+010000..U+10FFFF skip this, can't print that at all
          memmove(p, p+4, len-3);
          //Serial.printf("4:%s\n", p);
          goto again;
        }
      }
      

      if(len>2) 
      { if( ((*(p+0) & 0b11110000) == 0b11100000) && ((*(p+1) & 0b11000000) == 0b10000000) && ((*(p+2) & 0b11000000) == 0b10000000) )
        { // U+0800..U+D7FF en U+E000..U+FFFF skip this, can't print that at all
          memmove(p, p+3, len-2);
          //Serial.printf("3:%s (%d)\n", p, len);
          goto again;
        }
      }

      if(len>1000) 
      { Serial.printf("len>1A:%s = %02X\n", p, *p);
        Serial.printf("len>1B:%s = %02X\n", p, *(p+1));
        if ( ((*(p+0) & 0b11100000) == 0b11000000) && ((*(p+1) & 0b11000000) == 0b10000000) )
        { // U+0080..U+07FF 110bbbaa 10aaaaaa
          //Serial.printf("len>1C:%s = %02X\n", p, *(p+1));

          if( (*(p+0) & 0b00011100) ) // > 0xff skip this, can't print that at all
          { memmove(p, p+2, len-1);
            //Serial.printf("2:%s\n", p);
            goto again;
          }
          else // U+0080..U+00FF 110bbbaa 10aaaaaa
          { c = (*(p+0)<<6) | (*(p+1)& 0b00111111);  
            memmove(p, p+1, len);
            *p = c;
            //Serial.printf("1:%s = %02X\n", p, *p);
            //Serial.printf("char %c\n", *p);
            
            p++;
            goto again;
          }
        }
      }
      // U+0000..U+007F // do nothing
      //Serial.printf("Single:%s = %02X\n", p, *p) ;
      p++;
    }
  }

  p = target;
  len = strlen(p);
  Serial.printf("UTF8-decoded -> %s -> %d bytes\n", p, len);
}




// called from encoder task
void checkSpeakerToggleButton(void)
{ static uint32_t previous_event = millis();
  static bool previous_button_state = digitalRead(SPEAKER_TOGGLE_PIN);
  bool button_state;
  uint32_t state_timer;
  char message[64];
  // check for button press
  button_state = digitalRead(SPEAKER_TOGGLE_PIN);

  state_timer = millis() - previous_event;

  
  if(previous_button_state != button_state)
  { previous_button_state = button_state;
    if( (state_timer > 50) && (state_timer < 1000) && (button_state == HIGH))
    { Serial.println("Short Press Button");
      if(Timer100msSerialCanBeOpened)
      { Serial.printf("Serial port open!\n");
        Timer100msSerialIsOpen = 300; // 30 seconds
        return;
      }

      if(GlobeSettings.ee_internal_speakers == 1)
      { GlobeSettings.ee_internal_speakers = 0;
        Speakers(SPEAKERS_OFF);
        Serial.println("SPEAKERS_OFF by button press");
      }
      else  
      { GlobeSettings.ee_internal_speakers = 1;
        if(stream.isRunning())
        { Speakers(SPEAKERS_ON);
          Serial.println("SPEAKERS_ON by button press");
        }  
      }
      // inform puck display
      // tell display what our stored values are for volume and tone control and speaker status
      sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
      AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
    }
    previous_event = millis();
    state_timer = 0;
  }

  if((state_timer > 5000) && (button_state == LOW))
  { previous_event = millis();
    state_timer = 0;
    Serial.println("Long Press Button");
    digitalWrite(BT_CONNECT_PIN, LOW); 
    delay(500); 
    digitalWrite(BT_CONNECT_PIN, HIGH);  
    PlaySoundBite((uint8_t *)mp3_bluetoothconnecting, sizeof(mp3_bluetoothconnecting), 50);
  }
}     

// needs attention - lot of fuzzy code here
void PlaySoundBite(uint8_t *soundbite, unsigned long long length, uint16_t volumeoveride)
{  uint16_t volume_to_use;
  
  if((volumeoveride>0) && (volumeoveride<GlobeSettings.ee_volume))volume_to_use = volumeoveride;
  else volume_to_use = GlobeSettings.ee_volume;

  if(volume_to_use<10)volume_to_use=10; 
  if(volume_to_use>70)volume_to_use=70; 
  SetVolumeMapped(volume_to_use); // also enables amplifiers

  // extra reset, or else sometimes an old snippet of station music is played before the power down tune is played
  //chunkplayer.switchToMp3Mode(); // does a softReset() also

  Speakers(SPEAKERS_ON);
  // delay(250); // move to Speakers function
  //   chunkplayer.playChunk((uint8_t *)mp3_silence_1_sec, sizeof(mp3_silence_1_sec));
   
  stream.playChunk(soundbite, length);
  //delay(1000);
  //  chunkplayer.playChunk((uint8_t *)mp3_silence_1_sec, sizeof(mp3_silence_1_sec));
  //  chunkplayer.playChunk((uint8_t *)mp3_silence_1_sec, sizeof(mp3_silence_1_sec));
  //delay(1000);
  delay(250);
  SetVolumeMapped(GlobeSettings.ee_volume); // will enable amplifiers (if speaker switch == on) if radio stream running, else amplifiers off
  // did this soundbite played during radio playing (like battery low warning)
  if(stream.isRunning())return; // leave speakers on, radio stream continues
  // all done 
  Speakers(SPEAKERS_DELAYED_OFF);
}

void GlobePowerDown(void)
{ uint16_t restore_volume;
  bPowerStatus = false;
  EEPROM.get(0x0, GlobeSettings); // need stored volume settings
  Serial.printf("POWERDOWN - volume -> %d \n", GlobeSettings.ee_volume);
  SetVolumeMapped(GlobeSettings.ee_volume); 
  stream.stopSong();
  strcpy(ActiveStationTitle, "");
  strcpy(ActiveSongTitle, "");
  AddToQueueForDisplay(ActiveStationTitle, MESSAGE_STATION_NAME);
  AddToQueueForDisplay(ActiveSongTitle, MESSAGE_SONG_TITLE);
  loopMQTT();
  delay(500); // really?? can't remember
  DataFromGlobe.D_QueueStationIndex = -1;
  
  PlaySoundBite((uint8_t *)mp3_shutdown, sizeof(mp3_shutdown), 0); 
  Serial.printf("POWERDOWN completed\n");

  digitalWrite(BT_POWER_PIN, LOW); // turn off BT module
}

void GlobePowerUp(void)
{ char message[QUEUEMESSAGELENGTH]; 

  bPowerStatus = true;
  EEPROM.get(0x0, GlobeSettings); // need stored volume settings
  Serial.printf("POWERUP - volume -> %d \n", GlobeSettings.ee_volume);
  // tell display what our stored values are for volume and tone control
  sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
  AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
  AddToQueueForDisplay(build_timestamp, MESSAGE_GLOBE_BUILD_DATE_TIME);
  
  SetVolumeMapped(GlobeSettings.ee_volume); 
  AddToQueueForDisplay("Globe Just Booted", MESSAGE_GLOBE_WANTS_CURRENT_STATION);
      
  PlaySoundBite((uint8_t *)mp3_startup, sizeof(mp3_startup), 0);
  Serial.printf("POWERUP done\n");

  sprintf(message, "%d-%d-%d", GlobeSettings.btmodule_switchable, GlobeSettings.btmodule_power_on, GlobeSettings.btmodule_installed);
  AddToQueueForDisplay(message, MESSAGE_DISPLAY_BT_SWITCHABLE_STATE);

  if(GlobeSettings.btmodule_power_on) digitalWrite(BT_POWER_PIN, HIGH);  // turn on
  else digitalWrite(BT_POWER_PIN, LOW);  // turn off

}


#define SNIPLENGHT 1500
uint32_t PlayWhile(uint8_t *soundbite, unsigned long long length, bool playwhile)
{  static size_t bytes_to_play;
   static uint32_t snippets_to_play;
   static uint32_t snippet = 0;
   static bool oldplaywhile = 0;
   unsigned long snippetMs;

   Serial.printf("oldplaywhile =%d playwhile = %d\n", (int)oldplaywhile, (int)playwhile);

   if(oldplaywhile != playwhile)
   { Serial.printf("playwhile = %d\n", (int)playwhile);
     oldplaywhile = playwhile;
     if(playwhile) // start or continue
     { bytes_to_play = length;
       snippets_to_play = bytes_to_play / SNIPLENGHT;
       snippet = 0; // start at begin of soundbite
       Serial.printf("snippets_to_play = %ld\n", snippets_to_play);
     
     }
     else
     { snippets_to_play = 0;
       Speakers(SPEAKERS_DELAYED_OFF);
     }
   }

   if(snippet <= snippets_to_play)
   { SetVolumeMapped(DataFromDisplay.volumevalue); 
     if(snippet < snippets_to_play) // full size snippets
     { Serial.printf("snippet = %ld\n", snippet);
       if(GlobeSettings.ee_internal_speakers)Speakers(SPEAKERS_ON);
       snippetMs = millis();
       stream.playChunk(&soundbite[(snippet * SNIPLENGHT)], SNIPLENGHT); 
       
       
       Serial.println(millis() - snippetMs);
       Speakers(SPEAKERS_DELAYED_OFF);
       snippet++;
     }
     else // last partial snippet
     { Serial.printf("last partial snippet = %ld\n", snippet);
       if(bytes_to_play % SNIPLENGHT)
       { if(GlobeSettings.ee_internal_speakers)Speakers(SPEAKERS_ON);
         stream.playChunk(&soundbite[(snippet * SNIPLENGHT)], (bytes_to_play % SNIPLENGHT)); 
         Speakers(SPEAKERS_DELAYED_OFF);
         snippet++;
       }
       oldplaywhile = 0;
     }
   }
   return (snippets_to_play - snippet);
}




void InitializeGlobeSettings(void)
{ if(GlobeSettings.magicnumber == 123456)
  { Serial.println("EEprom data seems OK");
    return;
  }
  Serial.println("Initialize EEprom with some default values");
  GlobeSettings.ee_bass = 0;
  GlobeSettings.ee_treble = 0;
  GlobeSettings.ee_volume = 40;
  GlobeSettings.ee_internal_speakers = 1;
  memset(GlobeSettings.ee_mqttserver_ip, 0 , 4);
  strcpy(GlobeSettings.google_api_key, "0");
  memset(GlobeSettings.ee_puckmac, 0 , 6); 
  strcpy(GlobeSettings.HomeCountryCode, "NL");
  strcpy(GlobeSettings.ssid, "SSID");
  strcpy(GlobeSettings.password, "PASSWORD");
  GlobeSettings.serialnumber = 50;
  GlobeSettings.magicnumber = 123456;
  GlobeSettings.btmodule_installed = 1;
  
  if(CheckIfBTSwitchable())GlobeSettings.btmodule_switchable = 1;
  else GlobeSettings.btmodule_switchable = 0;
  
  GlobeSettings.btmodule_power_on = 0;
  strcpy(GlobeSettings.open_weather_map_api_key, "0");
  EEPROM.put(0x0, GlobeSettings);
  EEPROM.commit();
  Serial.println("Eeprom initialized & saved..");
}

bool CheckIfBTSwitchable(void)
{ bool result;
  pinMode(BT_POWER_PIN, INPUT_PULLUP);
  delay(10);
  if(digitalRead(BT_POWER_PIN))
  { // no switch circuit with pulldown 4K7 resistor present
    Serial.printf("BT module switch circuit not present\n");
    result = false;
  }
  else
  { Serial.printf("BT module switch circuit is present\n");
    result = true;
  }
  pinMode(BT_POWER_PIN, OUTPUT); // test done, make it an output now
  return result;
}

void Speakers(uint8_t mode)
{ static uint8_t prev_mode =123;
  if(prev_mode != mode)
  { prev_mode = mode;
    if(mode == SPEAKERS_OFF)
    { digitalWrite(MUTE_AMPLIFIERS, 1); // turn off immediately
      Serial.println("Speakers OFF");
    }
    else if(mode == SPEAKERS_DELAYED_OFF)
    { SpeakerOffAfter25mS = 200; // turn of after 200 x 25mS -> 5000mS delay
    }
    else if(mode == SPEAKERS_ON)
    { SpeakerOffAfter25mS = 0;
      Serial.println("Speakers ON");
      digitalWrite(MUTE_AMPLIFIERS, 0); // turn on immediately
      //delay(250); 
    }
  }  
}

// Called when codec is detected
void codecCallback(const char *codec)
{
    Serial.printf("codec: %s\n", codec);
    SetVolumeMapped(DataFromDisplay.volumevalue); // will also enable amplifiers
}


// Called when bitrate is detected (cbr) and changes (vbr)
void bitrateCallback(uint32_t bitrate)
{ Serial.printf("bitrate: %lu kbps\n", bitrate);
}

// called from VS1053 driver
void audio_fail(void) 
{ Serial.printf("Decoder failed to sync\n");
}  

// called from VS1053 driver
void audio_error(const char *error) 
{ Serial.printf("Error from VS1053 -> %s\n", error);
  VS1053_connectResult = error;
}



// EOF