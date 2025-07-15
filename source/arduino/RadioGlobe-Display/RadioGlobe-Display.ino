/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
//#include "lv_conf.h"
//#include "lvgl.h"

#include "Wireless.h"
#include "Gyro_QMI8658.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
//#include "LVGL_Example.h"
#include "BAT_Driver.h"

//#include "lv_conf.h"
#include "ui.h"

// esp now
#include <WiFi.h>
#include <esp_wifi.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "franks-esp-now.h"
#include "database.h"
#include "favorites.h"

extern stations_arraybin Stations;

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  // my test board is b4:3a:45:a5:03:10

  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

bool bTimer100ms = false;
bool bUpAndRunning = false;
uint32_t GlobalTicker100mS = 0;
char SecretCode[6] = "12345";  
uint16_t CalibrationModeLatLong = 0;
#define CALMODE_NS   0x01
#define CALMODE_EW   0x02
#define CALMODE_NSEW 0x03

typedef struct ns_ew_intersection
{ int16_t ns;
  int16_t ew; // in tenths of degree
};

// this bin holds the NS/EW coordinates that belong to intersection points of longitudes and latitudes around the globe.
// initially filled with data for a perfect globe.
// as we calibrate, the arry gets more sophisticated and holds the coordinates as the encoders see them on the actual not-so-accurately-printed globe.
// we have two rows for n/s at equator as the upper half and lower haf of the globe may not be aligned very well, as was certainly the case with my globe.
typedef struct calibrations_arraybin
{  ns_ew_intersection calibrations[(180/15)+2][360/15]; // 14 positions for north to south, 0-15-30-45-60-75-90
};

calibrations_arraybin ns_ew_calibrations;
calibrations_arraybin def_cal;
uint16_t CalToIndexNS;
uint16_t CalToIndexEW;
bool bPowerStatus = true;
#define AUTOPOWERDOWNAFTER 3600;
uint32_t AutoSleepTimer = AUTOPOWERDOWNAFTER;
static int backlightvalue = 50;
static int defaultbacklightvalue = 75;



void Driver_Loop(void *parameter)
{
  while(1)
  { GlobalTicker100mS++;
    //QMI8658_Loop();
    RTC_Loop();
    // BAT_Get_Volts();
    bTimer100ms = true;
  // in driver 
//          delay(50);
//          Set_EXIO(EXIO_PIN8,Low);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void Driver_Init()
{
  //Flash_test();
  BAT_Init();
  I2C_Init();
  TCA9554PWR_Init(0x00); // io extender  
  Set_EXIO(EXIO_PIN8,Low); // buzzer off
  PCF85063_Init(); // rtc
  QMI8658_Init(); // gyro

  
  
  xTaskCreatePinnedToCore(
    Driver_Loop,     
    "Other Driver task",   
    4096,                
    NULL,                 
    3,                    
    NULL,                
    0                    
  );
}

void setup()
{ Serial.begin(115200);
  setup_esp_now(); 
  readMacAddress();

  //Wireless_Test2();
  Driver_Init();
  
  delay(1000); // delay added, apperently after new upload, 5V has dropped, give it some time to recover, tested ok with 750, use 1000 for extra margin (FB)

  LCD_Init();                                     // If you later reinitialize the LCD, you must initialize the SD card again !!!!!!!!!!
  SD_Init();                                      // It must be initialized after the LCD, and if the LCD is reinitialized later, the SD also needs to be reinitialized
  Lvgl_Init();

  

  //Lvgl_Example1();
  //lv_demo_widgets();               
  // lv_demo_benchmark();          
  // lv_demo_keypad_encoder();     
  // lv_demo_music();              
  // lv_demo_printer();
  // lv_demo_stress();   

  ui_init();
  lv_obj_add_flag(ui_Power_Off_Icon, LV_OBJ_FLAG_HIDDEN);
  
  
  // load preset stations from favorites.txt file
  LoadFavorites();
  
  // set all calibrations to perfect values
  CalibrationInit();
  // load stored values from setup file, calibrations.txt, from SD card
  LoadCalibrations(); 

  if(SD_MMC.begin("/sdcard", true, false))
  { ReadStationsBitmapFile(SD_MMC, "/stationsmap.bmp"); // read the bmp with the worldmap
    SD_MMC.end();
  }  

 
  bUpAndRunning = true;

  AddToQueueForGlobe("VOL-BASS-TREBLE PLEASE", MESSAGE_DISPLAY_WANTS_VOLUME_AND_TONE);

  // SD card driver for LVGL
  lv_port_fs_init();

}

bool bCheckDatabase = false;
bool bNoTimeZone = true;
bool bInfoScreen = false;



void loop()
{ char *text;
  char content[256];
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
  int16_t stream_connecttohost_result; // received from callback in globe 
  char logfile[32];

  Lvgl_Loop();

  if(bTimer100ms)
  //if(1)
  {  bTimer100ms = false;

     lv_obj_t * screen = lv_scr_act(); //get active screen

     if(oldscreen != screen) // screen changed
     { oldscreen = screen;
       if(bPowerStatus==true)
       { if(screen == ui_CalibrationScreen) // force refresh raw coordinate
         { PrevDataFromGlobe.ns = -1;  
         }
         if(screen == ui_CalibrationScreenAdvanced) // force refresh
         { PrevDataFromGlobe.ns = -1;   
           PrevDataFromDisplay.ns_cal = -1;
         }
         backlightvalue = defaultbacklightvalue;
         Set_Backlight(backlightvalue); 
       }  
       Serial.println("Screen changed!");
       if(screen == ui_Home)
       { CalibrationModeLatLong = (CALMODE_NS | CALMODE_EW); // also effects the display of calibrated coordinates, 
         bInfoScreen = false;
       }
     }

     // process messages from globe
     if(DataFromDisplay.G_QueueSerialNumber != DataFromGlobe.G_QueueSerialNumber) // not yet acknowlegded and processed
     { // acknowledge message from globe as received and processed
       DataFromDisplay.G_QueueSerialNumber = DataFromGlobe.G_QueueSerialNumber; 
       if((DataFromGlobe.G_QueueMessageType>=0) && (DataFromGlobe.G_QueueMessageType<(sizeof(messagetexts)/8)))Serial.printf("GLOBE SAYS: %s >%s<\n", messagetexts[DataFromGlobe.G_QueueMessageType], DataFromGlobe.G_QueueMessage);  
       switch(DataFromGlobe.G_QueueMessageType)
       { case MESSAGE_SONG_TITLE: // 1
           lv_label_set_text(ui_Station_Title, DataFromGlobe.G_QueueMessage);
           break;
         case MESSAGE_ARTIST: // 2
           break;
         case MESSAGE_ALBUM:
           break;
         case MESSAGE_GENRE:
           break;
         case MESSAGE_STATION_NAME:
           lv_label_set_text(ui_Station_Name, DataFromGlobe.G_QueueMessage);
           // rename in scroller if it makes sense
           if(strlen(DataFromGlobe.G_QueueMessage)>63)break;
           if(strlen(DataFromGlobe.G_QueueMessage)<3)break;
           if(Stations.requested<MAX_STATIONS)
           { strcpy(Stations.StationNUG[Stations.requested].name, DataFromGlobe.G_QueueMessage);
             ReloadScroll();
           }
           // else it is a station from the preset (1000 or more)  
           break;
         case MESSAGE_DESCRIPTION:
           break;
         case MESSAGE_GOOGLE_API_KEY:
           break;
         case MESSAGE_TIMEZONE_ID:
           lv_label_set_text(ui_Time_Zone, DataFromGlobe.G_QueueMessage);
           datetime.month = DataFromGlobe.timeinfo.tm_mon;
           datetime.day = DataFromGlobe.timeinfo.tm_mday;
           datetime.dotw = DataFromGlobe.timeinfo.tm_wday;
           datetime.hour = DataFromGlobe.timeinfo.tm_hour; // + (timeinfo.tm_isdst>0)?1:0;
           datetime.minute = DataFromGlobe.timeinfo.tm_min;
           datetime.second = DataFromGlobe.timeinfo.tm_sec;
           PCF85063_Set_All(datetime);
           // also set system wide clock as well, to get the right times and dates on created 
           now = mktime(&DataFromGlobe.timeinfo); // get it in epoch seconds 
//           Serial.printf("NEW GLOBE EPOCH UTC SECONDS = %ld\n", (long)now);
           settimeofday((const timeval *) &now, NULL);
           break;
         case MESSAGE_TIMEZONE_NAME:
           break;
         case MESSAGE_GET_WIFI_CREDENTIALS:
           AddToQueueForGlobe("WAPPIE", MESSAGE_SSID_FOR_GLOBE);
           AddToQueueForGlobe("FRITS", MESSAGE_PASSWORD_FOR_GLOBE);
           break;
         case MESSAGE_WIFI_STATUS:
           break;
         case MESSAGE_CALIBRATE_ZERO:  
           break;
         case MESSAGE_FINDNEWSTATION:
           if(bPowerStatus == true)
           { if((screen != ui_CalibrationScreen) && (screen != ui_CalibrationScreenAdvanced))
             { // if in tone controle screen or preset screen, jump back to home screen
               if((screen != ui_DatabaseScreen) || (bInfoScreen==true))
               { lv_scr_load(ui_Home);
                 backlightvalue = defaultbacklightvalue;
                 Set_Backlight(backlightvalue); 
                 FindNewStation();
                 ReloadScroll();
               }  
             }
           }  
           break;
         case MESSAGE_GLOBE_MAC:  
           break;
         case MESSAGE_GLOBE_IP:  
           break;

         case MESSAGE_CONNECTTOHOST_FAILURE:
           sscanf(DataFromGlobe.G_QueueMessage, "%d", &stream_connecttohost_result);
           break; 


         case MESSAGE_DEAD_STATION:
         case MESSAGE_AUDIO_EOF_STREAM:
           lv_label_set_text(ui_StationRollerComment, "");
           DataFromDisplay.D_QueueStationIndex = -1;
           Stations.playing = -1;
           sprintf(content, "%s - Not Responding", lv_label_get_text(ui_Station_Name));
           lv_label_set_text(ui_Station_Name, content); 
           Lvgl_Loop(); // update screen
           delay(300); // so we can actually notice the text change on the screen

           SD_MMC.end(); //??
           if(SD_MMC.begin("/sdcard", true, false))
           { if(DataFromGlobe.G_QueueMessageType == MESSAGE_DEAD_STATION)
             { sprintf(logfile, "/badstations-%d.txt", stream_connecttohost_result);
               AppendBadStationToFile(SD_MMC, logfile, DataFromGlobe.G_QueueMessage);
             }
             if(DataFromGlobe.G_QueueMessageType == MESSAGE_AUDIO_EOF_STREAM)AppendBadStationToFile(SD_MMC, "/audio-eof-stream.txt", DataFromGlobe.G_QueueMessage);
             SD_MMC.end();
           }  
           

           break;
         case MESSAGE_WANT_NEXT_STATION: // request from globe since it couldn't use the last url
           if(Stations.requested >= MAX_STATIONS) // a problematic preset was ordered
           { // now what, what is a next station in this context?
             SetLed(Stations.requested-MAX_STATIONS,0); // just turn off the led
           }
           else
           { sprintf(content, "%s - Skipped", Stations.StationNUG[Stations.requested].name);
             lv_label_set_text(ui_Station_Name, content); 
             lv_label_set_text(ui_StationRollerComment, content); 
             Lvgl_Loop(); // update screen
             delay(300); // so we can actually notice the text change on the screen
             if(Stations.connect_attempts<MAX_STATIONS)
             { Stations.requested++;
               Stations.requested %= MAX_STATIONS;
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
           }
           break;

         case MESSAGE_STATION_CONNECTED: // 25
           // MESSAGE FROM GLOBE: 25 >https://stream06.dotpoint.nl:8004/stream<
           // update text for station scroller 
           // update leds on preset screen
           if(Stations.requested<MAX_STATIONS+MAX_FAVORITES)
           { sprintf(content, "%s - Playing", Stations.StationNUG[Stations.requested].name);
             Stations.playing = Stations.requested;
             lv_label_set_text(ui_StationRollerComment, content); 
             // lv_roller_set_selected(uic_StationRoller, Stations.requested, LV_ANIM_ON);
             if(Stations.requested<MAX_STATIONS)
             { Serial.printf("(GLOBE SAYS): Station Playing %s\n", Stations.StationNUG[Stations.requested].name);
               SetLed(0,0); SetLed(1,0); SetLed(2,0); SetLed(3,0);
             }
             else
             { Serial.printf("(GLOBE SAYS): Preset Playing %s\n", Stations.StationNUG[Stations.requested].name);
               SetLed(Stations.requested-MAX_STATIONS, UI_THEME_COLOR_GREEN);
             }
           }  
           break;

         case MESSAGE_GLOBE_WANTS_CURRENT_STATION: // 26
            if(Stations.playing>=0)
            { AddStationToQueueForGlobe(Stations.playing);
            }
            break;

         case MESSAGE_VOLUME_AND_TONE:
            { int32_t volume;
              int32_t bass;
              int32_t treble;
              sscanf(DataFromGlobe.G_QueueMessage, "%ld %ld %ld", &volume, &bass, &treble);
              // sync controls to that
              lv_arc_set_value(uic_VolumeArc, volume);
              lv_arc_set_value(uic_BassArc, bass);
              lv_arc_set_value(uic_TrebleArc, treble);

              sprintf(content, "%ld", volume);
              lv_label_set_text(uic_VolumeValue, content); 
              sprintf(content, "%ld", bass);
              lv_label_set_text(uic_BassValue, content); 
              sprintf(content, "%ld", treble);
              lv_label_set_text(uic_TrebleValue, content); 
              AddToQueueForGlobe("VOLUME AND TONE SET AS REQUESTED", MESSAGE_VOLUME_AND_TONE);
              bPowerStatus = true;
            }
            break;
          case MESSAGE_GET_GEOLOCATION_BY_GPS:
            if(Stations.requested<MAX_STATIONS+MAX_FAVORITES)
            { if(strcmp(Stations.StationNUG[Stations.requested].countrycode, DataFromGlobe.G_QueueMessage)!=NULL)
              { Serial.printf("Error in Database - countrycode %s should be %s\n", Stations.StationNUG[Stations.requested].countrycode, DataFromGlobe.G_QueueMessage);
                Serial.printf("                  - gps_ns = %f gps_ew = %f\n", Stations.StationNUG[Stations.requested].gps_ns, Stations.StationNUG[Stations.requested].gps_ew);
                Serial.printf("                  - url = %s\n", Stations.StationNUG[Stations.requested].url);
                Serial.printf("                  - town = %s\n", Stations.StationNUG[Stations.requested].town);
              }
              strcpy(Stations.StationNUG[Stations.requested].countrycode, DataFromGlobe.G_QueueMessage);
            }
            break;
         default:
           Serial.printf("Unsupported message type %d from globe: >%s<\n", DataFromGlobe.G_QueueMessageType, DataFromGlobe.G_QueueMessage);  
           break;
       }
       
     }

     text = lv_label_get_text(ui_VolumeValue);
     strcpy(content, text);
     sscanf(content, "%d", &newvolumevalue);
     if(DataFromDisplay.volumevalue != newvolumevalue)
     { DataFromDisplay.volumevalue = newvolumevalue;
       backlightvalue = defaultbacklightvalue;
       Set_Backlight(backlightvalue); 
       if(newvolumevalue)
       { lv_obj_add_flag(ui_Power_Off_Icon, LV_OBJ_FLAG_HIDDEN);
         lv_obj_clear_state(ui_VolumeValue, LV_STATE_DISABLED);
       }
       else
       { lv_obj_add_state(uic_VolumeValue, LV_STATE_DISABLED); 
         lv_obj_clear_flag(ui_Power_Off_Icon, LV_OBJ_FLAG_HIDDEN);
       }
     }

     text = lv_label_get_text(ui_BassValue);
     strcpy(content, text);
     sscanf(content, "%d", &newbassvalue);
     if(DataFromDisplay.bassvalue != newbassvalue)
     { DataFromDisplay.bassvalue = newbassvalue;
       backlightvalue = defaultbacklightvalue;
       Set_Backlight(backlightvalue); 
     }

     text = lv_label_get_text(ui_TrebleValue);
     strcpy(content, text);
     sscanf(content, "%d", &newtreblevalue);
     if(DataFromDisplay.treblevalue != newtreblevalue)
     { DataFromDisplay.treblevalue = newtreblevalue;
       backlightvalue = defaultbacklightvalue;
       Set_Backlight(backlightvalue); 
     }
     
     if((GlobalTicker100mS % 1)==0)loop_esp_now(); // send volume & other stuff to globe
     

     if((PrevDataFromDisplay.ns_cal !=  DataFromDisplay.ns_cal) || (PrevDataFromDisplay.ew_cal !=  DataFromDisplay.ew_cal))
     { //Serial.println("Update coordinates on lcd!");
       if(screen == ui_CalibrationScreenAdvanced)
       {  GetFormattedLocation(content, "D", CalibrationModeLatLong);
          lv_label_set_text(ui_CalibrationAdvGps, content);
       }
       else if(screen == ui_Home) 
       {  GetFormattedLocation(content, "D", CalibrationModeLatLong);
          lv_label_set_text(ui_HomeGps, content);
       }
       else  if(screen == ui_DatabaseScreen) 
       { lv_obj_set_pos(uic_MapCursor, DataFromDisplay.ew_cal/10, -DataFromDisplay.ns_cal/10); 
       }
       PrevDataFromDisplay.ns_cal = DataFromDisplay.ns_cal;
       PrevDataFromDisplay.ew_cal = DataFromDisplay.ew_cal;
     }

     // new uncalibrated encoder position arrived, remap to calibrated
     if(((PrevDataFromGlobe.ns !=  DataFromGlobe.ns) || (PrevDataFromGlobe.ew !=  DataFromGlobe.ew)) && DataFromGlobe.G_EncoderReliable)
     { //Serial.println("Update coordinates on lcd!");
       remap_ns_ew(DataFromGlobe.ns, DataFromGlobe.ew); // if PrevDataFromGlobe.ns == -1, just refresh the display
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
     }



     if(screen == ui_PasswordScreen)
     { if(strcmp(SecretCode, "GLOBE")!=0)
       { lv_label_set_recolor(ui_lockstatus, true);
         lv_label_set_text(ui_lockstatus, "#FF0000 LOCKED#");
       }
       else
       { lv_label_set_recolor(ui_lockstatus, true);
         lv_label_set_text(ui_lockstatus, "#00FF00 UNLOCKED#");
       }
     }

     // battery indicator consists of 3 slightly different icons on top of one another
     newbatteryvoltage = ((analogReadMilliVolts(BAT_ADC_PIN) * 3) + 50) / 100; // read voltage in tenths of volts times 3 because of voltage divider 
     if((oldbatteryvoltage != newbatteryvoltage) && (!(GlobalTicker100mS%100))) // 
     { oldbatteryvoltage = newbatteryvoltage;
       //Serial.println("newbatteryvoltage = "); Serial.println(newbatteryvoltage);
       sprintf(content, "%d.%dV", newbatteryvoltage/10, newbatteryvoltage%10);
       lv_label_set_text(ui_BatteryVoltage, content);

       if(newbatteryvoltage > 39)
       { lv_obj_add_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
         lv_obj_add_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
         lv_obj_clear_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
       }
       else if(newbatteryvoltage > 37)
       { lv_obj_add_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
         lv_obj_clear_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
         lv_obj_add_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
       }
       else
       { lv_obj_clear_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
         lv_obj_add_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
         lv_obj_add_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
       } 
     }

     if((GlobalTicker100mS % 5)==0)
     { oldminute = datetime.minute;
       oldhour = datetime.hour;
       // Serial.print("Seconds oldsecond: "); Serial.println(oldsecond);
       // sprintf(content, "%02d:%02d:%02d", datetime.hour, datetime.minute, datetime.second);
       sprintf(content, "%02d%c%02d", datetime.hour, ((GlobalTicker100mS % 10)==0)?':':' ', datetime.minute);
       lv_label_set_text(ui_Local_Time, content);
       // brightness test  
       // brightness of 5 is about the lowest value that makes sense, is very dim
       // brightness of 50 is very reasonable brightness
       // brightness of 100 is pretty bright and perhaps not that useful
       // if((Ticker100mS % 10)==0)Set_Backlight(newvolumevalue);                                 
       // else Set_Backlight(0x0);    

       // auto dim backlight and gyro test 
       if((GlobalTicker100mS % 10)==0)
       { static uint16_t freeze = 120;
         getGyroscope();
         uint16_t motion = abs((int)Gyro.x) + abs((int)Gyro.y) + abs((int)Gyro.z); 
         //Serial.printf("x%f - y%f - z%f\n", Gyro.x, Gyro.y, Gyro.z);
         //Serial.printf("motion is %d\n", motion);
         //Serial.printf("motion %d freeze %d backlightvalue %d\n", motion, freeze, backlightvalue);
         if(freeze)freeze--;

         if(bPowerStatus)
         { if(freeze==0)
           { if(backlightvalue>10)Set_Backlight(backlightvalue--);    
           }
           if(motion>30)
           { backlightvalue = defaultbacklightvalue;
             Set_Backlight(backlightvalue);
             freeze = 120; // give our beloved user 120 seconds of full brightness before it starts fading again
           }
         }
         else
         { if(motion>30)
           { backlightvalue = defaultbacklightvalue;
             Set_Backlight(backlightvalue);
           }
           else if(backlightvalue/5)Set_Backlight(backlightvalue-=5);
         }
       }
     }

     if(bCheckDatabase)BuildDatabaseNow();

  }
  delay(5);
  vTaskDelay(pdMS_TO_TICKS(10));
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
