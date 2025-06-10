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
uint32_t GlobalTicker100mS = 0;
char SecretCode[6] = "GLOBE";

void Driver_Loop(void *parameter)
{
  while(1)
  { GlobalTicker100mS++;
    //QMI8658_Loop();
    RTC_Loop();
    // BAT_Get_Volts();
    //loop_esp_now();
    bTimer100ms = true;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void Driver_Init()
{
  //Flash_test();
  BAT_Init();
  I2C_Init();
  TCA9554PWR_Init(0x00);   
  Set_EXIO(EXIO_PIN8,Low);
  PCF85063_Init();
  QMI8658_Init(); 

  
  
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
  setup_esp_now(); // WiFi.begin("WNAP10", "ALIBABA4711");  WiFi.mode(WIFI_AP_STA);
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
  
}

bool bCheckDatabase = false;

void loop()
{ char *text;
  char content[256];
  int newvolumevalue;
  static int newbatteryvoltage;
  static int oldbatteryvoltage;
  static int Ticker100mS = 0;
  static uint8_t oldsecond;
  static uint8_t oldhour;
  static uint8_t oldminute;
  static lv_obj_t * oldscreen;

  Lvgl_Loop();

  if(bTimer100ms)
  {  Ticker100mS++;
     bTimer100ms = false;
     
     text = lv_label_get_text(ui_VolumeValue);
     strcpy(content, text);
     sscanf(content, "%d", &newvolumevalue);
     //Serial.println(content);
     DataFromDisplay.volumevalue = newvolumevalue;
     if((Ticker100mS % 2)==0)loop_esp_now(); // send volume & other stuff to globe
     
     lv_obj_t * screen = lv_scr_act(); //get active screen
     if(oldscreen != screen) // screen changed
     { oldscreen = screen;
       PrevDataFromGlobe.ns = 99999; // force reprint
       PrevDataFromGlobe.ew = 99999; // force reprint
       Serial.println("Screen changed!");
     }

     if((screen == ui_Home) || (screen == ui_CalibrationScreen)) 
     { 
       if((PrevDataFromGlobe.ns !=  DataFromGlobe.ns) || (PrevDataFromGlobe.ew !=  DataFromGlobe.ew))
       { //Serial.println("Update coordinates on lcd!");
         PrevDataFromGlobe.ns =  DataFromGlobe.ns;
         PrevDataFromGlobe.ew =  DataFromGlobe.ew;
         sprintf(content, "%c%d.%d  -  %c%d.%d", (DataFromGlobe.ns<0)?'S':'N', abs(DataFromGlobe.ns)/10, abs(DataFromGlobe.ns)%10, 
                                               (DataFromGlobe.ew<0)?'W':'E', abs(DataFromGlobe.ew)/10, abs(DataFromGlobe.ew)%10);
         if(screen == ui_Home)lv_label_set_text(ui_GPS_Coordinate, content);
         if(screen == ui_CalibrationScreen)lv_label_set_text(ui_GPS_Coordinate_Cal, content);
       }
     } 
     else if(screen == ui_PasswordScreen)
     { if(strcmp(SecretCode, "GLOBE")!=0)
       { lv_label_set_recolor(ui_lockstatus, true);
         lv_label_set_text(ui_lockstatus, "#FF0000 LOCKED#");
       }
       else
       { lv_label_set_recolor(ui_lockstatus, true);
         lv_label_set_text(ui_lockstatus, "#00FF00 UNLOCKED#");
       }
     }

     if(strcmp(PrevDataFromGlobe.TimeZoneId, DataFromGlobe.TimeZoneId) != 0)
     { strcpy(PrevDataFromGlobe.TimeZoneId, DataFromGlobe.TimeZoneId);
       sprintf(content, "%s", DataFromGlobe.TimeZoneId);
       lv_label_set_text(ui_Time_Zone, content);
     }

     if(strcmp(PrevDataFromGlobe.Name, DataFromGlobe.Name) != 0)
     { strcpy(PrevDataFromGlobe.Name, DataFromGlobe.Name);
       lv_label_set_text(ui_Station_Name, DataFromGlobe.Name);
     }

     if(strcmp(PrevDataFromGlobe.Title, DataFromGlobe.Title) != 0)
     { strcpy(PrevDataFromGlobe.Title, DataFromGlobe.Title);
       lv_label_set_text(ui_Station_Title, DataFromGlobe.Title);
     }


//     if(memcmp(&PrevDataFromGlobe, &DataFromGlobe, sizeof(DataFromGlobe))!=0)
//     { memcpy(&PrevDataFromGlobe, &DataFromGlobe, sizeof(DataFromGlobe));
//       Serial.println("Update Name & Title lcd!");
//       lv_label_set_text(ui_Station_Name, DataFromGlobe.Name);
//       lv_label_set_text(ui_Station_Title, DataFromGlobe.Title);
//     }


  
     // battery indicator
     newbatteryvoltage = analogReadMilliVolts(BAT_ADC_PIN) / 100 * 3; // read voltage in tenths of volts times 3 because of voltage divider 
     if(((oldbatteryvoltage != newbatteryvoltage) && (!(Ticker100mS % 100))) || Ticker100mS==1 ) // once per 10 seconds
     { oldbatteryvoltage = newbatteryvoltage;
       //Serial.println("newbatteryvoltage = "); Serial.println(newbatteryvoltage);
       sprintf(content, "%d.%dV", newbatteryvoltage/10, newbatteryvoltage%10);
       lv_label_set_text(ui_BatteryVoltage, content);
     }

     // clock stuff
     if((PrevDataFromGlobe.timeinfo.tm_hour != DataFromGlobe.timeinfo.tm_hour) || (PrevDataFromGlobe.timeinfo.tm_min != DataFromGlobe.timeinfo.tm_min))
     { PrevDataFromGlobe.timeinfo.tm_hour = DataFromGlobe.timeinfo.tm_hour;
       PrevDataFromGlobe.timeinfo.tm_min = DataFromGlobe.timeinfo.tm_min;
       datetime.month = DataFromGlobe.timeinfo.tm_mon;
       datetime.day = DataFromGlobe.timeinfo.tm_mday;
       datetime.dotw = DataFromGlobe.timeinfo.tm_wday;
       datetime.hour = DataFromGlobe.timeinfo.tm_hour; // + (timeinfo.tm_isdst>0)?1:0;
       datetime.minute = DataFromGlobe.timeinfo.tm_min;
       datetime.second = DataFromGlobe.timeinfo.tm_sec;
       PCF85063_Set_All(datetime);
     }

     if(PrevDataFromGlobe.FindNewStation != DataFromGlobe.FindNewStation)
     { PrevDataFromGlobe.FindNewStation = DataFromGlobe.FindNewStation;
       if(DataFromGlobe.FindNewStation == 1)
       { FindNewStation();
       }  
     }



//     sprintf(content, "%c%d.%d  -  %c%d.%d", (DataFromDisplay.ns_cal<0)?'S':'N', abs(DataFromDisplay.ns_cal)/10, abs(DataFromDisplay.ns_cal)%10, 
//                                               (DataFromDisplay.ew_cal<0)?'W':'E', abs(DataFromDisplay.ew_cal)/10, abs(DataFromDisplay.ew_cal)%10);
//     Serial.println(content);

      //sprintf(content, "Tijd %02d:%02d:%02d", datetime.hour, datetime.minute, datetime.second);
      //Serial.println(content);
      //if((oldhour != datetime.hour) || (oldminute != datetime.minute))
      if((Ticker100mS % 5)==0)
      { oldminute = datetime.minute;
        oldhour = datetime.hour;
        // Serial.print("Seconds oldsecond: "); Serial.println(oldsecond);
        // sprintf(content, "%02d:%02d:%02d", datetime.hour, datetime.minute, datetime.second);
        sprintf(content, "%02d%c%02d", datetime.hour, ((Ticker100mS % 10)==0)?':':' ', datetime.minute);
        lv_label_set_text(ui_Local_Time, content);
      }

      if(bCheckDatabase)BuildDatabaseFromSD();

  }

  vTaskDelay(pdMS_TO_TICKS(5));
}
