// handling of events on various screens, beeps and long button presses

void FuzzinessButtonL(lv_event_t * e)
{ char content[128];
  lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { if(DisplaySettings.expand_search>2)DisplaySettings.expand_search--;
    sprintf(content, "FUZZINESS %d", DisplaySettings.expand_search);
    lv_label_set_text(uic_SetFuzzinessValue, content);
  }
}

void FuzzinessButtonR(lv_event_t * e)
{ char content[128];
  lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { if(DisplaySettings.expand_search<100)DisplaySettings.expand_search++;
    sprintf(content, "FUZZINESS %d", DisplaySettings.expand_search);
    lv_label_set_text(uic_SetFuzzinessValue, content);
  }
}





#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
void MapClicked(lv_event_t * e)
{ lv_point_t p;
  lv_indev_t *indev = lv_indev_get_act();
  lv_indev_type_t indev_type = lv_indev_get_type(indev);
 
  if(bDatabaseScreenUpdate)return; // not wanted it here
  
  lv_indev_get_point(indev, &p);
  p.y = 240 - p.y; // reverse direction and calculate from center
  p.x -= 240; // calculate from center
  p.x = MIN(p.x,179);
  p.y = MIN(p.y,89);
  p.x = MAX(p.x,-179);
  p.y = MAX(p.y,-89);
  Serial.printf("x=%d y=%d\n", p.x, p.y);


  DataFromDisplay.ns_cal = p.y * 10;
  DataFromDisplay.ew_cal = p.x * 10;
  FindNewStation();


  beepforMs(50);
}



// cog wheel on Home Screen
void SetupEnter(lv_event_t * e)
{ char content[128];
  if(strcmp(SecretCode, "GLOBE") != 0)
  { lv_scr_load(ui_PasswordScreen);
    return;
  }
  strcpy(SecretCode, "12345"); // relock again
  // activate 1st Setup Screen
  if(Stations.requested<MAX_STATIONS+MAX_FAVORITES)
  { // show current flag and text 
    if(strcmp(Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode, Stations.StationNUG[Stations.requested].countrycode)==NULL)lv_label_set_text(uic_YouLiveHere, "You Live Here");
    //else lv_label_set_text(uic_YouLiveHere, "You Are Here");
    lv_obj_clear_flag(uic_HomeFlagToStore, LV_OBJ_FLAG_HIDDEN); // show country flag again
    lv_obj_clear_flag(uic_YouLiveHere, LV_OBJ_FLAG_HIDDEN); // show country flag again
  }

  sprintf(content, "FUZZINESS %d", DisplaySettings.expand_search);
  lv_label_set_text(uic_SetFuzzinessValue, content);

  lv_scr_load(ui_SetupScreen);
  Lvgl_Loop();
}

// calibration button on setup screen
void CalibrationEnter(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    lv_scr_load(ui_CalibrationScreen);
    Lvgl_Loop();
  }  
}

void DatabaseEnter(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    Serial.printf("CheckDatabase clicked from SetupScreen with SecretCode=%s\n", SecretCode);
    lv_obj_clear_flag(uic_RebuildDatabase, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(uic_RebuildDatabaseButtonText, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(uic_Database_Flag, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(uic_Database_Town_Name, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_clear_flag(uic_MapBanner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(uic_Database_Dir_Path, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Database_GPS_Position, LV_OBJ_FLAG_HIDDEN); 


    lv_label_set_text(ui_Database_Progress, "");
    lv_label_set_text(ui_Database_Dir_Path, "");
    lv_label_set_text(ui_Database_Output_File, "Long Press Button To Start");
//    lv_obj_add_state(uic_MapBanner, LV_STATE_DISABLED); 
    lv_obj_add_state(uic_MapCursor, LV_STATE_DISABLED); 
    
    lv_scr_load(ui_DatabaseScreen);
    Lvgl_Loop();  
    strcpy(SecretCode,"12345"); // have to enter again to be sure
    bDatabaseScreenUpdate = true; // we are in the update version of the worldmap screen
    ShowTheStations(); // the red speckled map
  }
}

void ResetAllEnter(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { // set all calibrations to perfect values
    CalibrationInit();
    // save calibrations 
    SaveCalibrations();   
    beepforMs(1000);
  }
}

void SetHomeEnter(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { // set current selected radio station as home position 
    // also send country code to globe - for use in valuta exchange rates
    if(Stations.requested<MAX_STATIONS+MAX_FAVORITES)
    { // store current station as home location, as it has country code, country name and gps coordinates
      Serial.printf("You live In %s\n", Stations.StationNUG[Stations.requested].countryname);
      memcpy(&Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)], &Stations.StationNUG[Stations.requested], sizeof(Stations.StationNUG[0]));
      strcpy(Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].name, "Not A Station - You Live Here");
      strcpy(Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].town, "Home Sweet Home");
      SaveFavorites();
      AddToQueueForGlobe(Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode, MESSAGE_THIS_IS_HOME);
      beepforMs(1000);
    }
  }
}

// returns higher value when longer pressed
uint16_t isLongPressed(lv_event_t * e)
{ static uint16_t longpressed=0;

  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) // 6
  { //Serial.printf("Eventcode = LV_EVENT_LONG_PRESSED_REPEAT\n");
    longpressed++;
  }
  else if(event_code == LV_EVENT_CLICKED) // 7
  { //Serial.printf("Eventcode = LV_EVENT_CLICKED\n");
    longpressed = 0;
  }
  //Serial.printf("longpressed = %d\n", longpressed);
  return longpressed;
}

void beep(lv_event_t * e)
{ beepforMs(50);
} 

void beepforMs(uint16_t ms)
{ Set_EXIO(EXIO_PIN8,High);
  delay(ms);
  Set_EXIO(EXIO_PIN8,Low); // end of beep
}

// handle the OLEGB buttons for secret code
void click1(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_CLICKED) // 7
  { strncpy(SecretCode, &SecretCode[1], 5); 
    SecretCode[4] = 'O';
    Serial.println(SecretCode);
  }
  
  if(isLongPressed(e)==5) // after typing OOOOO followed by long press O, shut down
  { if(strcmp(SecretCode, "OOOOO")==0)
    { // enter deep sleep to save battery
      esp_deep_sleep_start();
    }
  }
}
void click2(lv_event_t * e)
{ strncpy(SecretCode, &SecretCode[1], 5); 
  SecretCode[4] = 'L';
  Serial.println(SecretCode);
}
void click3(lv_event_t * e)
{ strncpy(SecretCode, &SecretCode[1], 5); 
  SecretCode[4] = 'E';
  Serial.println(SecretCode);
}
void click4(lv_event_t * e)
{ strncpy(SecretCode, &SecretCode[1], 5); 
  SecretCode[4] = 'G';
  Serial.println(SecretCode);
}
void click5(lv_event_t * e)
{ strncpy(SecretCode, &SecretCode[1], 5); 
  SecretCode[4] = 'B';
  Serial.println(SecretCode);
}

void SerialNumberButton(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_CLICKED) // 7
  { Serial.println("SerialNumber clicked");
  }
  if(isLongPressed(e)==5) // after typing OOOOO followed by long press O, shut down
  { Serial.println("SerialNumber long pressed");
    beepforMs(1000);
    lv_scr_load(ui_Home);
    AddToQueueForGlobe("1", MESSAGE_UPDATE_GLOBE);
  }
}




void PowerCycle(lv_event_t * e)
{ if(isLongPressed(e)==2)
  { Serial.printf("Power button long pressed\n");
    handlePowerCycle();
  }
}

void handlePowerCycle(void)
{ if(AutoSleepTimer)beepforMs(50); // don't beep if we arrive here by countdown of AutoSleepTimer, do it all silente
    
    if(bPowerStatus == true)
    { // Power off
      bPowerStatus = false;
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_red);
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_red);
      lv_obj_invalidate(ui_Home_Power_Off_Icon);
      lv_img_set_src(ui_Clock_Power_Off_Icon, &ui_img_power75x75_png); // icon on clock face
      ui_object_set_themeable_style_property(ui_Clock_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_red);
      ui_object_set_themeable_style_property(ui_Clock_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_red);
      lv_obj_invalidate(ui_Clock_Power_Off_Icon);
      lv_refr_now(NULL);
      Lvgl_Loop();  

      
      if(AutoSleepTimer) AddToQueueForGlobe("OFF", MESSAGE_POWERDOWN); // initiated by user
      else AddToQueueForGlobe("OFF", MESSAGE_SILENT_POWER_DOWN); // intiated by countdown of AutoSleepTimer
      
      // power down, backlight down
      while(BacklightValue)
      { Set_Backlight(--BacklightValue);    
        delay(15);
      }
      delay(100);
      SetClockHands(); // quicly update neglected clock  
      ClockBackLight = true;
      lv_scr_load(ui_ClockScreen);
      lv_refr_now(NULL);
      Lvgl_Loop();  
      //BacklightValue = DEFAULT_BACKLIGHT;
      while(BacklightValue < DEFAULT_BACKLIGHT) // gently fade up
      { Set_Backlight(++BacklightValue);    
        delay(15);
      }
    }
    else
    { // Power on
      char content[128];
      sprintf(content, "Sleeptimer =% d -> %d by handlePowerCycle doing a power on\n", AutoSleepTimer, AUTOPOWERDOWNAFTER);
      AppendToLogFile("/Sleeptimer.log", content);
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
      bPowerStatus = true;
      ClockBackLight = true;
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
      lv_obj_invalidate(ui_Home_Power_Off_Icon);
      ui_object_set_themeable_style_property(uic_Clock_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
      ui_object_set_themeable_style_property(uic_Clock_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
      lv_obj_invalidate(uic_Clock_Power_Off_Icon);
      lv_obj_add_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN);
      lv_refr_now(NULL);
      Lvgl_Loop();  

      AddToQueueForGlobe("ON", MESSAGE_POWERUP);
      delay(1000);
      lv_scr_load(ui_Home);
    }
}

void SaveVolTone(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { // tell globe to save
    AddToQueueForGlobe("SAVE VOLUME AND TONE CONTROLS", MESSAGE_STORE_VOLUME_AND_TONE);
    SaveDisplaySettings();
    beepforMs(1000);
  }
}

// on Home screen, flag is pressed pressed
void StationInfo(lv_event_t * e)
{ if(isLongPressed(e)==5) // actually not used
  { beepforMs(1000);
  }
  else
  { // use database screen to show where station is located on worldmap with gps coordinates
    beepforMs(50);
    Serial.printf("Station %d Info clicked from Home screen\n", Stations.playing);
    lv_obj_add_flag(uic_RebuildDatabase, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(uic_RebuildDatabaseButtonText, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_add_flag(uic_MapBanner, LV_OBJ_FLAG_HIDDEN);  // scroll over text
    lv_obj_add_flag(uic_Database_Dir_Path, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_Database_GPS_Position, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(uic_Database_Town_Name, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(uic_MapCursor, LV_STATE_DISABLED); 
    lv_obj_clear_flag(uic_Database_Flag, LV_OBJ_FLAG_HIDDEN);
    lv_scr_load(ui_DatabaseScreen);

    ShowTheStations(); // the red speckled map
    lv_scr_load(ui_DatabaseScreen);
    bInfoScreen = true;
  }
}


// flags are stored on SD card as bin files, converted from png originals, using https://lvgl.io/tools/imageconverter LVGLv8, Color format -> CF_TRUE_COLOR_ALPHA Output format -> Binary RGB565 format
// immage descriptor and data for flag
lv_img_dsc_t my_flag_img;
uint8_t my_flag_img_data[96*72*3]; // 20736 bytes

void SetFlag(char *countrycode)
{ char lowercasecode[8];
  char path[32];
  if(strlen(countrycode)>0)strcpy(lowercasecode, countrycode);
  else strcpy(lowercasecode, "xxxx");
  for(int i = 0; lowercasecode[i]; i++)
  { lowercasecode[i] = tolower(lowercasecode[i]);
  }
  if(strcmp(countrycode, "??")==NULL)strcpy(lowercasecode, "xxxx");
  if(strcmp(countrycode, "XX")==NULL)strcpy(lowercasecode, "xx-sea");
  
  
  sprintf(path, "/flags-bin/%s.bin", lowercasecode );
  Serial.printf("Setflag with %s\n", path);

  my_flag_img.header.always_zero = 0;
  my_flag_img.header.w = 96;
  my_flag_img.header.h = 72;
  my_flag_img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
  my_flag_img.data = my_flag_img_data;

  SD_MMC.begin("/sdcard", true, false);
  File fp = SD_MMC.open(path, FILE_READ);
  if(fp!=NULL)
  { fp.read((uint8_t *)my_flag_img_data, 4); // header, basically skipping it, header is already filled above
    fp.read((uint8_t *)my_flag_img_data, sizeof(my_flag_img_data)); // header
    fp.close();
    //Serial.printf("Setflag loaded\n");
  }  
  SD_MMC.end();

  
  lv_img_set_src(uic_Home_Flag, &my_flag_img);
  lv_img_set_src(uic_Database_Flag, &my_flag_img);
  lv_img_set_src(uic_HomeFlagToStore, &my_flag_img);
  lv_img_set_src(uic_ClockFlag, &my_flag_img);
  lv_img_set_src(ui_PresetFlag, &my_flag_img);

}


static lv_img_dsc_t StationMap_img_dsc;
lv_obj_t * canvas_obj;

void ShowTheStations(void)
{ // this took me a whole day
  StationMap_img_dsc.header.always_zero = 0;
  StationMap_img_dsc.header.w = 384;
  StationMap_img_dsc.header.h = 180;
  StationMap_img_dsc.header.cf = LV_IMG_CF_ALPHA_1BIT; 
  StationMap_img_dsc.data = StationsMap.pixeldata;
//  lv_obj_t * canvas_obj = lv_canvas_create(ui_WorldMap);
  canvas_obj = lv_canvas_create(ui_WorldMap);
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_img_recolor(&style, lv_color_hex(0xff0000));
  lv_style_set_img_recolor_opa(&style, LV_OPA_100); 
  lv_obj_add_style(canvas_obj, &style, LV_PART_MAIN | LV_STATE_DEFAULT); 
  lv_canvas_set_buffer(canvas_obj, StationsMap.pixeldata, 384, 180, LV_IMG_CF_ALPHA_1BIT);
  // wow
}

void SpeakerToggle(lv_event_t * e)
{ //Serial.println("SpeakerToggle");
  if(lv_obj_has_state(uic_InternalSpeaker, LV_STATE_CHECKED))
  { DataFromDisplay.internalspeakeron = 1;
    //AddToQueueForGlobe("INTERNAL SPEAKER ON", MESSAGE_INTERNAL_SPEAKER_ON);
    lv_obj_add_flag(uic_speakeroff, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_speakeron, LV_OBJ_FLAG_HIDDEN);
  }
  else 
  { //AddToQueueForGlobe("INTERNAL SPEAKER OFF", MESSAGE_INTERNAL_SPEAKER_OFF);
    DataFromDisplay.internalspeakeron = 0;
    lv_obj_add_flag(uic_speakeron, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_speakeroff, LV_OBJ_FLAG_HIDDEN);
  }
}

// Clock flag is a toggle to select between home location time and local time of globe position
void ClockFlagToggle(lv_event_t * e)
{ char content[64];
  ClockHomeTime = !ClockHomeTime;
  beepforMs(50);

  // toggles between home and possibly other location on globe
  // todo - two structures for home/world, each aware of timezone, names and texts for the clock, for a faster switch between the two
  // that would save the time of doing an actual lookup every time
  if(ClockHomeTime)
  { Serial.println("ClockFlagToggle = HOME");
    // use home location as stored in favorites.txt as 5th record to get timezone and place
    DataFromDisplay.D_StationGpsNS = Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].gps_ns;
    DataFromDisplay.D_StationGpsEW = Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].gps_ew;
    sprintf(content, "%f-%f", DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW);
    //AddToQueueForGlobe("DISPLAY WANTS MQTT STATUS", MESSAGE_MQTT_STATUS); 
    if(strncmp(DisplaySettings.home_tz_posix, "CUSTOM", 5)!=0)AddToQueueForGlobe(content, MESSAGE_GET_HOME_TIMEZONE);  
    else AddToQueueForGlobe(content, MESSAGE_GET_HOME_TIMEZONE);  // todo -> change timezone system clock
    //lv_label_set_text(ui_Clock_Country, AllUpperCase(Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countryname));
    // Home.CountryName, in setup(), intitialized with "HOME SWEET HOME", which is a nicer alternative
    lv_label_set_text(ui_Clock_Country, Home.CountryName);
    //lv_label_set_text(ui_Time_Zone_Clock, Home.TZname); // on clock screen
  }
  else 
  { Serial.println("ClockFlagToggle = WORLD");
    DataFromDisplay.D_StationGpsNS = Stations.StationNUG[Stations.requested].gps_ns;
    DataFromDisplay.D_StationGpsEW = Stations.StationNUG[Stations.requested].gps_ew;
    sprintf(content, "%f-%f", DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW);
    //AddToQueueForGlobe("DISPLAY WANTS MQTT STATUS", MESSAGE_MQTT_STATUS); 
    AddToQueueForGlobe(content, MESSAGE_GET_TIMEZONE_BY_GPS);  
    //lv_label_set_text(ui_Clock_Country, AllUpperCase(Stations.StationNUG[Stations.requested].countryname));
    lv_label_set_text(ui_Clock_Country, World.CountryName);
    //lv_label_set_text(ui_Time_Zone_Clock, World.TZname); // on clock screen
  }

}

// Touching location/time on Home screen, switches over to clock
void SwitchToClock(lv_event_t * e)
{ char content[64];
  beepforMs(50);
  // set icon to home icon
  lv_img_set_src(ui_Clock_Power_Off_Icon, &ui_img_home_png);
  ui_object_set_themeable_style_property(ui_Clock_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_turquoise);
  ui_object_set_themeable_style_property(ui_Clock_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_turquoise);

  ClockHomeTime = false; // to sync the clocks Flag toggle state 
  lv_scr_load(ui_ClockScreen);
}

// clicking home or power icon on clock face
void ClockHomePower(lv_event_t * e)
{ char content[64];
  beepforMs(50);

  if(bPowerStatus == true)
  { //Serial.printf("Clock Home/Power clicked while power is on\n");
    if(ClockHomeTime) // set back to world status
    { ClockFlagToggle(e);
      if(strcmp(ClockFlagCountryCode, Stations.StationNUG[Stations.requested].countrycode)!=NULL)
      { lv_obj_add_flag(ui_Home_Flag, LV_OBJ_FLAG_HIDDEN);
      }
    }
    
    lv_scr_load(ui_Home);
    lv_img_set_src(ui_Clock_Power_Off_Icon, &ui_img_power75x75_png); // icon on clock face
  }
  else
  { //Serial.printf("Clock Home/Power clicked while power is off\n");
    bPowerStatus = true;
    ClockBackLight = true;
    ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
    ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
    lv_obj_invalidate(ui_Home_Power_Off_Icon);
    lv_obj_add_flag(ui_Home_Flag, LV_OBJ_FLAG_HIDDEN);
    lv_refr_now(NULL);
    Lvgl_Loop();  
    AddToQueueForGlobe("ON", MESSAGE_POWERUP);
    delay(1000);
    lv_scr_load(ui_Home);
  } 

}

void ClockFaceClick(lv_event_t * e)
{ AutoSleepTimer = AUTOPOWERDOWNAFTER;
  if(BacklightValue==9) // auto dimmed down
  { // just wake up backlight
    // BacklightValue = DEFAULT_BACKLIGHT;
    while(BacklightValue < DEFAULT_BACKLIGHT) // gently fade up
    { Set_Backlight(++BacklightValue);    
      delay(15);
    }
  }
  else
  { ClockBackLight = !ClockBackLight;
    if(ClockBackLight)
    { //BacklightValue = DEFAULT_BACKLIGHT;
      while(BacklightValue < DEFAULT_BACKLIGHT) // gently fade up
      { Set_Backlight(++BacklightValue);    
        delay(15);
      }
      Serial.println("ClockBacklight = true");
    }
    else
    { Serial.println("ClockBacklight = false");
      while(BacklightValue)
      { Set_Backlight(--BacklightValue);    
        delay(15);
      }
    }
  }
  Serial.printf("Clock Face click\n");
}



void BlueToothToggle(lv_event_t * e)
{ //Serial.println("Bluetooth Toggle");
  if(lv_obj_has_state(uic_bluetoothswitch, LV_STATE_CHECKED))
  { DataFromDisplay.btmodule_power_on = 1;
    DisplaySettings.btmodule_power_on = 1;
    lv_obj_add_flag(uic_bluetoothoff, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_bluetoothon, LV_OBJ_FLAG_HIDDEN);
  }
  else 
  { DataFromDisplay.btmodule_power_on = 0;
    DisplaySettings.btmodule_power_on = 0;
    lv_obj_add_flag(uic_bluetoothon, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_bluetoothoff, LV_OBJ_FLAG_HIDDEN);
    DataFromDisplay.internalspeakeron = 1;
    lv_obj_add_state(uic_InternalSpeaker, LV_STATE_CHECKED); // force speakers when BT off
    lv_obj_add_flag(uic_speakeroff, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_speakeron, LV_OBJ_FLAG_HIDDEN);
  }
}

// immage descriptor and data for weather icon
lv_img_dsc_t my_global_weather_img;
uint8_t my_global_weather_img_data[70*60*3]; // 12600 bytes

void SetWeatherData(char *settings)
{ char path[32];
  float temperature;
   int temperaturerounded;
  int humidity;
  char icon[8];
  char content[32];
  // example "Temp 25.3 Rh 8 Icon 01d"

  sscanf(settings, "Temp %f Rh %d Icon %s", &temperature, &humidity, &icon[0]);

//  sprintf(content, "%.1f°C", temperature);
  
  temperaturerounded = (temperature + 0.5);
  sprintf(content, "%d °C", temperaturerounded);
  

  lv_label_set_text(uic_Weather_Temperature, content);
  sprintf(content, "Rh %d%%", humidity);
  lv_label_set_text(uic_Weather_Humidity, content);

  sprintf(path, "/weather-bin/%s-70x60.bin", icon);

  my_global_weather_img.header.always_zero = 0;
  my_global_weather_img.header.w = 70;
  my_global_weather_img.header.h = 60;
  my_global_weather_img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
  my_global_weather_img.data = my_global_weather_img_data;

  SD_MMC.begin("/sdcard", true, false);
  File fp = SD_MMC.open(path, FILE_READ);
  fp.read((uint8_t *)my_global_weather_img_data, 4); // header, basically skipping it, header is already filled above
  fp.read((uint8_t *)my_global_weather_img_data, sizeof(my_global_weather_img_data)); // data
  fp.close();
  SD_MMC.end();

  lv_img_set_src(uic_Weather_Icon, &my_global_weather_img);

}

void ShowWeatherData(bool state)
{ if(state == false)
  { //Serial.println("Hide Weather Stuff");
    lv_obj_add_flag(uic_Weather_Icon, LV_OBJ_FLAG_HIDDEN); // hide weather icon until new weather data is received
    lv_obj_add_flag(uic_Weather_Temperature, LV_OBJ_FLAG_HIDDEN); // hide weather temperature until new weather data is received
    lv_obj_add_flag(uic_Weather_Humidity, LV_OBJ_FLAG_HIDDEN); // // hide weather humidity until new weather data is received
  }
  else
  { //Serial.println("Show Weather Stuff");
    lv_obj_clear_flag(uic_Weather_Icon, LV_OBJ_FLAG_HIDDEN); // hide weather icon until new weather data is received
    lv_obj_clear_flag(uic_Weather_Temperature, LV_OBJ_FLAG_HIDDEN); // hide weather temperature until new weather data is received
    lv_obj_clear_flag(uic_Weather_Humidity, LV_OBJ_FLAG_HIDDEN); // // hide weather humidity until new weather data is received
  }
}

void ShowBatteryLevel(int newbatteryvoltage) // 0 -> hide, 1 -> show hide weather icon and show battery icons, negative -> only update value 
{ static int abs_newvoltage;
  static int battery_voltage = -1;
  static int actbatterypercentage = 1;
  static int battery_percentage = -1;
  char content[32];

  if(newbatteryvoltage == 0) // means hide battery
  { lv_obj_add_flag(uic_BatteryLevelHome, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN); 
    lv_event_send(uic_BatteryLevelHome, LV_EVENT_REFRESH, NULL);
    lv_event_send(uic_Battery_Icon_High, LV_EVENT_REFRESH, NULL);
    lv_event_send(uic_Battery_Icon_Low, LV_EVENT_REFRESH, NULL);
    lv_event_send(uic_Battery_Icon_Medium, LV_EVENT_REFRESH, NULL);
    return;
  }

  if(newbatteryvoltage<0) // when negative it is just store and return
  { if(abs_newvoltage != -newbatteryvoltage)
    { abs_newvoltage = -newbatteryvoltage;
      
      // track voltage, one step up/down at the time 
      if(battery_voltage = -1) battery_voltage = abs_newvoltage;
      if(battery_voltage < abs_newvoltage)battery_voltage++;
      if(battery_voltage > abs_newvoltage)battery_voltage--;

      // map voltage to percentage 
      if(battery_voltage>=410)actbatterypercentage = 100;
      else if(battery_voltage>=406)actbatterypercentage = map(battery_voltage, 406, 409, 90, 100);  //  90; 
      else if(battery_voltage>=398)actbatterypercentage = map(battery_voltage, 398, 405, 80, 89);  //  80; 
      else if(battery_voltage>=392)actbatterypercentage = map(battery_voltage, 392, 397, 70, 79);  //  70; 
      else if(battery_voltage>=387)actbatterypercentage = map(battery_voltage, 387, 391, 60, 69);  //  60; 
      else if(battery_voltage>=375)actbatterypercentage = map(battery_voltage, 375, 386, 50, 59);  //  50; 
      else if(battery_voltage>=367)actbatterypercentage = map(battery_voltage, 367, 374, 40, 49);  //  40; 
      else if(battery_voltage>=360)actbatterypercentage = map(battery_voltage, 360, 366, 30, 39);  //  30; 
      else if(battery_voltage>=345)actbatterypercentage = map(battery_voltage, 345, 359, 20, 29);  //  20; 
      else if(battery_voltage>=330)actbatterypercentage = map(battery_voltage, 330, 344, 10, 19);  //  10;
      else if(battery_voltage>=300)actbatterypercentage = map(battery_voltage, 300, 329, 0, 9);  //  1;
      else battery_voltage = 0;

      // track percentage, one step up/down at the time
      if(battery_percentage==-1)battery_percentage = actbatterypercentage;
      if(battery_percentage < actbatterypercentage)battery_percentage++;
      if(battery_percentage > actbatterypercentage)battery_percentage--;

      sprintf(content, "%d%%", battery_percentage);
      lv_label_set_text(ui_BatteryLevelHome, content);
      lv_label_set_text(ui_BatteryLevel, content);
    }
    return;
  }

  ShowWeatherData(false);

  sprintf(content, "%d%%", battery_percentage);
  lv_label_set_text(ui_BatteryLevelHome, content);
  lv_label_set_text(ui_BatteryLevel, content);


  lv_obj_clear_flag(uic_BatteryLevelHome, LV_OBJ_FLAG_HIDDEN); 
    
  if(actbatterypercentage > 90)
  { lv_obj_add_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
  }
  else if(actbatterypercentage > 50)
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



// put the globe in 'play from SD' mode
void GlobePlaySD(lv_event_t * e)
{ char content[128];
  lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_CLICKED) 
  { SetLed(0,0); SetLed(1,0); SetLed(2,0); SetLed(3,0);
    AddToQueueForGlobe("", MESSAGE_GLOBE_PLAY_SD);
  }

  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { if(isLongPressed(e)==5)
    { // beepforMs(1000);
    }
  }  
}

// returns higher value when longer pressed
int16_t isLongPressedV2(lv_event_t * e)
{ static int16_t longpressed=0;

  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) // 6
  { if(longpressed<0)longpressed=0;
    if(longpressed<1000)longpressed++;
    //Serial.printf("isLongPressedV2 Eventcode = LV_EVENT_LONG_PRESSED_REPEAT %d\n", longpressed);
    return longpressed;  
  }
  else if(event_code == LV_EVENT_CLICKED) // 7
  { //Serial.printf("isLongPressedV2 Eventcode = LV_EVENT_CLICKED %d\n",longpressed);
    if(longpressed>0)longpressed = -1;
    else longpressed = 0;
    //Serial.printf("isLongPressedV2 Eventcode = LV_EVENT_CLICKED %d\n",longpressed);
    return longpressed;  
  }
  //Serial.printf("longpressed = %d\n", longpressed);
  return longpressed;
}




