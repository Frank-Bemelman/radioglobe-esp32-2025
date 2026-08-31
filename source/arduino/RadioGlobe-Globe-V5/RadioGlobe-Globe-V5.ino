// in arduino library manager........
// install libs as shown here https://randomnerdtutorials.com/esp32-wi-fi-manager-asyncwebserver/
// this is for the wifi manager portal

// libraries needed in Arduino ide
// Arduinojson by Benoit - v7.4.3
// https://github.com/baldram/ESP_VS1053_Library download zip file and install
// AS5600 by Rob Tilaart - v0.6.7
// Adafruit Neopixel 1.15.5
// WiFiManager by tzapu V2.0.17
// Async TCP by ESP32Async V3.50
// ESPAsyncWebServer by ESP32Async V3.12.0 
// ESP_SSLClient by Mobitz V 3.1.3
// home-assistant-integration Version  by Dawid 2.1.0

// arduino ide settings 
// esp32 3.1.11 board library
// esp32s3 dev module
// tools -> flash size 16MB
// tools -> partition 16MB 3MB APP 9.9MB FATFS
// tools -> PSRAM OPI PSRAM 

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
const char build_label[] PROGMEM =  BUILD_LABEL;
const char build_timestamp[] PROGMEM =  BUILD_TIMESTAMP;
const char build_timestamp_only[] =  BUILD_TIMESTAMP_ONLY;
extern uint8_t UpdateFirmware(uint8_t state); // start with 1 for a full date/time check and update
extern bool CheckForNewGlobeUpdate(void);
bool UpdateAvailable = false;

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESPmDNS.h>
#include <WiFi.h>
//#include "ESP32FtpServer.h"
#include "SimpleFTPServer.h"
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
#include "countrycodes.h" 
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

// later PCB boards have the SD card integrated in the design
#define SD_MOSI 41 // green (pin 3 SD module)
#define SD_CLK 40  // yellow (pin 4 SD module)
#define SD_MISO 38 // orange (pin 5 SD module)
#define SD_CS 39 // blue (pin 2 SD module)
// brown GND (pin 6 SD module)
// red +3V3 (pin 1 SD module)



SPIClass *hspi = 0;

ESP32_VS1053_Stream stream;
//VS1053 chunkplayer(VS1053_CS, VS1053_DCS, VS1053_DREQ); // made a playChunk member in ESP32_VS1053_Stream library

// AS5600 encoders esp32-S3-N16R8 pin arrangement
const int SCL_1 = 4; // &wire as5600_lat grey NS LAT
const int SDA_1 = 5; // &wire as5600_lat brown NS LAT
const int SCL_2 = 6; // &wire as5600_lon grey EW LON
const int SDA_2 = 7; // &wire as5600_lon brown EW LON

#define MUTE_AMPLIFIERS 14 // pin number
#define SPEAKERS_OFF 0
#define SPEAKERS_ON 1
#define SPEAKERS_DELAYED_OFF 3

AS5600 as5600_lat(&Wire); // NS LAT Encoder
AS5600 as5600_lon(&Wire1); // EW LON Encoder

static uint32_t startMillis;
static uint32_t lapMillis;
static uint32_t currentMillis;
static uint32_t ConnectedInMillis;
static uint32_t StationConnectedAtMillis;


int16_t PrevTick = 0;
bool bPowerStatus = true;
bool bVolumeToneControlsActive = false;

bool bUpAndRunning = false;
bool bSetupCompleted = false;
bool bEncoderNewPosition = false;
bool bEncoderKillStation = false;
bool bGlobeStable = false;
uint16_t Timer100msSerialCanBeOpened = 0; // set to 5 seconds after 5 taps on touch
uint16_t Timer100msSerialIsOpen = 0; // set to 30 seconds after speaker switch pressed
uint16_t SpeakerOffAfter25mS = 0;

char TargetUrl[QUEUEMESSAGELENGTH] = ""; // most recent url requested by display
char ActiveUrl[QUEUEMESSAGELENGTH] = ""; // most recent url connected to
char PrevUrl[QUEUEMESSAGELENGTH] = ""; // most recent url connected to


bool Tuning = false;
bool bSomethingPlays = false;

#define SPEAKER_TOGGLE_PIN 21 // pushbutton behind globe support for speaker toggle and bleutooth connect
#define BT_CONNECT_PIN 17 // output connected to BT module to force a BT connect
#define BT_POWER_PIN 2
#define PORTALSWITCH_PIN 20 // pushbutton located on the back, short press  opens portal to connect to smartphone and configure wifi and puck mac, long press > 3 seconds is resets,all settings

#include <EEPROM.h>
#define EEPROM_SIZE 512
struct eepromData 
{ int16_t OffsetLat;
  int16_t OffsetLon;
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
  uint16_t globe_sd_gb;
  char spare_data[];
};
eepromData GlobeSettings; 

uint8_t rtone[4] = {0, 3, 14, 0}; // tone control register of VS1053 responding to display controls for bass & treble, values of 0-15

uint32_t RefreshRatesCountDownTimer = 0;
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

char VS1053_CallBackLastError[QUEUEMESSAGELENGTH];

extern void setupwebserver(void);


char PrevStreamType[64]="";
char StreamType[64]="";
char StreamCodec[16]="";
char StreamBitrate[16]="";
char StreamFilllevel[16]="";
bool bStreamTypeNeedsUpdated = false;



volatile uint32_t GlobalTicker100mS = 0;
volatile bool bFlag100mS = false; // will always reset to true within 100mS by timer interrupt

void IRAM_ATTR onGlobalTimer100ms(void* arg) {
    GlobalTicker100mS++;
    bFlag100mS = true; 
}

void SetupGlobalTimer100mS(void) {
    const esp_timer_create_args_t timer_args = {
        .callback = &onGlobalTimer100ms,
        .name = "global_100ms_ticker"
    };

    esp_timer_handle_t global_timer;
    esp_timer_create(&timer_args, &global_timer);
    // start timer every 100.000 microseconden -> 100 ms
    esp_timer_start_periodic(global_timer, 100000); 
}

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

bool bPortalOpened = false; // to freeze loop_esp_now(); during portal, so wifi channel is more quiet

// stacksize is standard 8K
// as experiment, tried to increase it to see if it made any differences regarding instabilities I had at a certain moment. 
// can't say it made much difference
// so I took it out again
// 20AUG26 tried back 8, Rad Suravi stutters
// back 16
//SET_LOOP_TASK_STACK_SIZE(16*1024);
//SET_LOOP_TASK_STACK_SIZE(8*1024); // weer eens een tijdje op 8

VS1053 *franks_vs1053;

void setup()
{ char message[64];
  startMillis = millis();
  // 6AUG26  globe freezes wehen running a heavy testurl job,  after 2-12 hours of testing 1 station every 1-3 seconds
  // so I direct malloc to PSRAM for larger>1024, rather than ram. Also increase stacksize from 8K to 16K, see SET_LOOP_TASK_STACK_SIZE macro above
  // heap_caps_malloc_extmem_enable(1024); // trying without, 27AUG, after crashes with AU station https://tcom-s1.tcom.net.au/2ten after Ringbuffer empty and reporting to puck in eof_stream()
  // as per 28AUG 12:10 no new crashes seen sofar, after ringbuffer empty goes to next station in roller

  Serial.begin(115200);

  extern size_t getArduinoLoopTaskStackSize(void);
  Serial.printf("Stack size = %d\n", getArduinoLoopTaskStackSize());
  TaskHandle_t CurrentTask = xTaskGetCurrentTaskHandle();
  TaskStatus_t TaskStatus;
  vTaskGetInfo(CurrentTask, &TaskStatus, pdTRUE, eInvalid);
  Serial.printf("Taskname: %s\n", TaskStatus.pcTaskName);
  Serial.printf("Free Stack Size: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
  // 6AUG26  een kijken of de globe minder vastloopt (bij url test na 12 uur soms)

  
  SetupGlobalTimer100mS(); // give us a steady uint32_t GlobalTicker100mS upcounter and bFlag100mS going true always every 100mS

  // replace !!!! characters in fetched api key (from secrets.h, where it is stored slighly corrupted)
  // basically to disguise apikey in final firmware
  google_api_key[0]='A';
  google_api_key[1]='I';
  google_api_key[2]='z';
  google_api_key[3]='a';


  

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0x0, GlobeSettings);

  InitializeGlobeSettings(); // at first run, initialize eeprom with some reasonable values

  if(CheckIfBTSwitchable())GlobeSettings.btmodule_switchable = 1;
  else GlobeSettings.btmodule_switchable = 0;
  

  xTaskCreatePinnedToCore(
                    TaskTouch,   /* Task function. */
                    "TaskTouch",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core */   
  
  PixelUpdate(0, 0xFFFFFF, 0x000000, 10000); // solid white                     

  // BT module is switchable
  pinMode(BT_CONNECT_PIN, OUTPUT);
  if(GlobeSettings.btmodule_power_on) digitalWrite(BT_POWER_PIN, HIGH);  // turn on
  else digitalWrite(BT_POWER_PIN, LOW);  // turn off
  
  pinMode(MUTE_AMPLIFIERS, OUTPUT);
  Speakers(SPEAKERS_OFF);

  pinMode(SPEAKER_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(PORTALSWITCH_PIN, INPUT_PULLUP); // input to button for opening portal

  Serial.printf("GlobeSettings size = %d bytes\n", sizeof(GlobeSettings));

  Serial.printf("Sofar %dmS spend\n", (millis()-startMillis));


//  pinMode(VS1053_CS, OUTPUT);
//  pinMode(VS1053_DCS, OUTPUT);
  pinMode(VS1053_RESET, OUTPUT);

//  digitalWrite(VS1053_CS, HIGH); 
//  digitalWrite(VS1053_DCS, HIGH); 
  digitalWrite(VS1053_RESET, LOW); 

  delay(53);   
  //pinMode(SPI_CLK_PIN, OUTPUT);
  //pinMode(SPI_MOSI_PIN, OUTPUT);
  //digitalWrite(SPI_CLK_PIN, LOW);
  //digitalWrite(SPI_MOSI_PIN, LOW);
  //delay(100);

  digitalWrite(VS1053_RESET, HIGH); 
  delay(20);          

  // Start SPI bus, takes less than a mS
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  

  // Initialize the VS1053 decoder, takes quite long, 3762mS
  startMillis = millis();
  if (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !stream.isChipConnected()) 
  { Serial.println("Decoder not running - this is bad");
  }
  else Serial.printf("VS1053 up & running after %dmS\n", (millis()-startMillis));
  // Set the codec callback
  stream.setCodecCB(codecCallback);
  // Set the bitrate callback
  stream.setBitrateCB(bitrateCallback);   
  // Set the station name callback
  stream.setStationCB(audio_showstation);
  // Set the stream metadata callback
  stream.setInfoCB(audio_showstreamtitle);
  // Set the EOF callback
  stream.setEofCB(audio_eof_stream);    
  // Set the error callback
  stream.setErrorCB(VS1053error);    
  // Set the filllevel callback
  stream.setFilllevelCB(filllevelCallback);   
  
  //stream.stopSong(); // had a _vs1053->switchToMp3Mode(); inside that first, otherwise my VS1053 doesn't always boot well
  // original code doesn't have that, so I do it myself now, to keep code
  franks_vs1053 = stream.getVS1053pointer();
  franks_vs1053->switchToMp3Mode();
  franks_vs1053->loadDefaultVs1053Patches(); 
  #define SCI_CLOCKF 0x03
  franks_vs1053->writeRegister(SCI_CLOCKF, 0x6000); 



  // maybe if we want to poll vu meter sound level
  //if (franks_vs1053->getChipVersion() == 4)
  //  {
  //      log_d("Patching vs1053 firmware");
        //_vs1053->loadDefaultVs1053Patches(); 
  //      _vs1053->loadUserCode(PATCHES_FLAC, PATCHES_FLAC_SIZE);
  //  }

  //Speakers(SPEAKERS_ON); 
  //stream.forceVolume(75);
  //stream.playChunkNonBlocking((uint8_t *)mp3_happy_ping, sizeof(mp3_happy_ping), false);
  //stream.playChunkNonBlocking((uint8_t *)mp3_radio_tuning, sizeof(mp3_radio_tuning), true);
  //while(stream.isRunning()) stream.loop();



  PlaySoundBite((uint8_t *)mp3_happy_ping, sizeof(mp3_happy_ping), 0); 

  // start SD card
  hspi = new SPIClass (HSPI);
  hspi->begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  CheckSD(); // also updates GlobeSettings.globe_sd_gb and eeprom update if need be

  GlobeSettings.ssid[sizeof(GlobeSettings.ssid)-1]=0;
  //Serial.printf("Eeprom ssid: %s\n", GlobeSettings.ssid);
  GlobeSettings.password[sizeof(GlobeSettings.password)-1]=0;
  //Serial.printf("Eeprom password: %s\n", GlobeSettings.password);

  Serial.printf("Eeprom stored OffsetLat: %d\n", GlobeSettings.OffsetLat);
  Serial.printf("Eeprom stored OffsetLon: %d\n", GlobeSettings.OffsetLon);

  Serial.printf("Mqtt Server set at %d.%d.%d.%d\n", (uint16_t)GlobeSettings.ee_mqttserver_ip[0], (uint16_t)GlobeSettings.ee_mqttserver_ip[1], (uint16_t)GlobeSettings.ee_mqttserver_ip[2], (uint16_t)GlobeSettings.ee_mqttserver_ip[3]);

  // Initialise as5600_lat and as5600_lon connection
  Wire.begin(SDA_1, SCL_1);
  Wire1.begin(SDA_2, SCL_2);
  
    // NS encoder
  as5600_lat.begin();  //  set direction pin.
  as5600_lat.setDirection(AS5600_CLOCK_WISE);
  as5600_lat.setOffset(GlobeSettings.OffsetLat * AS5600_RAW_TO_DEGREES);
  Serial.print("Connect Encoder LAT: ");
  Serial.println(as5600_lat.isConnected() ? "true" : "false");

  as5600_lon.begin();  //  set direction pin.
  as5600_lon.setDirection(AS5600_CLOCK_WISE);
  as5600_lon.setOffset(GlobeSettings.OffsetLon * AS5600_RAW_TO_DEGREES);  
  Serial.print("Connect Encoder LON: ");
  Serial.println(as5600_lon.isConnected() ? "true" : "false");

  Serial.print("readangle NS = "); Serial.println(as5600_lat.readAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("readangle EW = "); Serial.println(as5600_lon.readAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("ReadAGC NS = "); Serial.println(as5600_lat.readAGC());
  Serial.print("ReadAGC EW = "); Serial.println(as5600_lon.readAGC());

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
//  delay(1000);
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
    // play it loud enough to be noticed
    PlaySoundBite((uint8_t *)mp3_smartphone_portal, sizeof(mp3_smartphone_portal), 40); 
    AutoConnect(); // wifi auto connect with portal if need be
    if(WiFi.isConnected())
    { Serial.println("Connected with WifiManager stored credentials");
    }
  }
  
  if(WiFi.isConnected())
  { Serial.print("Wifi connected to IP: ");
    Serial.println(WiFi.localIP());
    PixelUpdate(0, 0x00FF00, 0x000000, 5000); // solid green
    configTime(0, 0, "pool.ntp.org");
    uint32_t huidigIntervalMs = sntp_get_sync_interval();
    uint32_t intervalMinuten = huidigIntervalMs / 1000 / 60;
    Serial.printf("Ingesteld SNTP interval: %u ms (%u minuten)\n", huidigIntervalMs, intervalMinuten);
    delay(2000);
  }
  else
  { Serial.println(" wifi NOT connected.");
    // play it loud enough to be noticed
    PlaySoundBite((uint8_t *)mp3_wifidisconnected, sizeof(mp3_wifidisconnected), 40); 
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
  snprintf(message, sizeof(message), "%02X:%02X:%02X:%02X:%02X:%02X", GlobeMac[0], GlobeMac[1], GlobeMac[2], GlobeMac[3], GlobeMac[4], GlobeMac[5]);
  AddToQueueForDisplay(message, MESSAGE_GLOBE_MAC);
  snprintf(message, sizeof(message), "%d", (uint32_t) WiFi.channel());
  AddToQueueForDisplay(message, MESSAGE_SET_PUCK_WIFI_CHANNEL);
  

  if(GlobeSettings.btmodule_installed == 0)Serial.println("No Bluetooth module installed");
  else Serial.println("Has Bluetooth module installed");
  if(GlobeSettings.btmodule_switchable == 0)Serial.println("Bluetooth is NOT switchable");
  else Serial.println("Bluetooth is switchable");

  
  sprintf(message, "%d-%d-%d", GlobeSettings.btmodule_switchable, GlobeSettings.btmodule_power_on, GlobeSettings.btmodule_installed);
  AddToQueueForDisplay(message, MESSAGE_DISPLAY_BT_SWITCHABLE_STATE);

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
  RefreshRatesCountDownTimer = 0;
  bSetupCompleted = true;
  
  Serial.printf("Started with Hostname %s\n", WiFi.getHostname()); 
  sprintf(message, "%s", WiFi.getHostname());
  if(!MDNS.begin(message))
  { Serial.printf("MDNS setup of %s.local failed!\n", message);
  }
  else Serial.printf("MDNS setup of %s.local succes!\n", message);
    

  if(GlobeSettings.globe_sd_gb !=0)
  { // tell puck size of card
    sprintf(message, "%d GB", GlobeSettings.globe_sd_gb);
    AddToQueueForDisplay(message, MESSAGE_GLOBE_SD_GB);
    ftp.begin("globe", "globe");
    Serial.println("FTP gestart!");
    Serial.println(WiFi.localIP());
  }  
  else // no SD card
  { // tell puck
    AddToQueueForDisplay("0 GB", MESSAGE_GLOBE_SD_GB);
  }
  
  if(GlobeSettings.globe_sd_gb !=0)
  { setupwebserver(); // that's all
    sprintf(message, "%s.local", WiFi.getHostname());
    AddToQueueForDisplay(message, MESSAGE_GLOBE_HOSTNAME);
  }  
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
  static char StartThisStation[QUEUEMESSAGELENGTH] =""; // holds the url requested by display
  static bool bFirst = true;
  static uint32_t LoopTicker100mS = 0;
  
  static uint32_t nowmillis;
  static uint32_t prevmillis;
  static uint32_t lapmillis;

  if(GlobeSettings.globe_sd_gb != 0)ftp.handleFTP();
   
  loop2(); // checks portal button in advanced.ino
  
  nowmillis = millis();
  lapmillis = nowmillis - prevmillis;
  if(lapmillis>30)Serial.printf("main loop(); took = %dmS\n", lapmillis);
  prevmillis = nowmillis;

  
  stream.loop(); // keeps the VS1053 going..
  
  nowmillis = millis();
  lapmillis = nowmillis - prevmillis;
  if(lapmillis>50)Serial.printf("stream.loop(); took = %dmS\n", lapmillis);
  prevmillis = nowmillis;

  if(bMqttActivated==1234)loopMQTT();
  
  if(UpdateState) // update check in one step at the time
  { UpdateState = UpdateFirmware(UpdateState);
  } 

  if(bFlag100mS)
  { bFlag100mS = false; 
    LoopTicker100mS = GlobalTicker100mS;
    if(!bPortalOpened && lapmillis<40) // time just spent in stream.loop();
    { //loop_esp_now();
      nowmillis = millis();
      lapmillis = nowmillis - prevmillis;
      if(lapmillis>10)Serial.printf("loop_esp_now(); took = %dmS\n", lapmillis);
      prevmillis = nowmillis;
    }
    if((LoopTicker100mS % 10)==0) // every seconds or so
    { if(bStreamTypeNeedsUpdated) // update done once per second
      { AddToQueueForDisplay(StreamType, MESSAGE_STATUS_LINE);
        bStreamTypeNeedsUpdated = false;
      }
    }
  }  
  nowmillis = millis();
  lapmillis = nowmillis - prevmillis;
  if(lapmillis>10)Serial.printf("loop_esp_now(); took = %dmS\n", lapmillis);
  prevmillis = nowmillis;



  if((LoopTicker100mS % 50)==0) // every 5 seconds or so
  { if(WiFi.status() != WL_CONNECTED) 
    { if(connection_lost_counter++>500)
      { Serial.println("Unable to recover Wifi after 2500 seconds...");
        delay(100);
        ESP.restart(); 
      }
      Serial.printf("Reconnecting to Wifi attempt %hu...\n", connection_lost_counter);
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      delay(100); 
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false); 
      WiFi.begin(GlobeSettings.ssid, GlobeSettings.password);
    }
    else
    { if (connection_lost_counter > 0)
      { AddToQueueForDisplay("WIFI -> Reconnected After Connection Lost", MESSAGE_CONNECTTOHOST_FAILURE);
      }
      connection_lost_counter = 0;
    }
  }

  if(Timer100msSerialIsOpen) // only when enabled, as this works when serial port is actually connected, but could hang when there is no usb cable to PC
  { while(Serial.available()) 
    { Serial.print(".");
      incomingChar = Serial.read();
      if(incomingChar == '\n')
      { Serial.print("You typed this: ");
        Serial.println(receivedMessage);
        Serial.printf("S-GlobeSettings.btmodule_installed = %d\n", GlobeSettings.btmodule_installed);
        Serial.printf("S-GlobeSettings.btmodule_switchable = %d\n", GlobeSettings.btmodule_switchable);
        strcpy(text, receivedMessage.c_str());
        if((p = strstr(text, "BI")) != 0) 
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
        if((p = strstr(text, "BS")) != 0) 
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
        if((p = strstr(text, "SN")) != 0) 
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
  
  // process one or more queued messages from display
  while(FromDisplay.QueueIndexIn != FromDisplay.QueueIndexOut) // we have to catch up with new messages
  {  //Serial.printf("Messages from display pending %d\n", FromDisplay.QueueCnt);
     // copy the essential message info into a more readable variable
     QueueMessageType = FromDisplay.QueueMessageType[FromDisplay.QueueIndexOut];
     memcpy(QueueMessage, FromDisplay.QueueMessage[FromDisplay.QueueIndexOut], sizeof(QueueMessage));
     QueueMessage[sizeof(QueueMessage) - 1] = '\0'; // make sure it's 0 terminated

     if((QueueMessageType>=0) && (QueueMessageType<=MESSAGE_MAX)) 
     { Serial.printf("PROCESS: PUCK-SERIAL %d %s >%s<\n", FromDisplay.QueueMessageSerialNumber[FromDisplay.QueueIndexOut], messagetexts[QueueMessageType], QueueMessage); 
     }

     // and now take care of it
     switch(QueueMessageType)
     { case MESSAGE_OPEN_WEATHER_MAP_API_KEY:
         Serial.println(QueueMessage);
         if(strcmp(GlobeSettings.open_weather_map_api_key, QueueMessage)!=0)
         { strcpy(GlobeSettings.open_weather_map_api_key, QueueMessage);
           Serial.println("Eeprom saved -> GlobeSettings.open_weather_map_api_key");
           EEPROM.put(0x0, GlobeSettings);
           EEPROM.commit();
         }
         break;
      
       // puck sends google api key at booting up, could be different from hard coded key, it set in api key file in puck
       case MESSAGE_GOOGLE_API_KEY:
         Serial.println(QueueMessage);
         if(strcmp(GlobeSettings.google_api_key, QueueMessage)!=0)
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
         RefreshRatesCountDownTimer = 0; // get new rates immediately
         break;  

       // display want exchange rate, for a country code supplied
       case MESSAGE_EX_CHANGE_RATE:
         if(RefreshRatesCountDownTimer==0)
         { // rates not received yet
           break;
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
         // format QueueMessage is "NL-http://stationurl"
         if(!bGlobeStable)break; // ignore all (old) requests while globe is moving!
         extern char CountryCodeSelectorSD[3];
        
         if(QueueMessage[2]=='-')
         { strcpy(TargetUrl, QueueMessage+3); // new format with countrycode-url
           strncpy(CountryCodeSelectorSD, QueueMessage, 2);
           CountryCodeSelectorSD[3]=0;
         } 
         else strcpy(TargetUrl, QueueMessage); // old format only url

         if(!bPowerStatus)
         { Serial.printf("MESSAGE_START_THIS_STATION: IGNORED (Power Down)\n");
           break;
         }

         if((strcmp(PreviousUrl, TargetUrl) == 0) && stream.isRunning())
         { // already connected and playing this station
           Serial.printf("ALREADY CONNECTED TO STATION: %s\n", TargetUrl);  
           AddToQueueForDisplay(TargetUrl, MESSAGE_STATION_CONNECTED);
           break;
         }
             
         strcpy(PreviousUrl, TargetUrl);

         // DataFromDisplay.D_QueueStationIndex range = 0-49 for found stations and 50-53 for presets, -1 if idle
         // Serial.printf("DataFromDisplay.D_QueueStationIndex = %d\n", DataFromDisplay.D_QueueStationIndex);
         
         
         if(DataFromDisplay.D_QueueStationIndex>=MAX_STATIONS) // it's a preset
         { PresetRequestFromHA = DataFromDisplay.D_QueueStationIndex-MAX_STATIONS+1;
           // will be picked up by mqtt loop to inform HA
         }  
         else PresetRequestFromHA = 0;       
        

         // quick & dirty, but maybe too dirty, as not all (but most do) https urls are http approacheable (Http create error)
         // but we also catch the 400 and do a retry on https in that case 
         // so, always try http first, remove the 's' from https
         // if(TargetUrl[4]=='s')strcpy(&TargetUrl[4], &TargetUrl[5]);  
        
         Tuning = true;
         if(StartNewStation()==1) // succes
         { DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
           //Serial.printf("SUCCES: stream.connecttohost HTTP returncode %d\n", stream.connectResult()); 
           Serial.printf("SUCCES: stream.connecttohost\n"); 
           
           AddToQueueForDisplay(QueueMessage, MESSAGE_STATION_CONNECTED); 
           snprintf(message,  sizeof(message), "%d,\"%s\"", ConnectedInMillis, QueueMessage); 
           AddToQueueForDisplay(message, MESSAGE_STATION_CONNECTED_IN_MS); 
         }
         else
         {  DataFromGlobe.D_QueueStationIndex = -1;
            //Serial.printf("FAILED: stream.connecttohost HTTP code %d\n", stream.connectResult()); 
            Serial.printf("FAILED: stream.connecttohost ERROR code %s\n", VS1053_CallBackLastError); 

            if(strcmp(VS1053_CallBackLastError, "Http create error")==0) // DNS fail??l??
            { snprintf(message, sizeof(message), "%s -> %s", VS1053_CallBackLastError, QueueMessage); 
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
              else
              { if(TargetUrl[4]=='s')strcpy(&TargetUrl[4], &TargetUrl[5]);  
              }

              if(StartNewStation()==1) // succes
              { DataFromGlobe.D_QueueStationIndex = DataFromDisplay.D_QueueStationIndex;
                Serial.printf("SUCCES: stream.connecttohost\n"); 
                AddToQueueForDisplay(QueueMessage, MESSAGE_STATION_CONNECTED); 
                snprintf(message, sizeof(message), "%d,\"%s\"", ConnectedInMillis, QueueMessage); 
                AddToQueueForDisplay(message, MESSAGE_STATION_CONNECTED_IN_MS); 
              }
              else
              { Serial.printf("FAILED: stream.connecttohost ERROR code %s\n", VS1053_CallBackLastError); 
                snprintf(message, sizeof(message), "%s -> %s", VS1053_CallBackLastError, QueueMessage); 
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
        GlobePowerDown(0);
        break;
        
      case MESSAGE_SILENT_POWER_DOWN:
      GlobePowerDown(1);
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
        Serial.printf("GlobeSettings.ee_volume = %d\n", GlobeSettings.ee_volume);
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
        Speakers(SPEAKERS_ON);
        SetVolumeMapped(DataFromDisplay.volumevalue); 
        stream.connectToFile(SD, QueueMessage); // play it 
        break;
      
      //case MESSAGE_RELOAD_SD_WITH_COUNTRY:
        //extern char CountryCodeSelectorSD[3];
        //strcpy(CountryCodeSelectorSD, QueueMessage); // will trigger StartPlayFromSD(void)
      //  strcpy(PreviousUrl, "");
      //  break;

      case MESSAGE_START_FILE_BY_INDEX:
        uint16_t idx;
        sscanf(QueueMessage, "%d", &idx);
        PlayFromPlaylistByIndex(idx);
        break;

      case MESSAGE_UPDATE_GLOBE:
        // in puck setup menu, long press serialnumber to trigger
        if(UpdateState==0)UpdateState = 1; // triggers the update procedure, if procedure does not run yet
        break;

      case MESSAGE_TEST_URL:
        // QueueMessage is an url
        // checks if a url connects, nothing more
        // want to use this to check the entire database for bad urls
        // Heap was always healthy, values between ~90K - ~150K 
        // Serial.print("Free Heap: ");
        // Serial.println(ESP.getFreeHeap());
        if(TestUrl(QueueMessage))AddToQueueForDisplay("1", MESSAGE_TEST_URL);
        else
        { snprintf(message, sizeof(message), "%s -> %s", VS1053_CallBackLastError, QueueMessage); 
          AddToQueueForDisplay(message, MESSAGE_TEST_URL);
        } 
        break;  

      default:
        Serial.printf("Unsupported message type %d from display: >%s<\n", QueueMessageType, QueueMessage);  
        sprintf(message, "Message %d not supported %s max is %d", QueueMessageType, QueueMessage, MESSAGE_MAX);
        AddToQueueForDisplay(message, MESSAGE_MAX); 
        break;
    }

    FromDisplay.QueueIndexOut++;
    FromDisplay.QueueIndexOut %= QUEUESIZE;
    //FromDisplay.QueueCnt--;
    __atomic_fetch_sub(&FromDisplay.QueueCnt, 1, __ATOMIC_SEQ_CST);

  }

  nowmillis = millis();
  lapmillis = nowmillis - prevmillis;
  if(lapmillis>1)Serial.printf("message handler took = %dmS\n", lapmillis);
  prevmillis = nowmillis;



  if(bEncoderNewPosition && UpdateState==0)
  { bEncoderNewPosition = false;
    DataFromGlobe.D_QueueStationIndex = -1; // not meaningfull anymore, current list will be replaced
    AddToQueueForDisplay("?", MESSAGE_FINDNEWSTATION);
  }
  
  if(bEncoderKillStation && UpdateState==0)
  { bEncoderKillStation = false;
    stream.stopSong();
    Speakers(SPEAKERS_DELAYED_OFF);
    DataFromGlobe.D_QueueStationIndex = -1;
    strcpy(ActiveUrl, "");
    AddToQueueForDisplay("", MESSAGE_STATION_NAME); // remove 'station name'
    AddToQueueForDisplay("EXPLORING", MESSAGE_STATUS_LINE); // remove 'station name'
    AddToQueueForDisplay("", MESSAGE_SONG_TITLE); // remove 'song title'    
    AddToQueueForDisplay("", MESSAGE_EXPLORING); // remove 'song title'    
  }
  
  //if((LoopTicker100mS%10)==0)
  //{
  //  Serial.printf("Volume = %d\n", stream.getVuMeter());
  //}


  // volume and tone levels checking
  //if(bVolumeToneControlsActive == true)
  { if(PrevTick != LoopTicker100mS)
    { PrevTick = LoopTicker100mS;
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
   vTaskDelay(1); // makes me feel good but is nonsense
}





bool StartNewStation(void)
{ bool return_result = false;
  char message[QUEUEMESSAGELENGTH];


  bSomethingPlays = false;
  
  bMusicMode = false;
  if(bMusicModePrev != bMusicMode)
  { bMusicModePrev = bMusicMode;
    AddToQueueForDisplay("0", MESSAGE_MUSIC_MODE);
  }  

  strcpy(PrevStreamType, ""); // assures that new received stream info gets send to puck
  AddToQueueForDisplay("", MESSAGE_STATUS_LINE);
  

  PixelUpdate(0, 0xFF00FF, 0x000000, 10000); // solid purple

  Serial.printf("StartNewStation with %s\n", TargetUrl);
  SetVolumeMapped(0);

  if(stream.isRunning())
  { Serial.printf("First stop this one: %s\n", PrevUrl);
    stream.stopSong();
  }

  strcpy(StreamType, "");
  strcpy(StreamCodec, "");
  strcpy(StreamBitrate, "");
  strcpy(StreamFilllevel, "");
  AddToQueueForDisplay(StreamType, MESSAGE_STATUS_LINE);
  
  Speakers(SPEAKERS_DELAYED_OFF);
  DataFromGlobe.D_QueueStationIndex = -1;
  strcpy(ActiveUrl, "");
  strcpy(ActiveStationTitle, "");
  strcpy(ActiveSongTitle, "");
  AddToQueueForDisplay("", MESSAGE_SONG_TITLE); 

  
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
//  AI station from Andon labs, dit is de eerste, backlink broadcast
//  strcpy(TargetUrl, "https://streaming.live365.com/a13541");
//  strcpy(TargetUrl, "https://lyd.nrk.no/nrk_radio_p1_ostfold_mp3_m"); // test INVALID URL??


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
  { StationConnectedAtMillis = millis();
    currentMillis = millis();
    ConnectedInMillis = currentMillis - lapMillis;
    Serial.printf("Succesfully connected: %s -> time elapsed = %ld\n", TargetUrl, ConnectedInMillis);
    return_result = 1;
    strncpy(ActiveUrl, TargetUrl, sizeof(ActiveUrl));
    ActiveUrl[QUEUEMESSAGELENGTH-1]=0;
    strcpy(PrevUrl, ActiveUrl);
    PixelUpdate(0, 0x000000, 0x000000, 5000); // all off
  }
  else
  { Serial.printf("Could not connect: %s -> time elapsed = %ld\n", TargetUrl, (currentMillis = millis()) - lapMillis);
  }  
 
  return return_result;
}

void SetVolumeMapped(uint16_t volume)
{ static uint16_t prevvol;
  static uint8_t New_vs1053vol;

  // first some led animation
  if(bSetupCompleted)
  { if(volume>prevvol)
    { //PixelUpdate(4, 0x6060FF, 0x000000, 1000); // right turn purple
      PixelUpdate(4, 0x5050E0, 0x000000, 1000); // right turn purple
    }  
    if(volume<prevvol)
    { PixelUpdate(3, 0x5050EF, 0x000000, 1000); // left turn purple
    }
  }  
  prevvol = volume;

  // VS1053 volume range is -128dB to 0dB which is way too large for practical human use
  // a more practical useable volume range is 60-100, input 100 gives max volume -0dB and 60 being very quiet at -51.2db
  // such ranges suites better with daily usages, so we remap the 0-100 to this range
  if(volume)New_vs1053vol = map(volume, 0, 100, 60, 100); 
  else New_vs1053vol = 0;

  if(volume==0)Speakers(SPEAKERS_DELAYED_OFF);
  else // we have a volume
  { if(GlobeSettings.ee_internal_speakers == 1)
    { Speakers(SPEAKERS_ON); 
    }
    else Speakers(SPEAKERS_DELAYED_OFF); // no sound from speakers wanted
  } 

  Serial.printf("1333 Volume RadioGlobe-Globe-V5.ino->  %d adjusted to %d for VS1053\n", volume, New_vs1053vol);
  stream.setVolume(New_vs1053vol);
    
}



void audio_showstation(const char* info) 
{ char filtered[256];
  static char currentinfo[QUEUEMESSAGELENGTH];
  char *p;
  int cnt;

  if(!bPowerStatus && UpdateState == 0)return;

  // info parameter can be extremely long
  if(strncmp(currentinfo, info, (sizeof(currentinfo)-1)) ==0)return; // once, not twice the same
  strncpy(currentinfo, info, sizeof(currentinfo)); 
  currentinfo[QUEUEMESSAGELENGTH-1]=0;

  strcpy(MetaDataRadioStation, currentinfo); // keep a local copy to mess around with

  Serial.printf("Station: %s\n", MetaDataRadioStation);
  // filter crap station naming, stick with our own name from the datebase
  if((p=strchr(MetaDataRadioStation, '-')) !=0) *p=0; // split idotic long names that combine station & content 
  if(strcasecmp(MetaDataRadioStation, "no name")==0)return; // ignore meaningless names
  if(strcasecmp(MetaDataRadioStation, "my station name")==0)return; // ignore meaningless names
  if(strcasecmp(MetaDataRadioStation, "this is my server name")==0)return; // ignore meaningless names
  if(strcasecmp(MetaDataRadioStation, "untitled")==0)return; // ignore meaningless names
  if(strcasecmp(MetaDataRadioStation, "unnamed Server")==0)return; // ignore meaningless names
  if(strcasecmp(MetaDataRadioStation, "offline")==0)return; // ignore meaningless names
  if(strcasecmp(MetaDataRadioStation, "stream")==0)return; // ignore meaningless names
  if(strncmp(MetaDataRadioStation, "Streaming by", 12) == 0) memmove(MetaDataRadioStation, MetaDataRadioStation + 12, strlen(MetaDataRadioStation + 12) + 1); // remove nonsense
  if(strncmp(MetaDataRadioStation, "Radio Station Name:", 19) == 0) memmove(MetaDataRadioStation, MetaDataRadioStation + 19, strlen(MetaDataRadioStation + 19) + 1); // remove nonsense

  ReplaceHtmlEntities(MetaDataRadioStation);
  UTF8ToExtAscii(MetaDataRadioStation);
  clean_spaces(MetaDataRadioStation);

  if(strlen(MetaDataRadioStation)<2)return; // ignore meaningless names
  

  cnt = filter_string_v3(MetaDataRadioStation, filtered);
  
  Serial.printf("STATION <%s> filtered:%d <%s>\n", info, cnt, filtered );
  
  if(cnt==0) // readable text, not Thai, Arabic etc.
  { strcpy(ActiveStationTitle, MetaDataRadioStation);
    AddToQueueForDisplay(ActiveStationTitle, MESSAGE_STATION_NAME);
  }
  else
  { // keep the name as set by the database
  }

  
}

void audio_showstreamtitle(const char* info) 
{ char filtered[256];
  static char currentinfo[QUEUEMESSAGELENGTH];
  int cnt;
  char *p;

  if(!bPowerStatus && UpdateState == 0)return;
  
  // info parameter can be extremely long
  if(strncmp(currentinfo, info, (sizeof(currentinfo)-1)) ==0)return; // once, not twice the same
  strncpy(currentinfo, info, sizeof(currentinfo)); 
  currentinfo[QUEUEMESSAGELENGTH-1]=0;

  strcpy(MetaDataSongTitle, currentinfo); // keep a local copy to mess around with

  //strcpy(MetaDataSongTitle, "VĚRA MARTINOVÁ - JEZDEC Z NEZNÁMA"); // test
  //strcpy(MetaDataSongTitle, "218.ÃÑ¡àÃÒäÁèà¡èÒàÅÂ - äÁèÁÕàËµØ¼Å - ã¤ÃÊÑ¡¤¹"); // test
  //strcpy(MetaDataSongTitle, "358. L'Heure de la délivrance du 06-08-21 L'offrande sacrificielle. PES. mp3"); //é = 65-cc-81 (65-e combined with cc-81 accent)
  //strcpy(MetaDataSongTitle, "358. L'Heure de la délivrance du 06-08-21 L'offrande sacrificielle. PES. mp3"); // é = c3-ag -> e9
  
  // Serial.printf("Stream title: %s\n", MetaDataSongTitle);
  // filter known crap messages
  if(strcasecmp(MetaDataSongTitle, "Now Playing info goes here")==0)return;
  if(strcasecmp(MetaDataSongTitle, "adbreak")==0)return;  // ignore meaningless names
  if(strcasecmp(MetaDataSongTitle, "Unknown")==0)return;  // ignore meaningless names
  if(strcasecmp(MetaDataSongTitle, "adinsert")==0)return; // ignore meaningless names
  if((p=strstr(MetaDataSongTitle, ".mp3"))!=0) *p=0;      // ignore that too

  ReplaceHtmlEntities(MetaDataSongTitle);
  UTF8ToExtAscii(MetaDataSongTitle); 
  clean_spaces(MetaDataSongTitle);

  cnt = filter_string_v3(MetaDataSongTitle, filtered);
  Serial.printf("SONGTITLE <%s> %d <%s>\n", MetaDataSongTitle, cnt, filtered );
  // replace text if not printable text, like Thai, Arabic etc.
  if(cnt>0)strcpy(MetaDataSongTitle, "Mystery Content");
  if(strcmp(ActiveSongTitle, MetaDataSongTitle)!=0) // new one, send it to puck
  { strcpy(ActiveSongTitle, MetaDataSongTitle);
    AddToQueueForDisplay(ActiveSongTitle, MESSAGE_SONG_TITLE);
  }  
}



// called from VS1053 driver
void audio_eof_stream(const char* info) 
{ static char message[QUEUEMESSAGELENGTH];
  uint32_t StationConnectedForMillis;

  bSomethingPlays = false;
  

  StationConnectedForMillis = millis() - StationConnectedAtMillis;

  Speakers(SPEAKERS_DELAYED_OFF);
  
  DataFromGlobe.D_QueueStationIndex = -1;

  // only next station or file if power is on
  if(bPowerStatus == true)
  { if(!bMusicMode)
    { // VS1053_CallBackLastError is string
      //uxTaskGetStackHighWaterMark(NULL); // alleen dit crashed
      //Serial.printf("Free stack high water mark: %d bytes\n", uxTaskGetStackHighWaterMark(NULL)); // met dit crashed ook trouwens veel stack nog, 14K (van de 16K)
      snprintf(message, sizeof(message), "EOF -> (%s) url=%s stopped after %dmS", VS1053_CallBackLastError, ActiveUrl, StationConnectedForMillis); // crashed af en toe ook
      Serial.printf("snprintf made this string:%s\n", message); // crashed af en toe ook
      strcpy(VS1053_CallBackLastError, "No recent error");
      AddToQueueForDisplay(message, MESSAGE_CONNECTTOHOST_FAILURE); // let display store this number for log report // crashed ook - weer proberen vanaf 26AUG 11:16 goeie next 7x 12:53
      //AddToQueueForDisplay("Ringbuffer Empty", MESSAGE_CONNECTTOHOST_FAILURE); // ook heap_caps_malloc_extmem_enable(1024); verwijderd in setup - rustig nu 26AUG 11:15
      AddToQueueForDisplay("Globe wants next station", MESSAGE_WANT_NEXT_STATION);
      strcpy(ActiveUrl, "");
    }
    else
    { StartPlayFromSD(); // plays next
    }
  }    
}



#include <stdio.h>
#include <string.h>
#include <ctype.h>

void clean_spaces(char *str) 
{   int i = 0;
    int j = 0;
    int in_space = 0;

    while (str[i] != '\0') 
    { // check for newlines and white space
      if (str[i] == '\n' || str[i] == '\r' || str[i] == ' ') 
      { if (!in_space) 
        { str[j++] = ' '; // replace whitespace
          in_space = 1;   
        }
      } 
      else 
      { str[j++] = str[i];  
        in_space = 0;     
      }
      i++;
    }
    str[j] = '\0'; 
}

void ReplaceHtmlEntities(char *name) {
  char *p = name;
  
  // Find the next occurrence of an ampersand
  while ((p = strchr(p, '&')) != 0) 
  { // first Check for NUMERIC entities (starts with &#)
    if (strncmp(p, "&#", 2) == 0) 
    { int val = 0;
      int charsRead = 0;
      
      // Handle hexadecimal values (e.g., &#x20;)
      if (*(p + 2) == 'x' || *(p + 2) == 'X') 
      { sscanf(p + 3, "%x;%n", &val, &charsRead);
        if (charsRead > 0) charsRead += 3; // Account for the "&#x" prefix
      } 
      // Handle decimal values (e.g., &#32;)
      else  
      { sscanf(p + 2, "%d;%n", &val, &charsRead);
        if (charsRead > 0) charsRead += 2; // Account for the "&#" prefix
      }
      
      // If parsing succeeded and the value fits inside a standard ASCII char
      if (charsRead > 0 && val > 0 && val <= 255) 
      { *p = (char)val; // Overwrite '&' with the converted byte
        // Shift the rest of the string to the left (including the null terminator)
        memmove(p + 1, p + charsRead, strlen(p + charsRead) + 1);
        continue; // Keep processing from the same position
      }
    }
    
    // Check for TEXT-based named entities (e.g., &amp;, &lt;)
    bool found = false;
    static const struct { const char* enc; const char dec; } entities[] = {
      {"&lt;", '<'}, {"&gt;", '>'}, {"&amp;", '&'}, {"&quot;", '"'}, {"&apos;", '\''}
    };
    
    // FIXED: Rewritten with != to prevent the markdown HTML rendering bug
    unsigned int totalEntities = sizeof(entities) / sizeof(entities[0]);
    for (unsigned int j = 0; j != totalEntities; j++) {
      int len = strlen(entities[j].enc);
      if (strncmp(p, entities[j].enc, len) == 0) {
        *p = entities[j].dec; // Overwrite '&' with the decoded character
        // Shift the rest of the string to the left (including the null terminator)
        memmove(p + 1, p + len, strlen(p + len) + 1);
        found = true;
        break;
      }
    }
    
    // If no known entity was matched (like a lone "Johnny & Friends"), skip this ampersand
    if (!found) 
    { p++; 
    }
  }
}


void UTF8ToExtAscii(char *target) {
    char *p = target;   // Read pointer
    char *p2 = target;  // Write pointer

    while (*p) {
        // 4-byte UTF-8 sequence (e.g., Emojis) -> Completely skip all 4 bytes
        if ((*p & 0xF8) == 0xF0) {
            p += 4;
        }
        // 3-byte UTF-8 sequence (e.g., Special symbols, Asian characters) -> Skip
        else if ((*p & 0xF0) == 0xE0) {
            // OPTIONAL: Catch the Euro sign (€ = E2 82 AC) if your display supports it
            if ((unsigned char)*p == 0xE2 && (unsigned char)*(p+1) == 0x82 && (unsigned char)*(p+2) == 0xAC) {
                *p2++ = 0x80; // Adjust 0x80 to match your specific display's Euro symbol code
            }
            p += 3;
        }
        // 2-byte UTF-8 sequence (e.g., Western European accents like é, ö, ß, °)
        else if ((*p & 0xE0) == 0xC0) {
            // Extract the actual numerical Unicode value using bitwise operations
            uint16_t unicode = ((*p & 0x1F) << 6) | (*(p + 1) & 0x3F);
            
            // If it fits into standard Extended ASCII, write it down
            if (unicode <= 0xFF) {
                *p2++ = (char)unicode; 
            } // If it's outside Latin-1 (e.g., Polish/Czech), it gets safely ignored
            
            p += 2;
        }
        // Standard 1-byte ASCII (0 to 127) -> Copy 1-to-1
        else {
            *p2++ = *p++;
        }
    }
    *p2 = '\0'; // Properly close the shortened string with a null terminator
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
void PlaySoundBite(uint8_t *soundbite, size_t length, uint16_t volumeoveride)
{  uint16_t volume_to_use;
  
  bSomethingPlays = true;
  
  //if((volumeoveride>0) && (volumeoveride>GlobeSettings.ee_volume))volume_to_use = volumeoveride; // huh ??? what was I thinking
  if((volumeoveride>0) && (volumeoveride>GlobeSettings.ee_volume))volume_to_use = volumeoveride;
  else volume_to_use = GlobeSettings.ee_volume;

  if(volume_to_use<10)volume_to_use=10; 
  if(volume_to_use>70)volume_to_use=70; 

  // soundbites will always play, regardless preferred interal speakers on/off setting
  Speakers(SPEAKERS_ON); 
  SetVolumeMapped(volume_to_use); 
  Serial.printf("PlaySoundBite playing with volume -> %d\n", volume_to_use);
  
  stream.playChunk(soundbite, length); // plays really entire soundbite
  SetVolumeMapped(GlobeSettings.ee_volume); // will enable amplifiers (if speaker switch == on) if radio stream running, else amplifiers off
  // did this soundbite played during radio playing (like battery low warning)
  if(stream.isRunning())return; // leave speakers on, radio stream continues
  // all done, nothing was streaming at this moment, speakers off 
  bSomethingPlays = false;
  
  Speakers(SPEAKERS_DELAYED_OFF);
}

void GlobePowerDown(uint16_t silentmode)
{ uint16_t restore_volume;
  bPowerStatus = false;
  EEPROM.get(0x0, GlobeSettings); // need stored volume settings
  Serial.printf("POWERDOWN - volume -> %d \n", GlobeSettings.ee_volume);
  SetVolumeMapped(GlobeSettings.ee_volume); 
  stream.stopSong();
  franks_vs1053->switchToMp3Mode(); // prevent last snippet of station playing when power-down tune start playing

  strcpy(ActiveStationTitle, "");
  strcpy(ActiveSongTitle, "");
  AddToQueueForDisplay(ActiveStationTitle, MESSAGE_STATION_NAME);
  
  strcpy(PrevStreamType, ""); // assures that new received stream info gets send to puck
  AddToQueueForDisplay("", MESSAGE_STATUS_LINE);
  
  AddToQueueForDisplay(ActiveSongTitle, MESSAGE_SONG_TITLE);
  loopMQTT();
  DataFromGlobe.D_QueueStationIndex = -1;
  
  if(silentmode==0)PlaySoundBite((uint8_t *)mp3_shutdown, sizeof(mp3_shutdown), 0); 
  Serial.printf("POWERDOWN completed\n");

  SetVolumeMapped(0);
  digitalWrite(BT_POWER_PIN, LOW); // turn off BT module

  // tell puck that a globe update is availableor not
  UpdateAvailable = CheckForNewGlobeUpdate();
  if(UpdateAvailable)AddToQueueForDisplay("1", MESSAGE_GLOBE_UPDATE_AVAILABLE);
  else AddToQueueForDisplay("0", MESSAGE_GLOBE_UPDATE_AVAILABLE);
}

void GlobePowerUp(void)
{ char message[QUEUEMESSAGELENGTH]; 
  bPowerStatus = true;

  EEPROM.get(0x0, GlobeSettings); // need stored volume settings
  // tell display what our stored values are for volume and tone control
  sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
  AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
  AddToQueueForDisplay(build_timestamp, MESSAGE_GLOBE_BUILD_DATE_TIME);
  AddToQueueForDisplay(WiFi.localIP().toString().c_str(), MESSAGE_GLOBE_IP);
  sprintf(message, "%s.local", WiFi.getHostname());
  AddToQueueForDisplay(message, MESSAGE_GLOBE_HOSTNAME);
  AddToQueueForDisplay(GlobeSettings.ssid, MESSAGE_SSID_FOR_GLOBE);
  AddToQueueForDisplay(GlobeSettings.password, MESSAGE_PASSWORD_FOR_GLOBE);
  
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
// fun department
// only once use case, triggered from the lat/lon encoders changing, play sound as if rotating the tuning knob on an old radio
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
     if(playwhile) // it was a start
     { bytes_to_play = length;
       snippets_to_play = bytes_to_play / SNIPLENGHT;
       snippet = 0; // start at begin of soundbite
       Serial.printf("snippets_to_play = %ld\n", snippets_to_play);
     }
     else // stop
     { snippets_to_play = 0;
       Speakers(SPEAKERS_DELAYED_OFF);
     }
   }

   if(snippet <= snippets_to_play)
   { if(GlobeSettings.ee_internal_speakers && (digitalRead(MUTE_AMPLIFIERS)==1))Speakers(SPEAKERS_ON);
     // radio tuning sound at volume level as set on puck
     SetVolumeMapped(DataFromDisplay.volumevalue);
     if(snippet < snippets_to_play) // full size snippets
     { Serial.printf("snippet = %ld\n", snippet);
       //snippetMs = millis();
       stream.playChunk(&soundbite[(snippet * SNIPLENGHT)], (size_t)SNIPLENGHT, 0); // additional parameter 0 for early return
       //Serial.println(millis() - snippetMs);
       Speakers(SPEAKERS_DELAYED_OFF);
       snippet++;
     }
     else // last partial snippet
     { Serial.printf("last partial snippet = %ld\n", snippet);
       if(bytes_to_play % SNIPLENGHT)
       { if(GlobeSettings.ee_internal_speakers)Speakers(SPEAKERS_ON);
         stream.playChunk(&soundbite[(snippet * SNIPLENGHT)], (size_t)(bytes_to_play % SNIPLENGHT), 0); // additional parameter 0 for early return
         Speakers(SPEAKERS_DELAYED_OFF);
         snippet++;
       }
       oldplaywhile = 0;
     }
   }
   if(oldplaywhile == 0)return 0;
   else return (snippets_to_play - snippet);
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
  strcpy(GlobeSettings.google_api_key, google_api_key);
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
    { SpeakerOffAfter25mS = 0;
      digitalWrite(MUTE_AMPLIFIERS, 1); // turn off immediately
      Serial.println("Amplifiers are OFF");
    }
    else if(mode == SPEAKERS_DELAYED_OFF)
    { SpeakerOffAfter25mS = 200; // turn of after 200 x 25mS -> 5000mS delay
    }
    else if(mode == SPEAKERS_ON)
    { SpeakerOffAfter25mS = 0;
      digitalWrite(MUTE_AMPLIFIERS, 0); // turn on immediately
      Serial.println("Amplifiers are ON");
    }
  }  
}


  
// Called when codec is detected
void codecCallback(const char *codec)
{ Serial.printf("codec: %s\n", codec);
  // during url test, ignore this
  Serial.printf("1861 codecCallback codec = %s\n", codec);
  if(bPowerStatus  && UpdateState == 0)
  { bSomethingPlays = true;
    SetVolumeMapped(DataFromDisplay.volumevalue); // will also enable amplifiers
    strcpy(StreamCodec, codec);
    sprintf(StreamType, "Streaming %s - %s %s", StreamCodec, StreamBitrate, StreamFilllevel);
    if(strcmp(PrevStreamType, StreamType)!=0)
    { strcpy(PrevStreamType, StreamType);
      AddToQueueForDisplay(StreamType, MESSAGE_STATUS_LINE);
    }
  }  
}


// Called when bitrate is detected (cbr) and changes (vbr)
void bitrateCallback(uint32_t bitrate)
{   // during url test, ignore this
  if(bPowerStatus  && UpdateState == 0) 
  { //Serial.printf("MAIN 1870 bitrate: %lu kbps\n", bitrate);
    sprintf(StreamBitrate, "%lu kbps", bitrate);
    sprintf(StreamType, "Streaming %s - %s %s", StreamCodec, StreamBitrate, StreamFilllevel);
    if(strcmp(PrevStreamType, StreamType)!=0)
    { strcpy(PrevStreamType, StreamType);
      bStreamTypeNeedsUpdated = true; // update to puck done in main loop, once per second
      //AddToQueueForDisplay(StreamType, MESSAGE_STATUS_LINE);
    }
  }
}  

// called from VS1053 driver
void audio_fail(void) 
{ Serial.printf("Decoder failed to sync\n");
}  

// called from VS1053 driver

void VS1053error(const char *error) 
{ strcpy(VS1053_CallBackLastError, error);
  Serial.printf("Error from VS1053 -> %s\n", VS1053_CallBackLastError);
  AddToQueueForDisplay(VS1053_CallBackLastError, MESSAGE_STATUS_LINE);
}

// Called when bitrate is detected (cbr) and changes (vbr)
void filllevelCallback(uint32_t filllevel)
{   // during url test, ignore this
  if(bPowerStatus  && UpdateState == 0) 
  { //Serial.printf("MAIN 1879 filllevel: %lu kbps\n", filllevel);
    sprintf(StreamFilllevel, "%lu%% buffered", filllevel);
    sprintf(StreamType, "Streaming %s - %s %s", StreamCodec, StreamBitrate, StreamFilllevel);
    if(strcmp(PrevStreamType, StreamType)!=0)
    { strcpy(PrevStreamType, StreamType);
      bStreamTypeNeedsUpdated = true; // update to puck done in main loop, once per second
      //AddToQueueForDisplay(StreamType, MESSAGE_STATUS_LINE);
    }
  }
}  


#include <stdio.h>
#include <string.h>

int filter_string_v3(const char *source, char *destination) {
    int i = 0;
    int j = 0;
    int skipped_count = 0;
    int high_ascii_count = 0;

    // count Extended ASCII characters
    // Westersn selodm have more than 5, Thai mojibake has tens or more
    while (source[i] != '\0') {
        unsigned char c = (unsigned char)source[i];
        
        // Skip UTF-8 patterns
        if ((c & 0xE0) == 0xC0) { i += 2; continue; }
        else if ((c & 0xF0) == 0xE0) { i += 3; continue; }
        else if ((c & 0xF8) == 0xF0) { i += 4; continue; }

        if (c > 127) {
            high_ascii_count++;
        }
        i++;
    }

    // filter and copy
    i = 0;
    while (source[i] != '\0') {
        unsigned char c = (unsigned char)source[i];

        // 1. skip validd UTF-8 multi-byte (zoals Cyrillisch)
        if ((c & 0xE0) == 0xC0) { i += 2; skipped_count++; continue; }
        else if ((c & 0xF0) == 0xE0) { i += 3; skipped_count++; continue; }
        else if ((c & 0xF8) == 0xF0) { i += 4; skipped_count++; continue; }

        if (c > 127) {
            // if too many extended ascii, probably thai mojibake
            if (high_ascii_count > 5) {
                i++;
                skipped_count++;
                continue;
            }
        }

        // 3. Veilige ASCII of legitiem westerse Extended ASCII kopiëren
        destination[j++] = source[i++];
    }
    destination[j] = '\0';

    return skipped_count;
}


bool TestUrl(char *url)
{ bool return_result = false;
  char message[QUEUEMESSAGELENGTH];

  PixelUpdate(0, 0x000001, 0x000000, 10000); // solid blue

  //Serial.printf("Test Url with %s\n", url);
  //SetVolumeMapped(0);
  if(stream.isRunning())
  { //Serial.printf("First stop this one: %s\n", ActiveUrl);
    stream.stopSong();
  }
  
  Speakers(SPEAKERS_DELAYED_OFF);
  
  lapMillis = millis(); 
  //Serial.printf("Now connect: %s\n", url);
  stream.connectToHost(url);  

  if(stream.isRunning()) 
  { currentMillis = millis();
    ConnectedInMillis = currentMillis - lapMillis;
    //Serial.printf("Succesfully connected: %s -> time elapsed = %ld\n", url, ConnectedInMillis);
    return_result = true;
    PixelUpdate(0, 0x000100, 0x000000, 2000); // two seconds green
  }
  else
  { Serial.printf("Could not connect: %s -> time elapsed = %ld\n", url, (currentMillis = millis()) - lapMillis);
    PixelUpdate(0, 0x010000, 0x000000, 2000); // two seconds red
  }  
 
  return return_result;
}


// EOF