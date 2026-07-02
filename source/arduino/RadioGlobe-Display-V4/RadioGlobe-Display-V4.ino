
// Using LVGL with Arduino requires some extra steps:
// Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
// #include "lv_conf.h" (root lib folder)
// #include "lvgl.h" (loaded elsewere)
// In lv_conf.h (root lib folder) -> #define LV_MEM_SIZE (60U * 1024U)  /*[bytes]*/  // FB was 48U - remember to change to 60U
// lvgl by kisvegaborn 8.3.10 , not a more recent version, SquareLine Studio only does 8.3.11
// Boards manager, esp32 by Expressif Systems, V3.3.3 with ESP32S3 Dev Module 
// 10APR2026 Boards Manager (trying 3.3.7 now, Waveshare ESP32-S3-Touch-LCD-2.1)
// 10APR2025 Library Manager - trying now - lvgl by kisvegaborn 8.3.11 , not a more recent version, SquareLine Studio only does 8.1.11
// 10APR2025 In lv_conf.h (root lib folder)-> #define LV_MEM_SIZE (60U * 1024U)  /*[bytes]*/  // FB was 48U - display crashes with 48, maybe increase even more if we keep adding objects???
// 14APR2026 Boards Manager (trying 3.3.8 now, Waveshare ESP32-S3-Touch-LCD-2.1) -> graphics (flag and other things) go bad
// 14APR2026 Boards Manager (back to 3.3.7, Waveshare ESP32-S3-Touch-LCD-2.1) (seems to give glitches in display brightness)
// 14APR2026 Boards Manager (back to 3.3.6, Waveshare ESP32-S3-Touch-LCD-2.1) (still gives glitches in display brightness)
// 14APR2026 Boards Manager (back to 3.3.6, ESP32S3 Dev Module 16MB 3MB app PSRAM OPI) (testing, also still gives glitches in display brightness)
// 14APR2026 Boards Manager (back to 3.3.3, ESP32S3 Dev Module 16MB 3MB app PSRAM OPI) (testing..., also still gives glitches in display brightness)
// 14APR2026 Boards Manager (back to 3.3.7, Waveshare ESP32-S3-Touch-LCD-2.1) (seems to give gfixedsplay luminance)brightness
// 14APR2026 Boards Manager (back to 3.3.7, ESP32S3 Dev Module 16MB 4MB app (custom partition) PSRAM OPI)) 

// 28 JUN 26 Changed to custom partition table 4MB APP0 / 4MB APP1 / 8MB FFAT
// had to increase as 3MB was too small for the github download feature, went from 98% to 103% -> compiler now reports as 19% from 16MB but we need to stay below 25% (4MB)
// stored in project folder as partitions.csv
// choose custom partition in Arduino IDE
// # Name,   Type, SubType, Offset,  Size, Flags
// nvs,      data, nvs,     0x9000,  0x5000,
// otadata,  data, ota,     0xe000,  0x2000,
// app0,     app,  ota_0,   0x10000, 0x400000,
// app1,     app,  ota_1,   0x410000,0x400000,
// ffat,     data, fat,     0x810000,0x7E0000,
// coredump, data, coredump,0xFF0000,0x10000,


// update and features added
// 29 MAY 26 -> Only find and collect stations from one country
// 17 MAR 26 -> Added speaker off icon, during volume value shown, in case speakers are off
// 15 MAR 26 -> EXPERIMENT LVGL_Driver.cpp line 101 -> commented taskdelay again, maybe not neccesairy, eeprom screen glitch fix attempt
// 14 MAR 26 -> FEATURE shadow edges on long/short press buttons, green for just a selection, yellow for storing a change, red for possibly dangerous change
// 14 MAR 26 -> BUG sprintf timezone to get text string for clock, added (size_t) modifiers to uint8_t variables, as this caused rare random crash/reset of puck
// 14 MAR 26 -> BUG removed needless timezone update on clock hour hand change which caused unexplainable random crash/reset of puck every 5-50 minutes or so
// 13 MAR 26 -> IMPROVEMENT find station again after power up, in case globe was moved to new location
// 13 MAR 26 -> FIX revised eeprom stuff, now saves and loads from file on SD preventing display corruption with the real eeprom save function
// 12 MAR 26 -> FEATURE clickable infostation map
// 11 MAR 26 -> IMPROVEMENT improved click/longpressed algoritm
// 11 MAR 26 -> BUG improved roller setting in calibration menu
// 14 FEB 26 -> FEATURE now has wifi channel updated by globe wifi connection
// LVGL_Driver.cpp line 101 -> eeprom screen glitch fix attempt

#define MBEDTLS_SSL_MAX_CONTENT_LEN 4096

#define BUILD_LABEL "Puck Build"
// make this -> Puck Build Jun 23 2026 - 21:08:50
#define BUILD_TIMESTAMP  BUILD_LABEL " " __DATE__ " - " __TIME__
#define BUILD_TIMESTAMP_ONLY __DATE__ " - " __TIME__
const char build_label[] =  BUILD_LABEL;
const char build_timestamp[] =  BUILD_TIMESTAMP;
const char build_timestamp_only[] =  BUILD_TIMESTAMP_ONLY;
extern uint8_t UpdateFirmware(uint8_t state); // start with 1 for a full date/time check and update
extern char Wifi_SSID[];
extern char Wifi_PASSWORD[];

#include "..\secrets.h"

// as defined in ..\secrets.h
// char SSID [32] = "YOUR-SSID";
// char PASSWORD[32] = "YOUR-WIFI-PASSWORD";
// char google_api_key[] = "YOUR-API-KEY"; // free, get your own at google developer platform, used for timezone retrieval, replace first 4 letters AIza in !!!!
// to avoid a verbatim copy of api key in compiled binary 

// https://console.cloud.google.com/google/maps-apis/credentials?project=subtle-backup-498313-s5
// https://console.cloud.google.com/apis/dashboard?project=subtle-backup-498313-s5
// char open_weather_map_api_key[] = "YOUR-API-KEY"; // free, get  your own at https://openweathermap.org/api

#include "Wireless.h"
#include "Gyro_QMI8658.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "BAT_Driver.h"

//#include "lv_conf.h"
#include "ui.h"
#include "ui_additional_widgets.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClient.h>
//#include <NetworkClientSecure.h>


#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "franks-esp-now.h"
#include "database.h"
#include "favorites.h"

#include <EEPROM.h>
#define EEPROM_SIZE 4096

#include <ctype.h>

struct eepromData 
{ uint8_t globemac[6];
  uint32_t magicnumber;
  uint32_t serialnumber; // number from 51,52,53 and so on
  station_name_url_gps stationnug[MAX_FAVORITES+MAX_HOMES];
  uint16_t bluetoothswitchable; // true if hardware supports disabling power to bluetooth module in globe
  uint16_t btmodule_power_on;
  uint16_t wifichannel;
  uint16_t bluetoothinstalled; // true if module is installed
  char google_api_key[64];
  char open_weather_map_api_key[64];
  uint16_t expand_search; // number of degrees to look around for a station
  uint16_t sdcard_present;
  uint16_t auto_update_state = 0;
  
  char spare_data[];
};
eepromData DisplaySettings; // values to work with
eepromData OldDisplaySettings; // values to compare for changes

extern stations_arraybin Stations;

uint8_t PuckMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
uint8_t NewMac[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}; 
bool bMusicMode = false;
bool bMusicModePrev = false;
bool ScrollNeedsReload = false;

// FTP stuff
#define STORAGE_TYPE STORAGE_SD_MMC
#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SD_MMC
#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32 		NETWORK_ESP32
#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
#include <SimpleFTPServer.h>
extern FtpServer ftp;
extern bool bFtpActive;
extern uint16_t FtpBootState;
extern uint16_t UpdateState;

void readMacAddress()
{ esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, PuckMac);
  Serial.println(BUILD_TIMESTAMP);
  if (ret == ESP_OK) 
  { Serial.printf("readMacAddress() -> ESP32 Puck MAC Address read: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   PuckMac[0], PuckMac[1], PuckMac[2],
                   PuckMac[3], PuckMac[4], PuckMac[5]);
  } 
  else 
  { Serial.println("readMacAddress() -> Failed to read Puck MAC address");
  }
}

bool bTimer100ms = false;
bool bUpAndRunning = false;
bool ForceGlobeStationGPSupdate = false;

uint32_t GlobalTicker100mS = 0;
uint32_t GlobalTicker1S = 0;
uint32_t PrevGlobalTicker1S = 0;
char SecretCode[6] = "12345";  
char PrevSecretCode[6] = "-----";  
uint16_t CalibrationModeLatLong = 0;
#define CALMODE_NS   0x01
#define CALMODE_EW   0x02
#define CALMODE_NSEW 0x03

typedef struct ns_ew_intersection
{ int16_t ns; // in tenths of degree
  int16_t ew; // in tenths of degree
};

// this bin holds the NS/EW coordinates that belong to intersection points of longitudes and latitudes around the globe.
// initially filled with data for a perfect globe.
// as we calibrate, the arry gets more sophisticated and holds the coordinates as the encoders see them on the actual not-so-accurately-printed globe.
// we have two rows for n/s at equator as the upper half and lower haf of the globe may not be aligned very well, as was certainly the case with my globe.
typedef struct calibrations_arraybin
{  ns_ew_intersection calibrations[(180/15)+2][360/15]; // 14 positions for north to south, 0-15-30-45-60-75-90
};

EXT_RAM_ATTR calibrations_arraybin ns_ew_calibrations;
EXT_RAM_ATTR calibrations_arraybin def_cal;
uint16_t CalToIndexNS;
uint16_t CalToIndexEW;
bool bPowerStatus = true;
#define AUTOPOWERDOWNAFTER 8 * 3600; // auto power after 12HR 
uint32_t AutoSleepTimer = AUTOPOWERDOWNAFTER;

#define DEFAULT_BACKLIGHT 75
uint8_t BacklightValue = DEFAULT_BACKLIGHT;
uint8_t PrevBacklightValue = 255;
#define DEFAULT_HOLD_BACKLIGHT 60 // seconds
uint8_t HoldBacklight = 0;
#define DEFAULT_SHOW_VOLUME_TIMER 3 // seconds
uint16_t ShowVolumeTimer = 0;


lv_mem_monitor_t mon_p; // for lvgl memory info

#include "ui_additional_widgets.h";

char ClockFlagCountryCode[] = "??"; // actual flag shown on clock
char OldClockFlagCountryCode[] = "??"; // actual flag on clock before change
char GlobePositionCountryCode[] = "??"; // flag from country at chosen location
bool ClockHomeTime = true; 
bool ClockBackLight = true; // keep the clock lit up

void Driver_Loop(void *parameter)
{
  while(1)
  { GlobalTicker100mS++;
    //QMI8658_Loop();  // get accelero meter, not needed
    RTC_Loop();
    // BAT_Get_Volts();
    bTimer100ms = true;

    // auto dim backlight and gyro test every second
    if((GlobalTicker100mS % 10)==0)
    { GlobalTicker1S++;
      getGyroscope();
      uint16_t motion = abs((int)Gyro.x) + abs((int)Gyro.y) + abs((int)Gyro.z); 
      //Serial.printf("x%f - y%f - z%f\n", Gyro.x, Gyro.y, Gyro.z);
      //Serial.printf("motion is %d\n", motion);
      //Serial.printf("motion %d freeze %d backlightvalue %d\n", motion, freeze, backlightvalue);
      if(motion>30)
      { BacklightValue = DEFAULT_BACKLIGHT;
        AutoSleepTimer = AUTOPOWERDOWNAFTER;
        ClockBackLight = true;
        if(bPowerStatus)
        { HoldBacklight = DEFAULT_HOLD_BACKLIGHT; // give our beloved user 60 seconds of full brightness before it starts fading again
        }
      }    

            
      if(HoldBacklight>0)
      { HoldBacklight--;
      }
      else if(BacklightValue>10)
      { BacklightValue--;
      }
      
      if(ShowVolumeTimer)
      { ShowVolumeTimer--; // once timed out, flag will be shown again
        if(!ShowVolumeTimer && DataFromDisplay.volumevalue)
        { lv_obj_add_state(uic_VolumeValue, LV_STATE_DISABLED); // away with volume level
          lv_obj_add_flag(uic_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN); // away with speaker icon
          lv_obj_add_flag(uic_mainscreen_speakeron, LV_OBJ_FLAG_HIDDEN); // away with speaker icon
          lv_obj_clear_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN); // show country flag again
          lv_obj_clear_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // show city name again
          lv_obj_clear_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // show country name again
          ShowBatteryLevel(false); // hide battery level status
          ShowWeatherData(true); // show weather status again
        }  
      }
    }

    if(PrevBacklightValue != BacklightValue)
    { PrevBacklightValue = BacklightValue;
      //Serial.printf("Set BacklightValue to %d\n", BacklightValue);
      Set_Backlight(BacklightValue);    
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void Driver_Init()
{ //Flash_test();
  BAT_Init();
  I2C_Init();
  TCA9554PWR_Init(0x00); // io extender  
  Set_EXIO(EXIO_PIN8,Low); // buzzer off
  PCF85063_Init(); // rtc
  QMI8658_Init(); // gyro
}


// memory monitor on preset screen
void monitor_update(void) 
{ char buf[64];
  static uint32_t previous_free_size = 0;
  lv_mem_monitor(&mon_p);
  // sprintf(buf, "Mem: %d/%dKB (%.1f%%)",
  // mon_p.free_size / 1024,
  // mon_p.total_size / 1024,
  // (float)mon_p.free_size * 100.0f / mon_p.total_size);
  // Serial.printf("%s\n", buf);
  if(previous_free_size != mon_p.free_size)
  { previous_free_size = mon_p.free_size;
    sprintf(buf, "%.1f%%", (float)mon_p.free_size * 100.0f / mon_p.total_size);
    lv_label_set_text(uic_MemoryUsage, buf);
  }
}
    
uint64_t sleepTime = 10000000;  // Sleep duration in microseconds (10 seconds)

extern void setupusb();

void setup()
{ char content[64];

  // replace !!!! characters in fetched api key (secrets.h)
  // silly trick to disguise api key for github scan 
  google_api_key[0]='A';
  google_api_key[1]='I';
  google_api_key[2]='z';
  google_api_key[3]='a';
  
  Serial.begin(115200);

  heap_caps_malloc_extmem_enable(1024); 

  // sleep test
  // Enable wake-up by timer
  // esp_err_t result = esp_sleep_enable_timer_wakeup(sleepTime);

  Driver_Init();
  
  delay(1000); // delay added, apperently after new upload, 5V has dropped, give it some time to recover, tested ok with 750, use 1000 for extra margin (FB)

  LCD_Init();   // If you later reinitialize the LCD, you must initialize the SD card again !!!!!!!!!!
  SD_Init();    // It must be initialized after the LCD, and if the LCD is reinitialized later, the SD also needs to be reinitialized

  //setupusb(); // doesnt work - wanted to use puck as usb storage for PC 

  EEPROM.begin(EEPROM_SIZE);
  LoadDisplaySettingsFromEeprom();
  InitializeDisplaySettings(); // at first run, initialize with some reasonable values

  setup_esp_now(); 

  Lvgl_Init();

  ui_init();

  ui_additional_text_init(); // manual creation of simple items that didn't fit in the max 150 objects of Squareline Studio

  lv_obj_add_flag(ui_Power_Off_Icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN); // hide country flag until new country code is received
  lv_obj_add_flag(uic_Database_Flag, LV_OBJ_FLAG_HIDDEN); // hide country flag until new country code is received
  lv_obj_add_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // hide city name until new country code is received
  lv_obj_add_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // hide country name until new country code is received
  lv_label_set_text(ui_Home_City, "");
  lv_label_set_text(ui_Home_Country, "");
  ShowWeatherData(false); // hide weather info
  if(DisplaySettings.sdcard_present==1)lv_obj_clear_flag(uic_MusicLibraryButton, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(uic_MusicLibraryButton, LV_OBJ_FLAG_HIDDEN);

  Lvgl_Loop();

  xTaskCreatePinnedToCore(
    Driver_Loop,     
    "Other Driver task",   
    4096,                
    NULL,                 
    3,                    
    NULL,                
    0                    
  );

  // show used wifi channel in preset screen
  sprintf(content, "CH-%d", DisplaySettings.wifichannel);
  lv_label_set_text(uic_GlobeText, content);

  //lv_mem_init(); ??? crashes everything
  monitor_update(); // memory percentage

  set_optional_items(); // BT speaker switch

  readMacAddress(); // from puck 
  sprintf(content, "Puck Mac %02X:%02X:%02X:%02X:%02X:%02X", PuckMac[0], PuckMac[1], PuckMac[2], PuckMac[3], PuckMac[4], PuckMac[5]);
  Serial.println(content);
  lv_label_set_text(ui_PuckMac, content);
  sprintf(content, "Globe Mac %02X:%02X:%02X:%02X:%02X:%02X", DisplaySettings.globemac[0], DisplaySettings.globemac[1], DisplaySettings.globemac[2], DisplaySettings.globemac[3], DisplaySettings.globemac[4], DisplaySettings.globemac[5]);
  Serial.println(content);
  lv_label_set_text(ui_GlobeMac, content);
  lv_label_set_text(ui_PuckBuild, build_timestamp);

  ShowWeatherData(false);

  LoadFavoritesFromEEprom(); // to have some defaults when SD card not present
  // load preset stations from favorites.txt file
  LoadFavorites();
  LoadApiKeys();


  // set all calibrations to perfect values
  CalibrationInit();
  // load stored values from setup file, calibrations.txt, from SD card
  LoadCalibrations(); 


  if(SD_MMC.begin("/sdcard", true, false))
  { ReadStationsBitmapFile(SD_MMC, "/stationsmap.bmp"); // read the bmp with the dots as stations
    SD_MMC.end();
  }  

 
  bUpAndRunning = true;

  sprintf(content, "%s", DisplaySettings.google_api_key);
  AddToQueueForGlobe(content, MESSAGE_GOOGLE_API_KEY);
  sprintf(content, "%s", DisplaySettings.open_weather_map_api_key);
  AddToQueueForGlobe(content, MESSAGE_OPEN_WEATHER_MAP_API_KEY);

  AddToQueueForGlobe("", MESSAGE_POWERUP);
  AddToQueueForGlobe("DISPLAY WANTS MQTT STATUS", MESSAGE_MQTT_STATUS); 
  Stations.playing = -1;

  // SD card driver for LVGL
//  lv_port_fs_init(); // ??? do I really need this -> apperently not -> disabled this to see if it fixes the odd display jump/jitter 

  // use home location as stored in favorites.txt as 5th record to get timezone and place
  DataFromDisplay.D_StationGpsNS = Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].gps_ns;
  DataFromDisplay.D_StationGpsEW = Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].gps_ew;
  strcpy(GlobePositionCountryCode, Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode);
  sprintf(content, "%f-%f", DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW);
  AddToQueueForGlobe(content, MESSAGE_GET_GEOLOCATION_BY_GPS);
  AddToQueueForGlobe(content, MESSAGE_GET_TIMEZONE_BY_GPS);

  //if(DisplaySettings.sdcard_present)
  //{ ftp.begin("guest", "guest");
  //  Serial.println("FTP gestart!");
  //  Serial.println(WiFi.localIP());
  //}  
}

bool bDatabaseScreenUpdate = false;
bool bCheckDatabase = false;
bool bNoTimeZone = true;
bool bInfoScreen = false;

char *partofday[4] = {"Sweet Night", "Good Morning", "Jolly Afternoon", "Nice Evening"};
char *weekdays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *monthnames[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
char timezonename[64] = "";


void loop()
{ char content[256];
  int32_t value_int32t;
  static int newvolumevalue;
  static int newbassvalue;
  static int newtreblevalue;
  static int newbatteryvoltage;
  static int oldbatteryvoltage = 0;
  
  static uint8_t oldsecond;
  static uint8_t oldhour;
  static uint8_t oldminute;
  static lv_obj_t * oldscreen;
  static time_t now;
  unsigned int stream_connecttohost_result; // received from callback in globe 
  char logfile[32];

  static uint16_t old_mqtt_truss_volume;
  static uint16_t mqtt_truss_volume;
  static String receivedMessage = "";
  static char incomingChar;

  static uint16_t QueueMessageType;
  static char QueueMessage[QUEUEMESSAGELENGTH];
  static uint16_t old_puck_rssi = 0;
  static uint16_t puck_rssi;
  static uint16_t new_channel = 0;

  char town[64]="";         
  char *p1;
  char countrycode[3]="";

  static int16_t OldHourAngle;
  int16_t HourAngle;
  static int16_t OldMinuteAngle;
  int16_t MinuteAngle;
  static int16_t OldSecondAngle;
  int16_t SecondAngle;
  unsigned long ClockMillisLapse;
  static unsigned long SecondStartMs;



  Lvgl_Loop();
  if(bFtpActive)
  { if(DisplaySettings.sdcard_present)ftp.handleFTP();
  }

  loop_esp_now(); // send volume & other stuff to globe
  
  if(FtpBootState)
  { FtpBootState = HandleFtpBootState(FtpBootState);
  }

  if(UpdateState)
  { UpdateState = UpdateFirmware(UpdateState);
    if(UpdateState==0)WiFi.disconnect();
    Serial.println(WiFi.localIP());
  } 

  if(bTimer100ms)
  //if(1)
  {  bTimer100ms = false;
  }

  {
    //monitor_update(); 
    while(Serial.available()) 
    { incomingChar = Serial.read();
      if(incomingChar == '\n')
      { Serial.print("You typed this: ");
        Serial.println(receivedMessage);
        if(receivedMessage[0]=='G')
        { // store globe mac address example globe 54 -> GM=98:A3:16:EC:27:C4
          // scanf with %hx because NewMac[] is array of uint8_t
          sscanf(receivedMessage.c_str(), "GM=%hx:%hx:%hx:%hx:%hx:%hx", &NewMac[0], &NewMac[1], &NewMac[2], &NewMac[3], &NewMac[4], &NewMac[5]);
          memcpy(DisplaySettings.globemac, NewMac, sizeof(DisplaySettings.globemac));
          Serial.printf("New Mac Adress For Globe = %s\n", receivedMessage.c_str());
          SaveDisplaySettingsToEeprom();
          Serial.println("New Globe Mac saved to EEprom..");
          setup_esp_add_peer(); 
          sprintf(content, "Globe Mac %02X:%02X:%02X:%02X:%02X:%02X", (uint16_t)DisplaySettings.globemac[0], (uint16_t)DisplaySettings.globemac[1], (uint16_t)DisplaySettings.globemac[2], (uint16_t)DisplaySettings.globemac[3], (uint16_t)DisplaySettings.globemac[4], (uint16_t)DisplaySettings.globemac[5]);
          lv_label_set_text(ui_GlobeMac, content);
        }
        else if(receivedMessage[0]=='M')
        { // print pucks mac address
          readMacAddress();
        }
        else if(receivedMessage[0]=='S')
        { if(receivedMessage[1]=='B')
          { if(DisplaySettings.bluetoothswitchable == 1)
            { DisplaySettings.bluetoothswitchable = 0;
            }
            else
            { DisplaySettings.bluetoothswitchable = 1;
            }
            Serial.printf("Updating DisplaySettings.bluetoothswitchable -> %d\n", DisplaySettings.bluetoothswitchable);
            SaveDisplaySettingsToEeprom();
          }
          set_optional_items();
        } 
        else if(receivedMessage[0]=='F')
        { uint16_t digit = receivedMessage[1]-48;
          if(digit>3)digit=3;
          AddStationToQueueForGlobe(digit+MAX_STATIONS); // presets come just after the regular list of stations
        }
        else if(receivedMessage[0]=='Q')
        { AddToQueueForGlobe("", MESSAGE_GLOBE_PLAY_SD);
        }
        else if(receivedMessage[0]=='X')
        { DataFromDisplay.D_QueueStationIndex = 49;
          AddToQueueForGlobe("https://stream.zeno.fm/dahlxvtaz1guv", MESSAGE_START_THIS_STATION); https://stream.zeno.fm/dahlxvtaz1guv
        }
        else if(receivedMessage[0]=='R')
        { DisplaySettings.magicnumber = 0; // reset
          InitializeDisplaySettings();
          ESP.restart();
        }
        else if(receivedMessage[0]=='U')
        { //AddToQueueForGlobe("1", MESSAGE_UPDATE_GLOBE);
          UpdateState = 1;
        }
        receivedMessage = "";
      }
      else
      { receivedMessage += incomingChar;
      }
    }
  
    lv_obj_t * screen = lv_scr_act(); //get active screen

    if(oldscreen != screen) // screen changed
    { oldscreen = screen;
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
      if(bPowerStatus==true)
      { if(screen == ui_CalibrationScreen) // force refresh raw coordinate
        { PrevDataFromGlobe.ns = -1;  
        }
        if(screen == ui_CalibrationScreenAdvanced) // force refresh
        { PrevDataFromGlobe.ns = -1;   
          DataForLCD.ns_cal = -1;
        }
        BacklightValue = DEFAULT_BACKLIGHT;
      }  
      //Serial.println("Screen changed!");
      if(screen == ui_Home)
      { CalibrationModeLatLong = (CALMODE_NS | CALMODE_EW); // also effects the display of calibrated coordinates, 
        bInfoScreen = false;
        bDatabaseScreenUpdate = false;
        if(OldDisplaySettings.expand_search != DisplaySettings.expand_search)SaveDisplaySettingsToEeprom();
        // keep the code for 1 re-enter setup
        // if(strcmp(SecretCode, "GLOBE")!=0)strcpy(SecretCode,"12345");
      }
      monitor_update();
    }

    // process one or more queued messages from globe
    while((FromGlobe.QueueIndexIn != FromGlobe.QueueIndexOut) && !bFtpActive) // we have to catch up with new messages
    { Serial.printf("FromGlobe.QueueIndexIn = %d FromGlobe.QueueIndexOut = %d\n", FromGlobe.QueueIndexIn, FromGlobe.QueueIndexOut);
      Serial.printf("Messages from globe pending %d\n", FromGlobe.QueueCnt);
      DataFromDisplay.G_QueueSerialNumber = DataFromGlobe.G_QueueSerialNumber; 
       // copy the essential message info into a more readable variable
      QueueMessageType = FromGlobe.QueueMessageType[FromGlobe.QueueIndexOut];
      memcpy(QueueMessage, FromGlobe.QueueMessage[FromGlobe.QueueIndexOut], sizeof(QueueMessage));
      
      if((QueueMessageType>=0) && (QueueMessageType<MESSAGE_MAX)) 
      { Serial.printf("GLOBE SAYS: %s >%s<\n", messagetexts[QueueMessageType], QueueMessage);  
      }
       switch(QueueMessageType)
      { case MESSAGE_SONG_TITLE: // 1
          if(strlen(QueueMessage)>0)RemoveUTF8Unprintables(QueueMessage);
          lv_label_set_text(ui_Station_Title, QueueMessage);
          break;
        case MESSAGE_ARTIST: // 2
          break;
        case MESSAGE_ALBUM:
          break;
        case MESSAGE_GENRE:
          break;
        case MESSAGE_STATION_NAME:
          Serial.printf("case MESSAGE_STATION_NAME: %s\n", QueueMessage);
          if(strlen(QueueMessage))
          { if(strlen(QueueMessage)>31)break;
            if(strlen(QueueMessage)<3)break;
          }  
          lv_label_set_text(ui_Station_Name, QueueMessage);
          // rename in scroller if it makes sense
          if(!bMusicMode)
          { if(Stations.requested<MAX_STATIONS)
            { strcpy(Stations.StationNUG[Stations.requested].name, QueueMessage);
              ReloadScroll();
            }
          }  
          // else it is a station from the preset (1000 or more)  
          break;
        case MESSAGE_DESCRIPTION:
          break;
        case MESSAGE_GOOGLE_API_KEY:
          break;
        case MESSAGE_TIMEZONE_ID:
          AllUpperCase(QueueMessage);
          lv_label_set_text(ui_Time_Zone, QueueMessage);
          datetime.month = DataFromGlobe.timeinfo.tm_mon;
          datetime.day = DataFromGlobe.timeinfo.tm_mday;
          datetime.dotw = DataFromGlobe.timeinfo.tm_wday;
          datetime.hour = DataFromGlobe.timeinfo.tm_hour; // + (timeinfo.tm_isdst>0)?1:0;
          datetime.minute = DataFromGlobe.timeinfo.tm_min;
          datetime.second = DataFromGlobe.timeinfo.tm_sec;
          datetime.year = DataFromGlobe.timeinfo.tm_year;
          PCF85063_Set_All(datetime);
          // also set system wide clock as well, to get the right times and dates on files created 
          now = mktime(&DataFromGlobe.timeinfo); // get it in epoch seconds 
          // Serial.printf("NEW GLOBE EPOCH UTC SECONDS = %ld\n", (long)now);
          settimeofday((const timeval *) &now, NULL);
          strcpy(timezonename, QueueMessage);
          sprintf(content, "%s\n%s %d-%s-%d\n%s",  timezonename, weekdays[datetime.dotw], (size_t)datetime.day, monthnames[datetime.month],  (size_t)datetime.year%100, partofday[datetime.hour/6]);
          lv_label_set_text(ui_Time_Zone_Clock, content); // on clock screen
          break;
           
        case MESSAGE_TIMEZONE_NAME:
          break;

        case MESSAGE_EX_CHANGE_RATE:
          // 1 Euro = 1.13 US Dollar - or some sort of message
          // Pay With Euro Here - when same valuta as at home is used
          lv_label_set_text(ui_Database_Progress, QueueMessage); 
          break;
         case MESSAGE_WIFI_STATUS:
          break;
        
        case MESSAGE_CALIBRATE_ZERO:  
          break;
         case MESSAGE_FINDNEWSTATION:
          // station or file already killed by globe
          bMusicMode = false;
          SetLed(0,0); SetLed(1,0); SetLed(2,0); SetLed(3,0); // turn off leds just in case
           if((screen != ui_CalibrationScreen) && (screen != ui_CalibrationScreenAdvanced))
          { // if in tone controle screen or preset screen, jump back to home screen
            if((screen != ui_DatabaseScreen) || (bInfoScreen==true))
            { if(screen==ui_Power) // clock screen
              { Serial.printf("Clock screen, find new station for flag and time update \n");
                ClockHomeTime = false;
                FindNewStation();
                ReloadScroll();
              }
              else if(bPowerStatus == true)
              { // hide all info stuff until new station is requested after search
                //lv_obj_add_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN); // hide flag 
                //lv_obj_add_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // hide city name 
                //lv_obj_add_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // hide country name 
                //lv_obj_add_flag(uic_Clock_Country, LV_OBJ_FLAG_HIDDEN); // hide country name 
                ShowWeatherData(false); // hide weather info
                //lv_label_set_text(ui_Station_Name, "Searching..."); // pointless, searching goes to quick to notice
                lv_label_set_text(ui_Station_Title, "");
                lv_scr_load(ui_Home);
                Lvgl_Loop();  
                FindNewStation();
                ReloadScroll();
              }
            }  
          }
          break;

        case MESSAGE_GLOBE_MAC:
          // check if different from what we have
          Serial.printf("New Globe Mac Adress received from Globe = %s\n", QueueMessage);
          sscanf(QueueMessage, "%hx:%hx:%hx:%hx:%hx:%hx", &NewMac[0], &NewMac[1], &NewMac[2], &NewMac[3], &NewMac[4], &NewMac[5]);
          Serial.printf("Globe Mac %02X:%02X:%02X:%02X:%02X:%02X (Current)\n", (uint16_t)DisplaySettings.globemac[0], (uint16_t)DisplaySettings.globemac[1], (uint16_t)DisplaySettings.globemac[2], (uint16_t)DisplaySettings.globemac[3], (uint16_t)DisplaySettings.globemac[4], (uint16_t)DisplaySettings.globemac[5]);
          if(memcmp(DisplaySettings.globemac, NewMac, sizeof(DisplaySettings.globemac)) != 0)
          { memcpy(DisplaySettings.globemac, NewMac, sizeof(DisplaySettings.globemac));
            SaveDisplaySettingsToEeprom();
            Serial.println("New (Different) Globe Mac saved to EEprom..");
            setup_esp_add_peer(); 
            sprintf(content, "Globe Mac %02X:%02X:%02X:%02X:%02X:%02X", (uint16_t)DisplaySettings.globemac[0], (uint16_t)DisplaySettings.globemac[1], (uint16_t)DisplaySettings.globemac[2], (uint16_t)DisplaySettings.globemac[3], (uint16_t)DisplaySettings.globemac[4], (uint16_t)DisplaySettings.globemac[5]);
            lv_label_set_text(ui_GlobeMac, content);
            //ESP.restart();
          }
          break;
      
        case MESSAGE_GLOBE_IP:  
          break;

        case MESSAGE_CONNECTTOHOST_FAILURE:
          DataFromDisplay.D_QueueStationIndex = -1;
          Stations.playing = -1;
          if(DataFromGlobe.D_QueueMessageCount<10) // don't waste time logging when behind schedule
          { SD_MMC.end(); //?? just to be sure
            if(SD_MMC.begin("/sdcard", true, false)) // we have a card 
            { // response is like "error  -> url"
              // reported error message will be used as filename
              char filename[QUEUEMESSAGELENGTH];
              strcpy(filename, QueueMessage);
              char *p;
              if((p=strstr(filename, " -> "))!=0)
              { *p=0;
                //Serial.printf("MESSAGE_CONNECTTOHOST_FAILURE filename part is <%s>\n", filename);
                // reported error message will be used as filename, must sanize first
                const char *illegal_chars = " \\/:*?\"<>|";
                p = filename;
                while (*p)
                { if (strchr(illegal_chars, *p) != NULL) *p = '_'; // Replace with a safe character
                  p++;
                }
                //Serial.printf("MESSAGE_CONNECTTOHOST_FAILURE sanitized filename will be <%s>\n", filename);
                sprintf(logfile, "/ERR-%s.txt", filename);
                //Serial.printf("MESSAGE_CONNECTTOHOST_FAILURE full filename will be <%s>\n", logfile);
                AppendBadStationToFile(SD_MMC, logfile, QueueMessage);
                SD_MMC.end();
              }
              SD_MMC.end();
            }  
          }    
          break; 


        case MESSAGE_DEAD_STATION:
        case MESSAGE_AUDIO_EOF_STREAM:
          if(!bMusicMode)
          { //lv_label_set_text(ui_StationRollerComment, "");
            DataFromDisplay.D_QueueStationIndex = -1;
            Stations.playing = -1;
            //sprintf(content, "%s - Not Responding", lv_label_get_text(ui_Station_Name));
            //lv_label_set_text(ui_Station_Name, content); 
            //Lvgl_Loop(); // update screen
            //delay(300); // so we can actually notice the text change on the screen
            if(!bMusicMode)
            if(DataFromGlobe.D_QueueMessageCount<10) // don't waste time logging when behind schedule
            { SD_MMC.end(); //??
              if(SD_MMC.begin("/sdcard", true, false))
              { if(QueueMessageType == MESSAGE_DEAD_STATION)
                { sprintf(logfile, "/badstations-%d.txt", stream_connecttohost_result);
                  AppendBadStationToFile(SD_MMC, logfile, QueueMessage);
                }
                if(QueueMessageType == MESSAGE_AUDIO_EOF_STREAM)AppendBadStationToFile(SD_MMC, "/audio-eof-stream.txt", QueueMessage);
                SD_MMC.end();
              }  
            }    
          }
          break;

        case MESSAGE_WANT_NEXT_STATION: // request from globe since it couldn't use the last url
          if(Stations.requested<0)Stations.requested=0;
          if(Stations.count<=0) // not likely to happen, but can occur after a boot
          { FindNewStation();
            ReloadScroll();
          }  
          if(Stations.requested >= MAX_STATIONS) // a problematic preset was ordered
          { // now what, what is a next station in this context?
            SetLed(Stations.requested-MAX_STATIONS,0); // just turn off the led
            lv_label_set_text(ui_Station_Name, ""); 
            lv_label_set_text(ui_Station_Title, "");
            lv_label_set_text(ui_Home_City, "");
          }
          else
          { sprintf(content, "%s - Skipped", Stations.StationNUG[Stations.requested].name);
            lv_label_set_text(ui_Station_Name, content); 
            lv_label_set_text(ui_StationRollerComment, content); 
            Lvgl_Loop(); // update screen
            //delay(300); // so we can actually notice the text change on the screen
            if(Stations.connect_attempts<(2 * Stations.count) && Stations.count>0) // loop through all stations twice
            { Stations.requested++;
              Stations.requested %= Stations.count; // funny, this can crash as a division by zero, added protection to the if-condition above
              Serial.printf("Stations.count =%d\n", Stations.count);  
              if(Stations.requested < Stations.count) // until the end of the list, or else we are done
              { Serial.printf("Stations.requested =%d: >%s<\n", Stations.requested, Stations.StationNUG[Stations.requested].name);  
                lv_roller_set_selected(uic_StationRoller, Stations.requested, LV_ANIM_ON);
                sprintf(content, "%d-%d", Stations.requested+1, Stations.count); // top label 1-150 in stations roller
                lv_label_set_text(ui_StationRollerSelected, content);
                lv_label_set_text(ui_StationRollerComment, Stations.StationNUG[Stations.requested].name); 
                AddStationToQueueForGlobe(Stations.requested);
              }
            } 
            else 
            { // no 'next' station
              lv_label_set_text(ui_Station_Name, "No Next Stations Found"); 
              lv_label_set_text(ui_StationRollerComment, content); 
            }  
          }
          break;

        case MESSAGE_STATION_CONNECTED: // 25
          // MESSAGE FROM GLOBE: 25 >https://stream06.dotpoint.nl:8004/stream<
          // update text for station scroller 
          // update leds on preset screen
          bMusicMode = false;
          if(Stations.requested<MAX_STATIONS+MAX_FAVORITES)
          { 
            sprintf(content, "%s - Connected", Stations.StationNUG[Stations.requested].name);
            Stations.playing = Stations.requested;
            lv_label_set_text(ui_StationRollerComment, content); 
             if(Stations.requested<MAX_STATIONS) // not a preset but from the list of available stations
            { if(Stations.connect_attempts>0)Stations.connect_attempts--;
              Serial.printf("(GLOBE SAYS): Station Playing %s\n", Stations.StationNUG[Stations.requested].name);
              SetLed(0,0); SetLed(1,0); SetLed(2,0); SetLed(3,0);
            }
            else
            { Serial.printf("(GLOBE SAYS): Preset Playing %s\n", Stations.StationNUG[Stations.requested].name);
              SetLed(Stations.requested-MAX_STATIONS, UI_THEME_COLOR_GREEN);
            }
          }  
          break;

        case MESSAGE_GLOBE_WANTS_CURRENT_STATION: // 26 after a power up
           if(!bMusicMode)
           { if(Stations.playing>=0 && bPowerStatus)
             { ForceGlobeStationGPSupdate = 1;
               AddStationToQueueForGlobe(Stations.playing);
             }
             else // after power up, and globe moved to other place/timezone -> find a station for that 
             { FindNewStation();
             }
           }
           else
           {
             AddFileToQueueForGlobe(Stations.requested);
           }  
           break;
         case MESSAGE_VOLUME_AND_TONE:
           { int32_t volume;
             int32_t bass;
             int32_t treble;
             int32_t internalspeaker;
             sscanf(QueueMessage, "%ld %ld %ld %ld", &volume, &bass, &treble, &internalspeaker);
             // sync controls to that
             lv_arc_set_value(uic_VolumeArc, volume);
             lv_arc_set_value(uic_BassArc, bass);
             lv_arc_set_value(uic_TrebleArc, treble);
             if(internalspeaker == 1)
             { lv_obj_add_state(uic_InternalSpeaker, LV_STATE_CHECKED);
               lv_obj_add_flag(uic_speakeroff, LV_OBJ_FLAG_HIDDEN); 
               lv_obj_clear_flag(uic_speakeron, LV_OBJ_FLAG_HIDDEN);
               DataFromDisplay.internalspeakeron = 1;
               if(ShowVolumeTimer)
               { lv_obj_add_flag(uic_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN); 
                 //lv_obj_clear_flag(uic_mainscreen_speakeron, LV_OBJ_FLAG_HIDDEN);
               }  
             } 
             else
             { lv_obj_clear_state(uic_InternalSpeaker, LV_STATE_CHECKED); 
               lv_obj_add_flag(uic_speakeron, LV_OBJ_FLAG_HIDDEN); 
               lv_obj_clear_flag(uic_speakeroff, LV_OBJ_FLAG_HIDDEN);
               DataFromDisplay.internalspeakeron = 0;
               if(ShowVolumeTimer)
               { //lv_obj_add_flag(uic_mainscreen_speakeron, LV_OBJ_FLAG_HIDDEN); 
                 lv_obj_clear_flag(uic_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN);
               }  
             }
             sprintf(content, "%ld", volume);
             lv_label_set_text(uic_VolumeValue, content); 
             sprintf(content, "%ld", bass);
             lv_label_set_text(uic_BassValue, content); 
             sprintf(content, "%ld", treble);
             lv_label_set_text(uic_TrebleValue, content); 
             AddToQueueForGlobe("VOLUME AND TONE SET AS REQUESTED", MESSAGE_VOLUME_AND_TONE);
           }
           break;

        // globe forwards HA-MQTT power down request
        case MESSAGE_POWERDOWN:
          if(bPowerStatus == true)handlePowerCycle();
          break;

        case MESSAGE_POWERUP:
          ForceGlobeStationGPSupdate = 1;
          if(bPowerStatus == false)handlePowerCycle();
          break;
        
        case MESSAGE_GET_GEOLOCATION_BY_GPS: // in response to request using database station gps coordinates
        case MESSAGE_GET_GEOLOCATION: // in response to request using calibrated NS/EW coordinates
          // example: "NL"
          // example: "NL,"
          // example: "NL,Amsterdam"
          // example: ",Amsterdam"
          // example: ","
          // example: ""
          // example: "XX,???"  // at sea
          strcpy(town, "");
          strcpy(countrycode, "");
          if((p1=strchr(QueueMessage, ','))!= NULL)strcpy(town, p1+1);
          if(isalpha(QueueMessage[0]))
          { countrycode[0] = QueueMessage[0];
            if(isalpha(QueueMessage[1]))
            { countrycode[1] = QueueMessage[1];
              countrycode[2] = 0;
            }
            else countrycode[0] = 0;
          }
          RemoveUTF8Unprintables(town); // arabic town names are not printable with extended ascii fonts
          Serial.printf("GPS Countrycode = %s and town = %s\n", countrycode, town);

          // maybe this is not needed anymore once database is 100% cleaned up with correct country names
          // but leave it for now to see if we get alarming messages
          if(QueueMessageType == MESSAGE_GET_GEOLOCATION_BY_GPS)
          { // check if country code is different from database, log when mismatch
            { if(Stations.requested<MAX_STATIONS+MAX_FAVORITES) // skip MAX_HOMES here
              { if(strlen(Stations.StationNUG[Stations.requested].countrycode)==2) // not empty
                { if(strcmp(Stations.StationNUG[Stations.requested].countrycode, countrycode)!=NULL) // country different from expected
                  { // must write to file for later examination
                    Serial.printf("Problem with Stations.requested = %d\n", Stations.requested);
                    SD_MMC.end(); //??
                    if(SD_MMC.begin("/sdcard", true, false))
                    { strcpy(logfile, "/database-error.txt");
                      AppendBadStationToFile(SD_MMC, logfile, QueueMessage);
                      sprintf(content, "Error in Database - countrycode %s should be %s", Stations.StationNUG[Stations.requested].countrycode, countrycode);
                      AppendBadStationToFile(SD_MMC, logfile, content);
                      Serial.println(content);
                      sprintf(content, "                  - gps_ns = %f gps_ew = %f", Stations.StationNUG[Stations.requested].gps_ns, Stations.StationNUG[Stations.requested].gps_ew);
                      AppendBadStationToFile(SD_MMC, logfile, content);
                      Serial.println(content);
                      sprintf(content, "                  - url = %s", Stations.StationNUG[Stations.requested].url);
                      AppendBadStationToFile(SD_MMC, logfile, content);
                      Serial.println(content);
                      sprintf(content, "                  - town = %s", Stations.StationNUG[Stations.requested].town);
                      AppendBadStationToFile(SD_MMC, logfile, content);
                      Serial.println(content);
                      SD_MMC.end();             
                    }  
                  }
                }    
              } 
            }
          } // end error logging

          if(strlen(countrycode)==2) // && (strcmp(countrycode, "XX")!=NULL)) // XX is impossible for a radiostation
          { strcpy(Stations.StationNUG[Stations.requested].countrycode, countrycode); // only when a positive result was returned (Google does not report on palestine and some others)
            if(FindCountryNameByCode(Stations.StationNUG[Stations.requested].countryname, Stations.StationNUG[Stations.requested].countrycode))
            { Serial.printf("Called from radioglobe-display.ino line 785\n");
              SetFlag(countrycode);
              lv_event_send(ui_Home_Flag, LV_EVENT_REFRESH, NULL);
              lv_obj_clear_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN); // show flag 
              lv_label_set_text(uic_Home_Country, AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
              lv_obj_clear_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // it was hidden by a new search start
              lv_label_set_text(uic_Clock_Country, AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
              lv_obj_clear_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // it was hidden by a new search start
              sprintf(content, "%s  -  %s", Stations.StationNUG[Stations.requested].town, AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
              lv_label_set_text(uic_StationRollerPlace, content);
              // on setup screen
              lv_label_set_text(uic_YouLiveHere, AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
              
              if(strcmp(town, "???")==NULL)
              { sprintf(content, "%s", AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
                Serial.printf("content1 = <%s>\n", content);
                lv_label_set_text(uic_Home_City, "");
                // on map
                sprintf(content,"Greetings From  %s", AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
                lv_label_set_text(ui_Database_Town_Name, content);
              } 
              else 
              { // on map
                sprintf(content,"Greetings From  %s", AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
                lv_label_set_text(ui_Database_Town_Name, content);
                if(strlen(town)>3)sprintf(content,"You Are In  %s", town);
                else sprintf(content,"You Are In  %s", AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
                lv_label_set_text(ui_Database_Output_File, content);
                if(strlen(town)>3)sprintf(content, "%s  -  %s", town, AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
                else sprintf(content, "%s", AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
                Serial.printf("content2 = <%s>\n", content);
                Serial.printf("content2 = <%s>\n", town);
                lv_label_set_text(uic_Home_City, town);
              }
              lv_label_set_text(uic_StationRollerPlace, content);
              AddToQueueForGlobe(countrycode, MESSAGE_EX_CHANGE_RATE);
              lv_label_set_text(ui_Database_Progress, ""); // erase now, will be refreshed when globe answers 
            }
            else
            { sprintf(content, "Countrycode %s Not In List", QueueMessage);
              lv_label_set_text(uic_Home_Country, content);
              sprintf(content, "%s", Stations.StationNUG[Stations.requested].town);
              lv_label_set_text(uic_StationRollerPlace, content);
            }
          }  
          break;

        case MESSAGE_STATION_CONNECTED_IN_MS:
          // append to connect log
          SD_MMC.end(); //??
          if(SD_MMC.begin("/sdcard", true, false))
          { strcpy(logfile, "/connecting-times.csv");
            AppendBadStationToFile(SD_MMC, logfile, QueueMessage);
            SD_MMC.end();
          }  
          break;

         case MESSAGE_STATION_WEATHER_DATA:
          ShowBatteryLevel(false);
          Lvgl_Loop();
          delay(10);
          SetWeatherData(QueueMessage);
          ShowWeatherData(true);
          break;

        case MESSAGE_GLOBE_BUILD_DATE_TIME:
          lv_label_set_text(ui_GlobeBuild, QueueMessage);
          break;

        case MESSAGE_RADIO_PRESET:
          sscanf(QueueMessage, "%d", &value_int32t);
          playpreset(value_int32t-1);
          break;

        case MESSAGE_MQTT_STATUS:
          if(strcmp(QueueMessage, "ON")==0)lv_obj_clear_flag(uic_ledmqtt, LV_OBJ_FLAG_HIDDEN);
          else lv_obj_add_flag(uic_ledmqtt, LV_OBJ_FLAG_HIDDEN);
          break;

        case MESSAGE_DISPLAY_BT_SWITCHABLE_STATE:
          // 0-X -> no switch
          // 1-0 -> switch present but off
          // 1-1 -> switch present and on
          // X-X-1 -> BT module installed
          // X-X-0 -> no BT module present
          if(QueueMessage[0]=='1')
          { if(DisplaySettings.bluetoothswitchable != 1)
            { DisplaySettings.bluetoothswitchable = 1;
              Serial.printf("Updating DisplaySettings.bluetoothswitchable -> %d\n", DisplaySettings.bluetoothswitchable);
              SaveDisplaySettingsToEeprom();
              Serial.println("..and bluetoothswitchable saved to EEprom..");
            }
          }
          else
          { if(DisplaySettings.bluetoothswitchable != 0)
            { DisplaySettings.bluetoothswitchable = 0;
              Serial.printf("Updating DisplaySettings.bluetoothswitchable -> %d\n", DisplaySettings.bluetoothswitchable);
              SaveDisplaySettingsToEeprom();
              Serial.println("..and bluetoothswitchable to EEprom..");
            }
          }

          if(QueueMessage[2]=='1')
          { if(DisplaySettings.btmodule_power_on != 1)
            { DisplaySettings.btmodule_power_on = 1;
              Serial.printf("Updating DisplaySettings.btmodule_power_on -> %d\n", DisplaySettings.btmodule_power_on);
              SaveDisplaySettingsToEeprom();
              Serial.println("..and saved to EEprom..");
            }
          }
          else
          { if(DisplaySettings.btmodule_power_on != 0)
            { DisplaySettings.btmodule_power_on = 0;
              Serial.printf("Updating DisplaySettings.btmodule_power_on -> %d\n", DisplaySettings.btmodule_power_on);
              SaveDisplaySettingsToEeprom();
              Serial.println("..and saved to EEprom..");
            }
          }

          if(QueueMessage[4]=='1')
          { if(DisplaySettings.bluetoothinstalled != 1)
            { DisplaySettings.bluetoothinstalled = 1;
              Serial.printf("Updating DisplaySettings.bluetoothinstalled -> %d\n", DisplaySettings.bluetoothinstalled);
              SaveDisplaySettingsToEeprom();
              Serial.println("..and saved to EEprom..");
            }
          }
          else
          { if(DisplaySettings.bluetoothinstalled != 0)
            { DisplaySettings.bluetoothinstalled = 0;
              Serial.printf("Updating DisplaySettings.bluetoothinstalled -> %d\n", DisplaySettings.bluetoothinstalled);
              SaveDisplaySettingsToEeprom();
              Serial.println("..and saved to EEprom..");
            }
          }
          set_optional_items();
          break;

        case MESSAGE_DISPLAY_SERIALNUMBER:
          { Serial.printf("Stored serialnumber was %d\n", DisplaySettings.serialnumber);
            if(strlen(QueueMessage)>1)
            { sscanf(QueueMessage, "%d", &value_int32t);
              if((value_int32t>50) && (value_int32t<10000)) // valid serial number
              { if(DisplaySettings.serialnumber != value_int32t) // only take action if actuallly changed
                { DisplaySettings.serialnumber = value_int32t;
                  Serial.printf("New serialnumber is %d\n", DisplaySettings.serialnumber);
                  SaveDisplaySettingsToEeprom();
                  Serial.println("New serialnumber saved to EEprom..");
                }
              }
            }
          }
          break;

        case MESSAGE_SET_PUCK_WIFI_CHANNEL:
          sscanf(QueueMessage, "%d", &new_channel);
          if(DisplaySettings.wifichannel != new_channel)
          { DisplaySettings.wifichannel = new_channel;
            SaveDisplaySettingsToEeprom();
            Serial.println("New Wifi Channel saved to EEprom..");
            // reset puck
            ESP.restart();
          }
          break;

        case MESSAGE_GLOBE_HAS_SD:
          sscanf(QueueMessage, "%d", &DisplaySettings.sdcard_present);
          if(OldDisplaySettings.sdcard_present != DisplaySettings.sdcard_present)SaveDisplaySettingsToEeprom();
          if(DisplaySettings.sdcard_present==1)lv_obj_clear_flag(uic_MusicLibraryButton, LV_OBJ_FLAG_HIDDEN);
          else lv_obj_add_flag(uic_MusicLibraryButton, LV_OBJ_FLAG_HIDDEN);
          break;

        case MESSAGE_MUSIC_MODE:
          if(QueueMessage[0]=='1')bMusicMode = true;
          else bMusicMode = false;

          SetLed(0,0); SetLed(1,0); SetLed(2,0); SetLed(3,0);
           
          if(bMusicMode == true)
          { // empty roller
            Stations.count = 0;
            Stations.requested -1;
            ReloadScroll();
            lv_label_set_text(uic_Home_City, "SD-Card");
          }
          break;

        case MESSAGE_PLAYLIST_SONG_ARTIST:
          // add song and artist to roller
          AddToScroll(QueueMessage);
          Serial.printf("Added\n");
          break;

        case MESSAGE_SET_ROLLER_INDEX:
          // received from globe when (next) song from SD is started
          sscanf(QueueMessage, "%d", &Stations.requested);
          lv_roller_set_selected(uic_StationRoller, Stations.requested, LV_ANIM_ON);
          sprintf(content, "%d-%d", Stations.requested+1, Stations.count); // top label 1-150 in stations roller
          lv_label_set_text(ui_StationRollerSelected, content);
          break;

        case MESSAGE_UPDATE_PUCK:
          // in puck setup menu, long press serialnumber to trigger, globe updates first, then puck update
          if(UpdateState==0)UpdateState = 1; // triggers the update procedure, if procedure does not run yet
          break;

        case MESSAGE_SSID_FOR_GLOBE:
          strcpy(Wifi_SSID, QueueMessage);
          break;

        case MESSAGE_PASSWORD_FOR_GLOBE:
          strcpy(Wifi_PASSWORD, QueueMessage);
          break;

        default:
          Serial.printf("Unsupported message type %d from globe: >%s<\n", QueueMessageType, QueueMessage);  
          break;
      }

      FromGlobe.QueueIndexOut++;
      FromGlobe.QueueIndexOut %= QUEUESIZE;
      FromGlobe.QueueCnt--;
     
    }


    sscanf(lv_label_get_text(ui_VolumeValue), "%d", &newvolumevalue);
    if(DataFromDisplay.volumevalue != newvolumevalue)
    { DataFromDisplay.volumevalue = newvolumevalue; // globe will pick that up
      BacklightValue = DEFAULT_BACKLIGHT;
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
      ShowVolumeTimer = DEFAULT_SHOW_VOLUME_TIMER;
      lv_obj_add_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN); // hide flag
      lv_obj_add_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // hide town
      lv_obj_add_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // hide country name
      ShowWeatherData(false); // hide weather status
      ShowBatteryLevel(true); // show battery level
      if(newvolumevalue) // and show it
      { lv_obj_add_flag(uic_Power_Off_Icon, LV_OBJ_FLAG_HIDDEN); // hide power off icon
        lv_obj_clear_state(ui_VolumeValue, LV_STATE_DISABLED); // show volume value
        if(DataFromDisplay.internalspeakeron == 1)lv_obj_add_flag(uic_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN); 
        else lv_obj_clear_flag(uic_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN);
      }
      else // hide volume value
      { lv_obj_add_state(uic_VolumeValue, LV_STATE_DISABLED); // hide volume value
        lv_obj_add_flag(uic_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN); // hide speaker-off symbol
        lv_obj_clear_flag(uic_Power_Off_Icon, LV_OBJ_FLAG_HIDDEN); // show power off icon
      }
    }

    sscanf(lv_label_get_text(ui_BassValue), "%d", &newbassvalue);
    if(DataFromDisplay.bassvalue != newbassvalue)
    { DataFromDisplay.bassvalue = newbassvalue;
      BacklightValue = DEFAULT_BACKLIGHT;
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
    }

    sscanf(lv_label_get_text(ui_TrebleValue), "%d", &newtreblevalue);
    if(DataFromDisplay.treblevalue != newtreblevalue)
    { DataFromDisplay.treblevalue = newtreblevalue;
      BacklightValue = DEFAULT_BACKLIGHT;
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
    }
     
    

    if((DataForLCD.ns_cal !=  DataFromDisplay.ns_cal) || (DataForLCD.ew_cal !=  DataFromDisplay.ew_cal))
    { //Serial.println("Update coordinates on lcd!");
      if(screen == ui_CalibrationScreenAdvanced)
      {  GetFormattedLocation(content, "D", CalibrationModeLatLong);
         lv_label_set_text(ui_CalibrationAdvGps, content);
         DataForLCD.ns_cal = DataFromDisplay.ns_cal;
         DataForLCD.ew_cal = DataFromDisplay.ew_cal;
      }
      else if(screen == ui_Home) 
      {  GetFormattedLocation(content, "D", CalibrationModeLatLong);
         lv_label_set_text(ui_HomeGps, content);
         DataForLCD.ns_cal = DataFromDisplay.ns_cal;
         DataForLCD.ew_cal = DataFromDisplay.ew_cal;
      }
      else  if(screen == ui_DatabaseScreen) 
      { lv_obj_set_pos(uic_MapCursor, (int)DataFromDisplay.ew_cal/10 - 16, -(int)DataFromDisplay.ns_cal/10); // moved 16 to left, dot is painted left-top corner?
      }
    }

    // new uncalibrated encoder position arrived, remap to calibrated
    if(((PrevDataFromGlobe.ns !=  DataFromGlobe.ns) || (PrevDataFromGlobe.ew !=  DataFromGlobe.ew)) && DataFromGlobe.G_EncoderReliable)
    { //Serial.println("Update coordinates on lcd!");
      BacklightValue = DEFAULT_BACKLIGHT;
      remap_ns_ew(DataFromGlobe.ns, DataFromGlobe.ew); 
      if(screen == ui_CalibrationScreenAdvanced)
      { GetFormattedLocation(content, "G", CalibrationModeLatLong);
        lv_label_set_text(ui_CalFromRaw, content);
      }
      else  if(screen == ui_CalibrationScreen) 
      { GetFormattedLocation(content, "G", CalibrationModeLatLong);
        lv_label_set_text(ui_CalibrationRawCoord, content);
      }
      PrevDataFromGlobe.ns = DataFromGlobe.ns;
      PrevDataFromGlobe.ew = DataFromGlobe.ew;
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
    } 


    if(PrevDataFromGlobe.G_rssi_globe !=  DataFromGlobe.G_rssi_globe)
    { PrevDataFromGlobe.G_rssi_globe =  DataFromGlobe.G_rssi_globe; 
      sprintf(content, "%d dB", DataFromGlobe.G_rssi_globe); 
      lv_label_set_text(uic_GlobeRSSI, content); // update wifi strength on MY FAVORITES screen
    }

    // puck_rssi = WiFi.RSSI();
    // sprintf(content, "Puck %d dB", puck_rssi); 
    // Serial.println(content);
    // if(old_puck_rssi != puck_rssi)
    // { old_puck_rssi = puck_rssi;
    //  sprintf(content, "Puck %d dB", puck_rssi); 
    // Serial.println(content);
    // }

    if(strcmp(PrevSecretCode, SecretCode)!=0)
    //if(screen == uic_PasswordScreen)
    { strcpy(PrevSecretCode, SecretCode);
      if(strcmp(SecretCode, "GLOBE")==0)
      { lv_label_set_recolor(ui_lockstatus, true);
        lv_label_set_text(ui_lockstatus, "#00FF00 UNLOCKED#");
        lv_obj_clear_flag(ui_HomeButton4, LV_OBJ_FLAG_HIDDEN); // show home icon
      }
      else if(strcmp(SecretCode, "BOBOB")==0)
      { if(DisplaySettings.sdcard_present)
        { lv_label_set_recolor(ui_lockstatus, true);
          lv_label_set_text(ui_lockstatus, "#0000FF FTP ACTIVE#");
          lv_obj_add_flag(ui_HomeButton4, LV_OBJ_FLAG_HIDDEN); // hide home button
          FtpBootState = 1; // launch the FTP server 
        }  
      }
      else
      { lv_label_set_recolor(ui_lockstatus, true);
        lv_label_set_text(ui_lockstatus, "#FF0000 LOCKED#");
        // todo show home button
        lv_obj_clear_flag(ui_HomeButton4, LV_OBJ_FLAG_HIDDEN); // show home icon
        if(bFtpActive)FtpBootState = 6;
      }
    }

    // battery indicator consists of 3 slightly different icons on top of one another
    if(PrevGlobalTicker1S != GlobalTicker1S) // check every second
    { PrevGlobalTicker1S = GlobalTicker1S;
      newbatteryvoltage = ((analogReadMilliVolts(BAT_ADC_PIN) * 3) + 50) / 100; // read voltage in tenths of volts times 3 because of voltage divider 
      ShowBatteryLevel(-newbatteryvoltage);
      DataFromDisplay.D_BatteryVoltage = newbatteryvoltage;
      //if((PrevGlobalTicker1S%10)==0)
      //{ Serial.print("AutoSleepTimer ->");
      //  Serial.println(AutoSleepTimer);
     // }
      if(AutoSleepTimer)
      { AutoSleepTimer--;
        if(AutoSleepTimer==0)
        { if(bPowerStatus == true)
          { handlePowerCycle();
            Serial.println("AutoSleepTimer Power Down");
          }
        }
      }
    }
     
    // clock stuff
    // keep analog clock when globe is turned off
    // with smooth moving seconds hand (too shabby really)
    // or in second steps (better)
    // https://www.pixilart.com/draw?ref=home-page is a nice pixel editor for creating the hands of the clock


    if((GlobalTicker100mS % 1)==0)
    { //if(oldsecond != datetime.second)
      //{ oldsecond = datetime.second;
      //  //SecondStartMs = millis(); // uncomment for smooth movement
      //}
       
      if(screen == ui_Power) // we are on clock screen
      { // uncomment for smooth movement
        //ClockMillisLapse = millis() - SecondStartMs;
        //ClockMillisLapse %= 1000;
        //Serial.printf("ClockMillisLapse = %ld\n", ClockMillisLapse);
        //SecondAngle = ((datetime.second * 3600)/60) + (((ClockMillisLapse) * 60)/1000); // smooth movement

        SecondAngle = ((datetime.second * 3600)/60); // full second steps
        HourAngle = ((datetime.hour * 3600)/12) + ((datetime.minute * 3600)/720);
        MinuteAngle = (ui_MinuteHand, ((datetime.minute * 3600)/60)) + datetime.second;

        if(OldHourAngle != HourAngle)
        { OldHourAngle = HourAngle;
          lv_img_set_angle(ui_HourHand, HourAngle);
        }

        if(OldMinuteAngle != MinuteAngle)
        { OldMinuteAngle = MinuteAngle;
          lv_img_set_angle(ui_MinuteHand, MinuteAngle);
        }

        if(OldSecondAngle != SecondAngle)
        { OldSecondAngle = SecondAngle;
          lv_img_set_angle(ui_SecondHand, SecondAngle);
        }
      }
      
      // check flag
      if(ClockHomeTime)
      { strcpy(ClockFlagCountryCode, Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode);
        //Serial.printf("ClockFlagCountryCode from Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode = %s\n", ClockFlagCountryCode);
      }
      else
      { strcpy(ClockFlagCountryCode, GlobePositionCountryCode);
        //Serial.printf("ClockFlagCountryCode from GlobePositionCountryCode = %s\n", ClockFlagCountryCode);
      }
      if(strcmp(OldClockFlagCountryCode, ClockFlagCountryCode)!=NULL)
      { strcpy(OldClockFlagCountryCode, ClockFlagCountryCode);
        // set flag
        Serial.printf("New ClockFlagCountryCode = %s\n", ClockFlagCountryCode);
        //if(screen == ui_Power)
        SetFlag(ClockFlagCountryCode);
        lv_obj_clear_flag(ui_Home_Flag, LV_OBJ_FLAG_HIDDEN);
      }
    }  
     

    // let's do a flashing semicolon between hours an minutes of clock
    if(screen == ui_Home) 
    { static uint32_t PrevTicker;
      if(PrevTicker != GlobalTicker100mS)
      { PrevTicker = GlobalTicker100mS;
        if((GlobalTicker100mS % 5)==0) // every 500mS
        { // Serial.print("Seconds oldsecond: "); Serial.println(oldsecond);
          // sprintf(content, "%02d:%02d:%02d", datetime.hour, datetime.minute, datetime.second);
          sprintf(content, "%02d%c%02d", datetime.hour, ((GlobalTicker100mS % 10)==0)?':':' ', datetime.minute);
          lv_label_set_text(ui_Local_Time, content);
        }  
      }
    }  

    if(bCheckDatabase)BuildDatabaseNow();

    mqtt_truss_volume = DataFromGlobe.G_Volume;
    if(old_mqtt_truss_volume != mqtt_truss_volume)
    { old_mqtt_truss_volume = mqtt_truss_volume;
      //Serial.printf("mqtt_truss_volume = %d\n", mqtt_truss_volume);
      lv_arc_set_value(uic_VolumeArc, mqtt_truss_volume);
      sprintf(content, "%d", mqtt_truss_volume);
      lv_label_set_text(ui_VolumeValue, content);
    }

    // lame attempt to save power when running on battery
    //if(bPowerStatus == false)
    //{ if(ClockBackLight == false)
    //  { Serial.println("Going into light sleep mode");
    //    esp_light_sleep_start();     // Enter light sleep
    //    Serial.println("Returning from light sleep");
    //  } 
    //}

    if(bMusicModePrev != bMusicMode)
    { bMusicModePrev = bMusicMode;
      if(bMusicMode)lv_label_set_text(ui_Text_Radio_Globe, "MUSIC GLOBE");
      else lv_label_set_text(ui_Text_Radio_Globe, "RADIO GLOBE");
    } 
  }
  delay(5); // 18APR26 added this, display brightness glitches fixed this - do not remove!!
}


void GetFormattedLocation(char *dest, char *src, int16_t ModeLatLong)
{ int16_t ns;
  int16_t ew;
  if(*src == 'G')
  { ns = DataFromGlobe.ns;
    ew = DataFromGlobe.ew;
  }
  else if(*src == 'D')
  { ns = DataFromDisplay.ns_cal;
    ew = DataFromDisplay.ew_cal;
  }
  else if(*src == 'C')
  { ns = ns_ew_calibrations.calibrations[CalToIndexNS][CalToIndexEW].ns;
    ew = ns_ew_calibrations.calibrations[CalToIndexNS][CalToIndexEW].ew;
  }

  //Serial.printf("GetFormattedLocation(content, \"%c\", %d) : CalToIndexNS = %d CalToIndexEW = %d\n", *src, ModeLatLong, CalToIndexNS, CalToIndexEW);
 
  if(ModeLatLong == CALMODE_NS)
  {  sprintf(dest, "%c%d.%d", (ns<0)?'S':'N', abs(ns)/10, abs(ns)%10);
  }  
       
  if(ModeLatLong == (CALMODE_NS | CALMODE_EW)) 
  { sprintf(dest, "%c%d.%d - %c%d.%d", (ns<0)?'S':'N', abs(ns)/10, abs(ns)%10, (ew<0)?'W':'E', abs(ew)/10, abs(ew)%10);

  }

  if(ModeLatLong == CALMODE_EW)
  { sprintf(dest, "%c%d.%d", (ew<0)?'W':'E', abs(ew)/10, abs(ew)%10);
  }  
}

void InitializeDisplaySettings(void)
{ if(DisplaySettings.magicnumber == 123456)
  { Serial.println("EEprom data seems OK");
    if(DisplaySettings.expand_search<2)DisplaySettings.expand_search = 2;
    return;
  }

  
  Serial.println("Initialize Puck EEprom with some default values");
  DisplaySettings.magicnumber = 123456;
  DisplaySettings.serialnumber = 50;
  //memset(DisplaySettings.globemac, 0, 6);

  DisplaySettings.globemac[0] = 0xff;
  DisplaySettings.globemac[1] = 0xff;
  DisplaySettings.globemac[2] = 0xff;
  DisplaySettings.globemac[3] = 0xff;
  DisplaySettings.globemac[4] = 0xff;
  DisplaySettings.globemac[5] = 0xff;

  // set some presets
  strcpy(DisplaySettings.stationnug[0].name, "NPO Radio2");
  strcpy(DisplaySettings.stationnug[0].url, "https://icecast.omroep.nl/radio2-bb-mp3");
  DisplaySettings.stationnug[0].gps_ns = 53.174599;
  DisplaySettings.stationnug[0].gps_ew = 5.425200;
  strcpy(DisplaySettings.stationnug[0].town, "Harlingen");
  strcpy(DisplaySettings.stationnug[0].countrycode, "NL");
//  strcpy(DisplaySettings.stationnug[0].countryname, "Netherlands");

  strcpy(DisplaySettings.stationnug[1].name, "radio 98eins");
  strcpy(DisplaySettings.stationnug[1].url, "http://mmv-mediathek.de:8000/radio98eins");
  DisplaySettings.stationnug[1].gps_ns = 54.527802;
  DisplaySettings.stationnug[1].gps_ew = 13.553500;
  strcpy(DisplaySettings.stationnug[1].town, "Sagard");
  strcpy(DisplaySettings.stationnug[1].countrycode, "DE");
//  strcpy(DisplaySettings.stationnug[1].countryname, "Germany");

  strcpy(DisplaySettings.stationnug[2].name, "P4 Fyn");
  strcpy(DisplaySettings.stationnug[2].url, "http://live-icy.gss.dr.dk:8000/A/A07L.mp3");
  DisplaySettings.stationnug[2].gps_ns = 55.476501;
  DisplaySettings.stationnug[2].gps_ew = 8.459400;
  strcpy(DisplaySettings.stationnug[2].town, "Esbjerg");
  strcpy(DisplaySettings.stationnug[2].countrycode, "DK");
//  strcpy(DisplaySettings.stationnug[2].countryname, "Denmark");

  strcpy(DisplaySettings.stationnug[3].name, "Radio Lapurdi");
  strcpy(DisplaySettings.stationnug[3].url, "https://playerservices.streamtheworld.com/api/livestream-redirect/SER_TUDELA.mp3");
  DisplaySettings.stationnug[3].gps_ns = 42.061401;
  DisplaySettings.stationnug[3].gps_ew = -1.606300;
  strcpy(DisplaySettings.stationnug[3].town, "Tudela");
  strcpy(DisplaySettings.stationnug[3].countrycode, "ES");
//  strcpy(DisplaySettings.stationnug[3].countryname, "Spain");

  // Home Location
  strcpy(DisplaySettings.stationnug[4].name, "Not A Station - You Live Here");
  strcpy(DisplaySettings.stationnug[4].url, "https://icecast.omroep.nl/radio2-bb-mp3");
  DisplaySettings.stationnug[4].gps_ns = 52.250026;
  DisplaySettings.stationnug[4].gps_ew = 4.552549;
  strcpy(DisplaySettings.stationnug[4].town, "Home Sweet Home");
  strcpy(DisplaySettings.stationnug[4].countrycode, "NL");
//  strcpy(DisplaySettings.stationnug[4].countryname, "Netherlands");

  DisplaySettings.bluetoothswitchable = 0;
  DisplaySettings.wifichannel = 11;
  DisplaySettings.bluetoothinstalled = 1;
 
  // default keys are stored in secret.h
  // as defined in ..\secrets.h
  // char google_api_key[] = "YOUR-API-KEY"; // free, get your own at google developer platform, used for timezone retrieval
  // char open_weather_map_api_key[] = "YOUR-API-KEY"; // free, get your own at https://openweathermap.org/api
  strcpy(DisplaySettings.google_api_key, google_api_key);
  strcpy(DisplaySettings.open_weather_map_api_key, open_weather_map_api_key);

  DisplaySettings.expand_search = 5;
  DisplaySettings.auto_update_state = 0;

  // unconditionally save to eeprom
  EEPROM.put(0x0, DisplaySettings);
  EEPROM.commit();
  memcpy(&OldDisplaySettings, &DisplaySettings, sizeof(OldDisplaySettings));
  // also save to file if sd card is present
  SaveEepromToFile();
}

// set some ui items, value or availability
void set_optional_items(void)
{ char content[64];
  sprintf(content, "%d", DisplaySettings.serialnumber);
  lv_label_set_text(ui_SerialNumberText, content);

  if(!DisplaySettings.bluetoothinstalled || !DisplaySettings.bluetoothswitchable) // no BT module present or not switchable
  { lv_obj_add_flag(uic_bluetoothswitch, LV_OBJ_FLAG_HIDDEN); // no switch
    lv_obj_add_flag(uic_bluetoothon, LV_OBJ_FLAG_HIDDEN); // no bt icons
    lv_obj_add_flag(uic_bluetoothoff, LV_OBJ_FLAG_HIDDEN); // no bt icons
  }  
  else
  { if(DisplaySettings.bluetoothswitchable) // do we have a switch
    { Serial.printf("is bluetoothswitchable\n");
      lv_obj_clear_flag(uic_bluetoothswitch, LV_OBJ_FLAG_HIDDEN); // show switch
      if(DisplaySettings.btmodule_power_on) // powered on
      { Serial.printf("bt is powered on\n");
        DataFromDisplay.btmodule_power_on = 1;
        lv_obj_add_state(uic_bluetoothswitch, LV_STATE_CHECKED); // set switch in on position
        lv_obj_add_flag(uic_bluetoothoff, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_clear_flag(uic_bluetoothon, LV_OBJ_FLAG_HIDDEN); // show BT on icon
      }
      else // BT powered off
      { Serial.printf("bt is powered off\n");
        DataFromDisplay.btmodule_power_on = 0;
        lv_obj_clear_state(uic_bluetoothswitch, LV_STATE_CHECKED); // set switch in off position
        lv_obj_add_flag(uic_bluetoothon, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_clear_flag(uic_bluetoothoff, LV_OBJ_FLAG_HIDDEN); // show BT on icon
      }
    }  
    else // not switchable, hide switch, no icons
    { lv_obj_add_flag(uic_bluetoothswitch, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(uic_bluetoothoff, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(uic_bluetoothon, LV_OBJ_FLAG_HIDDEN);
    }
  }  
}

void SetClockHands(void)
{ int16_t HourAngle;
  int16_t MinuteAngle;
  HourAngle = ((datetime.hour * 3600)/12) + ((datetime.minute * 3600)/720);
  MinuteAngle = (ui_MinuteHand, ((datetime.minute * 3600)/60)) + datetime.second;

//  lv_img_set_angle(ui_HourHand, ((datetime.hour * 3600)/12));
//  lv_img_set_angle(ui_MinuteHand, ((datetime.minute * 3600)/60));
  lv_img_set_angle(ui_HourHand, HourAngle);
  lv_img_set_angle(ui_MinuteHand, MinuteAngle);
  lv_img_set_angle(ui_SecondHand, ((datetime.second * 3600)/60));
}


// Check if valid UTF-8 multibyte-sequence starts on s
// return lenght or 0 if not a UTF-8 sequence
static int utf8_sequence_length(const unsigned char *s) {
    if ((s[0] & 0xE0) == 0xC0 &&
        (s[1] & 0xC0) == 0x80)
        return 2;

    if ((s[0] & 0xF0) == 0xE0 &&
        (s[1] & 0xC0) == 0x80 &&
        (s[2] & 0xC0) == 0x80)
        return 3;

    if ((s[0] & 0xF8) == 0xF0 &&
        (s[1] & 0xC0) == 0x80 &&
        (s[2] & 0xC0) == 0x80 &&
        (s[3] & 0xC0) == 0x80)
        return 4;

    return 0; // not valid UTF-8 multibyte 
}


// see https://www.fileformat.info/info/unicode/utf8.htm
void RemoveUTF8Unprintables(char *str) 
{ char output[150]; // increased to 150 - see crash data below
  char *dst;
  uint16_t unc1;
  uint16_t unc2;
  uint16_t unc3;
   
  if(strlen(str)>150)str[149]=0;
  dst = output;

  unsigned char *src = (unsigned char *)str;
  //Serial.printf("RemoveUTF8Unprintables from %s - len = %d\n", str, strlen(str));

  while (*src) 
  { int len = utf8_sequence_length(src);
    if(len == 2)
    { //Serial.printf("RemoveUTF8Unprintables %s ->%d - %d\n", src, (uint16_t)src[0], (uint16_t)src[1]);

      
      unc1 = ((uint16_t)src[0] & 0x1f) << 6;
      unc2 = ((uint16_t)src[1] & 0x3f);
      unc3 = unc1 | unc2; 
      if(unc3<256) // printable UTF-8, so keep it
      { *dst++ = *src++; 
        *dst++ = *src++;
      } 
      else // UTF-8 above 0xff, can't print those, out font does not support that
      { src += len;
      }
    }
    else if (len > 2) 
    { //  Valid UTF-8 multibyte → skip it - can't print these */
      // Serial.printf("UTF-8 multibyte -> %d\n", len);
      src += len;
    } 
    else 
    { // ASCII of extended ASCII byte → keep it 
      // Serial.printf("ASCII of extended ASCII -> %d\n", len);
      *dst++ = *src++;
    }
  }

  *dst = '\0';

  // remove leading " - " (frequently seen as leftover seperator between thai and english resonse like ขอบคุณครับ - Lipta)
  dst = output;
  if(strncmp(output, " - ", 3) == NULL)dst+=3;
  //Serial.printf("Unicode removed from %s ->", str);
  strcpy(str, dst);
  //Serial.printf(" %s\n", str);
  // crash data - very long title received probably 128 
  //Dj BoLiK OFFICIEL X Yuri da Cunha Atchu Tchucha. ReMiX DCaL MAXII VersiOn 2o14 - Dj BoLiK OFFICIEL X Yuri da Cunha Atchu Tchucha
  //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
  //          1         2         3         4         5         6         7         8         9         0         1         2
}


void SaveDisplaySettingsToEeprom(void)
{ if(SaveEepromToFile())
  { // succes
    return;
  }

  EEPROM.put(0x0, DisplaySettings);
  EEPROM.commit();
  memcpy(&OldDisplaySettings, &DisplaySettings, sizeof(OldDisplaySettings));
  Serial.printf("Eeprom saved\n");
}

void LoadDisplaySettingsFromEeprom(void)
{ // first try loading from file
  if(LoadEepromFromFile())
  { memcpy(&OldDisplaySettings, &DisplaySettings, sizeof(OldDisplaySettings));
    Serial.printf("Eeprom loaded from file eeprom.dat\n");
    return;
  }
  // else use eeprom if file does not exist
  EEPROM.get(0x0, DisplaySettings);
  memcpy(&OldDisplaySettings, &DisplaySettings, sizeof(OldDisplaySettings));
  Serial.printf("Eeprom loaded from chip eeprom\n");
}

bool SaveEepromToFile(void)
{ File file;
  char filename[] = {"/eeprom.dat"};
  bool result = false;

  if (!SD_MMC.begin("/sdcard", true, false))return result; // no card

  SD_MMC.remove(filename); // delete old file
  file = SD_MMC.open(filename, FILE_WRITE);
  if(file)
  { file.write((uint8_t *)&DisplaySettings, sizeof(DisplaySettings));  
    result = true;
  }
  file.close();
  SD_MMC.end();
  Serial.printf("Eeprom saved to file eeprom.dat\n");
  memcpy(&OldDisplaySettings, &DisplaySettings, sizeof(OldDisplaySettings));
  return result;
}

bool LoadEepromFromFile(void)
{ File file;
  char filename[] = {"/eeprom.dat"};
  bool result = false;

  if (!SD_MMC.begin("/sdcard", true, false))
  { Serial.printf("No card present\n");
    return result; // no card
  }  
  Serial.printf("Card present\n");

  file = SD_MMC.open(filename, FILE_READ);
  if(file)
  { Serial.printf("Reading eeprom.dat\n");
    file.read((uint8_t *)&DisplaySettings, sizeof(DisplaySettings));  
    result = true;
  }
  file.close();
  SD_MMC.end();
  return result;
}


