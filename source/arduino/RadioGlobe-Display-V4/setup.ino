// handling of events on various screens, beeps and long button presses

void JukeboxClicked(lv_event_t * e)
{ static uint16_t longpressed=0;
  lv_event_code_t code = lv_event_get_code(e);
  
  longpressed = ShortOrLongPressed(e);
  //Serial.printf("longpressed is %d\n", longpressed);

  if(longpressed==5)
  { beepforMs(50);
    return;
  }
  else if(longpressed==0)
  { AddToQueueForGlobe("", MESSAGE_GLOBE_PLAY_SD); // next song in list
    beepforMs(50);
  }
}

// returns 0 for click, 1-?? for longer pressed
uint16_t ShortOrLongPressed(lv_event_t * e)
{ static uint16_t longpressed=0;

  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) // 6
  { //Serial.printf("Eventcode = LV_EVENT_LONG_PRESSED_REPEAT\n");
    longpressed++;
  }
  
  if(event_code == LV_EVENT_CLICKED) // 7
  { //Serial.printf("Eventcode = LV_EVENT_CLICKED\n");
    if(longpressed>1)longpressed = 1;
    else longpressed = 0;
  }
  //Serial.printf("longpressed = %d\n", longpressed);
  return longpressed;
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


void RadarScreenOn(lv_event_t * e)
{ // let's do a flight radar
  // now screen can switch over
  extern lv_obj_t * uic_RadarScreen;
  lv_scr_load(uic_RadarScreen);
  beepforMs(50);
}

// main screen unloaded
void OnMainScreenUnLoaded(lv_event_t * e)
{ 
}


// automatic update clock hands before showing clock
void OnClockScreenLoad(lv_event_t * e)
{ // before screen becomes visible, sync the clock hands

  lv_event_code_t code = lv_event_get_code(e);

  RTC_Loop(); // after a power on boot, fetch time early
  lv_obj_clear_flag(ui_ClockFlag, LV_OBJ_FLAG_HIDDEN); // in case it was hidden by a search 
  ForceSetClockHands();
  
  if(code == LV_EVENT_SCREEN_LOAD_START)
  { // set icon to home icon or power off switch icon
    if(bPowerStatus)
    { lv_img_set_src(ui_Clock_Power_Off_Icon, &ui_img_home_png);
      lv_obj_set_style_img_recolor(ui_Clock_Power_Off_Icon, lv_color_hex(0x0098EC), LV_PART_MAIN | LV_STATE_DEFAULT); // blue as used on the watch face
      lv_obj_set_style_img_recolor_opa(ui_Clock_Power_Off_Icon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else 
    { lv_img_set_src(ui_Clock_Power_Off_Icon, &ui_img_power75x75_png); // icon on clock face
      lv_obj_set_style_img_recolor(ui_Clock_Power_Off_Icon, lv_color_make(0xBB, 0xBB, 0xBB), LV_PART_MAIN | LV_STATE_DEFAULT); // off-white
      lv_obj_set_style_img_recolor_opa(ui_Clock_Power_Off_Icon, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    }                                         

    lv_obj_update_layout(lv_event_get_target(e)); 
    // now screen can switch over

  }

  if(code == LV_EVENT_SCREEN_LOADED)
  { lv_refr_now(NULL); 
    while(BacklightValue < DEFAULT_BACKLIGHT) // gently fade up
    { Set_Backlight(++BacklightValue);    
      delay(15);
    } 
  }  
}


// decrement the fuzziness
void FuzzinessButtonL(lv_event_t * e)
{ char content[128];
  lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { if(DisplaySettings.expand_search>2)DisplaySettings.expand_search--;
    sprintf(content, "FUZZINESS %d", DisplaySettings.expand_search);
    lv_label_set_text(uic_SetFuzzinessValue, content);
  }
}

// increment the fuzziness
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

// yes you can touch the map with your big finger and try your luck
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
    // check if we are in our own home country
    if(strcmp(Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode, Stations.StationNUG[Stations.requested].countrycode)==0)lv_label_set_text(uic_GlobeCurrentCountry, "You Live Here");
    lv_obj_clear_flag(uic_HomeFlagToStore, LV_OBJ_FLAG_HIDDEN); // show country flag again
    lv_obj_clear_flag(uic_GlobeCurrentCountry, LV_OBJ_FLAG_HIDDEN); // show country name again
  }

  sprintf(content, "FUZZINESS %d", DisplaySettings.expand_search);
  lv_label_set_text(uic_SetFuzzinessValue, content);

  lv_scr_load(ui_SetupScreen);
}

// calibration button on setup screen
void CalibrationEnter(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    lv_scr_load(ui_CalibrationScreen);
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
    { // enter deep sleep to save battery, for shipping purposes
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
  { // Serial.println("SerialNumber clicked");
    
  }
  if(isLongPressed(e)==5) 
  { Serial.println("SerialNumber long pressed");
    beepforMs(1000);
    lv_scr_load(ui_Home);
    AddToQueueForGlobe("1", MESSAGE_UPDATE_GLOBE);
  }
}

// power button on main screen, only visible with volume at zero
void PowerCycle(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_CLICKED) // 7
  { Serial.println("Powerbutton Mainscreen clicked");
    if(bPowerStatus && !trigger_power_cycle_flag) // prevent double triggers with repeated clicks
    { Serial.println("trigger_power_cycle_flag activated");
      trigger_power_cycle_flag = true; // to be satisfied in main loop()
    }
  }
  if(isLongPressed(e)==2)
  { Serial.printf("Power button long pressed\n");
    if(bPowerStatus && !trigger_power_cycle_flag) // prevent double triggers with repeat
    { Serial.println("trigger_power_cycle_flag activated");
      trigger_power_cycle_flag = true; // to be satisfied in main loop()
    }
  }
}

void handlePowerCycle(void)
{ if(AutoSleepTimer)beepforMs(50); // don't beep if we arrive here by countdown of AutoSleepTimer, do it all silente
    
    if(bPowerStatus == true)
    { // Power off
      extern bool bUrlTestRunning;
      bUrlTestRunning = false;
      bPowerStatus = false;
      // clear power status bit in RTC ram
      PCF85063_Write_RTCRAM((PCF85063_Read_RTCRAM() & (RTC_RAM_POWERED_ON ^ 0xFF)));

      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_red);
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_red);
      lv_obj_invalidate(ui_Home_Power_Off_Icon);
      lv_refr_now(NULL); // quick update to see it go red
      
      if(AutoSleepTimer) AddToQueueForGlobe("OFF", MESSAGE_POWERDOWN); // initiated by user
      else AddToQueueForGlobe("OFF", MESSAGE_SILENT_POWER_DOWN); // intiated by countdown of AutoSleepTimer
      
      // power down, backlight down
      while(BacklightValue)
      { Set_Backlight(--BacklightValue);    
        delay(15);
      }
      //lv_scr_load(ui_ClockScreen);
      lv_scr_load_anim(ui_ClockScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_refr_now(NULL);
    }
    else
    { // Power on
      char content[128];
      extern bool bUrlTestRunning;
      bUrlTestRunning = false;
      sprintf(content, "Sleeptimer =% d -> %d by handlePowerCycle doing a power on\n", AutoSleepTimer, AUTOPOWERDOWNAFTER);
      AppendToLogFile("/Sleeptimer.log", content);
      AutoSleepTimer = AUTOPOWERDOWNAFTER;
      bPowerStatus = true;
      // set power status bit in RTC ram
      PCF85063_Write_RTCRAM(PCF85063_Read_RTCRAM() | RTC_RAM_POWERED_ON);

      ClockBackLight = true;
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
      ui_object_set_themeable_style_property(ui_Home_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
      lv_obj_invalidate(ui_Home_Power_Off_Icon);
      lv_obj_add_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN);
      lv_refr_now(NULL);

      AddToQueueForGlobe("ON", MESSAGE_POWERUP);
      delay(1000);
      lv_scr_load(ui_Home);
    }
}

void SaveVolTone(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { // tell globe to save settings
    AddToQueueForGlobe("SAVE VOLUME AND TONE CONTROLS", MESSAGE_STORE_VOLUME_AND_TONE);
    SaveDisplaySettings();
    lv_label_set_text(ui_ToneControlInstruction, "Audio Preferences Saved");
    lv_refr_now(NULL);
    beepforMs(1000);
    lv_label_set_text(ui_ToneControlInstruction, "Long Press Button To Save");
  }
}

// on Home screen, flag is pressed pressed
void StationInfo(lv_event_t * e)
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

// flags are stored on SD card as bin files, converted from png originals, using https://lvgl.io/tools/imageconverter LVGLv8, Color format -> CF_TRUE_COLOR_ALPHA Output format -> Binary RGB565 format
// immage descriptor and data for flag
lv_img_dsc_t my_flag_img;
uint8_t my_flag_img_data[96*72*3]; // 20736 bytes
char my_flag_countrycode[3];
lv_img_dsc_t my_home_flag_img;
uint8_t my_home_flag_img_data[96*72*3]; // 20736 bytes
char my_home_flag_countrycode[3];

void SetFlag(char *countrycode)
{ char lowercasecode[8];
  char path[32];
  lv_img_dsc_t * flag_img;
  uint8_t * flag_img_data;
  char * flag_countrycode;

  if(strlen(countrycode)>0)strcpy(lowercasecode, countrycode);
  else strcpy(lowercasecode, "xxxx"); // white blank flag
  
  if(strcmp(countrycode, "??")==0)strcpy(lowercasecode, "xxxx"); // white blank flag
  if(strcmp(countrycode, "XX")==0)strcpy(lowercasecode, "xx-sea"); // sea flag
  
  for(int i = 0; lowercasecode[i]; i++)
  { lowercasecode[i] = tolower(lowercasecode[i]);
  }


  // for home weet home or for world 
  if(strcmp(countrycode, Stations.StationNUG[(MAX_STATIONS+MAX_FAVORITES+MAX_HOMES-1)].countrycode) == 0)
  { flag_img = &my_home_flag_img;
    flag_img_data = &my_home_flag_img_data[0];
    flag_countrycode = &my_home_flag_countrycode[0];

    if(strcmp(flag_countrycode, countrycode) == 0) // already loaded
    { lv_img_set_src(uic_Home_Flag, flag_img);
      lv_img_set_src(uic_Database_Flag, flag_img);
      lv_img_set_src(uic_HomeFlagToStore, flag_img);
      lv_img_set_src(uic_ClockFlag, flag_img);
      lv_img_set_src(ui_PresetFlag, flag_img);
      Serial.printf("HOME FLAG %s SET FROM CACHE\n", countrycode );
      return;
    } 
  }
  else
  { flag_img = &my_flag_img;
    flag_img_data = &my_flag_img_data[0];
    flag_countrycode = &my_flag_countrycode[0];
    if(strcmp(flag_countrycode, countrycode) == 0) // already loaded
    { lv_img_set_src(uic_Home_Flag, flag_img);
      lv_img_set_src(uic_Database_Flag, flag_img);
      lv_img_set_src(uic_HomeFlagToStore, flag_img);
      lv_img_set_src(uic_ClockFlag, flag_img);
      lv_img_set_src(ui_PresetFlag, flag_img);
      Serial.printf("WORLD FLAG %s SET FROM CACHE\n", countrycode );
      return;
    }
  }

  strcpy(flag_countrycode, countrycode);
  sprintf(path, "/flags-bin/%s.bin", lowercasecode ); // flag file names are all lowercase
  Serial.printf("Setflag with %s\n", path);

  flag_img->header.always_zero = 0;
  flag_img->header.w = 96;
  flag_img->header.h = 72;
  flag_img->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
  flag_img->data = flag_img_data;

  if(Puck_SD_GB)
  { File fp = SD_MMC.open(path, FILE_READ);
    if(fp)
    { fp.read((uint8_t *)flag_img_data, 4); // header, basically skipping it, header is already filled above
      fp.read((uint8_t *)flag_img_data, sizeof(my_flag_img_data)); // pixel data
      fp.close();
      lv_img_set_src(uic_Home_Flag, flag_img);
      lv_img_set_src(uic_Database_Flag, flag_img);
      lv_img_set_src(uic_HomeFlagToStore, flag_img);
      lv_img_set_src(uic_ClockFlag, flag_img);
      lv_img_set_src(ui_PresetFlag, flag_img);
      //Serial.printf("Setflag loaded\n");
    }
    else
    { strcpy(flag_countrycode, ""); // mark as not loaded from SD
    }
  }  
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
{ char content64[64];
  
  if(World.gps_ns>9999)return;

  bClockHomeTime = !bClockHomeTime;
  beepforMs(50);

  // toggles between home and possibly other location on globe
  // todo - two structures for home/world, each aware of timezone, names and texts for the clock, for a faster switch between the two
  // that would save the time of doing an actual lookup every time
  if(bClockHomeTime)
  { Serial.println("ClockFlagToggle = HOME");
    // use home location as stored in favorites.txt as 5th record to get timezone and place
    DataFromDisplay.D_StationGpsNS = Home.gps_ns;
    DataFromDisplay.D_StationGpsEW = Home.gps_ew;
    sprintf(content64, "%d %f %f", 9999, DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW);
    //AddToQueueForGlobe("DISPLAY WANTS MQTT STATUS", MESSAGE_MQTT_STATUS); 
    if(strncmp(DisplaySettings.home_tz_posix, "CUSTOM", 5)!=0)AddToQueueForGlobe(content64, MESSAGE_HOME_TIMEZONE_NAME);  
    else AddToQueueForGlobe(content64, MESSAGE_HOME_TIMEZONE_NAME);  // todo -> change timezone system clock instead without requesting TZ everytime - is a brain breaker
    //lv_obj_add_flag(ui_MinuteHand, LV_OBJ_FLAG_HIDDEN); // idea -> hide hands until timezone is refreshed - needs a bool bShowHandsAgain upopn TZ reception etc - leave it for now
    //lv_obj_add_flag(ui_HourHand, LV_OBJ_FLAG_HIDDEN);

    // do it quick
    lv_label_set_text(ui_Clock_Country, GetAllUpperCase(content64, Home.CountryName));
    SetFlag(Home.CountryCode);
    lv_obj_clear_flag(ui_Home_Flag, LV_OBJ_FLAG_HIDDEN); // in case it was hidden by a search 
    sprintf(content64, "%s\n%s %d-%s-%d\n%s",  Home.TZname, weekdays[datetime.dotw%7], (size_t)datetime.day, monthnames[datetime.month%12],  (size_t)datetime.year%100, partofday[(datetime.hour/6)%4]);
    lv_label_set_text(ui_Time_Zone_Clock, content64); // on clock screen
    Serial.println("ClockFlagToggle = HOME & DONE");
    
  }
  else 
  { Serial.println("ClockFlagToggle = WORLD");
    DataFromDisplay.D_StationGpsNS = World.gps_ns;
    DataFromDisplay.D_StationGpsEW = World.gps_ew;
    sprintf(content64, "%d %f %f", 9999, DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW);
    //AddToQueueForGlobe("DISPLAY WANTS MQTT STATUS", MESSAGE_MQTT_STATUS); 
    AddToQueueForGlobe(content64, MESSAGE_GET_TIMEZONE_BY_GPS);  
    //lv_obj_add_flag(ui_MinuteHand, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_add_flag(ui_HourHand, LV_OBJ_FLAG_HIDDEN);

    // do it quick
    lv_label_set_text(ui_Clock_Country, GetAllUpperCase(content64, World.CountryName));
    SetFlag(World.CountryCode);
    lv_obj_clear_flag(ui_ClockFlag, LV_OBJ_FLAG_HIDDEN); // in case it was hidden by a search 
    sprintf(content64, "%s\n%s %d-%s-%d\n%s",  World.TZname, weekdays[datetime.dotw%7], (size_t)datetime.day, monthnames[datetime.month%12],  (size_t)datetime.year%100, partofday[(datetime.hour/6)%4]);
    lv_label_set_text(ui_Time_Zone_Clock, content64); // on clock screen
    Serial.println("ClockFlagToggle = WORLD & DONE");
  }

}

// Touching location/time on Home screen, switches over to clock
void SwitchToClock(lv_event_t * e)
{ beepforMs(50);
  //ClockHomeTime = false; // to sync the clocks Flag toggle state 
  lv_scr_load(ui_ClockScreen);
}

// clicking home or power icon on clock face
void ClockHomePower(lv_event_t * e)
{ beepforMs(50);

  if(bPowerStatus == true) // the icon is a home icon
  { Serial.printf("Clock Home/Power clicked while power is on\n");
    if(bClockHomeTime) // set back to world status
    { ClockFlagToggle(e);
    //  if(strcmp(ClockFlagCountryCode, Stations.StationNUG[Stations.requested].countrycode)!=0)
    //  { lv_obj_add_flag(ui_Home_Flag, LV_OBJ_FLAG_HIDDEN);
    //  }
    }
    lv_scr_load(ui_Home);
  }
  else // globe is in off mode
  { // Serial.printf("Clock Home/Power clicked while power is off\n");
    handlePowerCycle();
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

// code for weather icon and rh/temperature texts
// immage descriptor and data for weather icon
lv_img_dsc_t my_global_weather_img;
uint8_t my_global_weather_img_data[70*60*3]; // 12600 bytes

void SetWeatherData(char *settings)
{ char path[32];
  float temperature;
  uint16_t humidity;
  char icon[8];
  uint16_t apirequest;
  char content[32];
  
  // example, setting = "Temp 25.3 Rh 8 Icon 01d"

  sscanf(settings, "Temp %f Rh %hu Icon %3s %hu", &temperature, &humidity, &icon[0], apirequest);

  int temperaturerounded = (int)round(temperature);
  sprintf(content, "%d °C", temperaturerounded); 

  lv_label_set_text(uic_Weather_Temperature, content);
  sprintf(content, "Rh %d%%", humidity);
  lv_label_set_text(uic_Weather_Humidity, content);

  sprintf(path, "/weather-bin/%s-70x60.bin", icon);

  if(Puck_SD_GB)
  { File fp = SD_MMC.open(path, FILE_READ);
    if(fp)
    { my_global_weather_img.header.always_zero = 0;
      my_global_weather_img.header.w = 70;
      my_global_weather_img.header.h = 60;
      my_global_weather_img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
      my_global_weather_img.data = my_global_weather_img_data;
      
      fp.read((uint8_t *)my_global_weather_img_data, 4); // header, basically skipping it, header is already filled above
      fp.read((uint8_t *)my_global_weather_img_data, sizeof(my_global_weather_img_data)); // data
      fp.close();
      lv_img_set_src(uic_Weather_Icon, &my_global_weather_img);
    }
  }  
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

uint16_t ShowBatteryLevel(int request) // 0 -> hide, 1 -> show hide weather icon and show battery icons, negative -> only update value 
{ uint16_t currentbatteryvoltage;
  uint16_t actbatterypercentage;
  static int16_t battery_percentage;
  static int16_t old_battery_percentage = -1; // to initialise values
  char content[32];

  currentbatteryvoltage = ((analogReadMilliVolts(BAT_ADC_PIN) * 3) + 50) / 10; // read voltage in 10mV steps times 3 because of voltage divider 
  //Serial.printf("currentbatteryvoltage (200-400) = %d\n", currentbatteryvoltage);

  if(request == 0) // means hide battery
  { lv_obj_add_flag(uic_BatteryLevelHome, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN); 
    lv_event_send(uic_BatteryLevelHome, LV_EVENT_REFRESH, NULL);
    lv_event_send(uic_Battery_Icon_High, LV_EVENT_REFRESH, NULL);
    lv_event_send(uic_Battery_Icon_Low, LV_EVENT_REFRESH, NULL);
    lv_event_send(uic_Battery_Icon_Medium, LV_EVENT_REFRESH, NULL);
    return currentbatteryvoltage/10;
  }

  if((request<0) || (old_battery_percentage<0)) // update with new battery level
  { // map voltage to percentage 
    if(currentbatteryvoltage>=410)actbatterypercentage = 100;
    else if(currentbatteryvoltage>=406)actbatterypercentage = map(currentbatteryvoltage, 406, 409, 90, 100);  //  90; 
    else if(currentbatteryvoltage>=398)actbatterypercentage = map(currentbatteryvoltage, 398, 405, 80, 89);  //  80; 
    else if(currentbatteryvoltage>=392)actbatterypercentage = map(currentbatteryvoltage, 392, 397, 70, 79);  //  70; 
    else if(currentbatteryvoltage>=387)actbatterypercentage = map(currentbatteryvoltage, 387, 391, 60, 69);  //  60; 
    else if(currentbatteryvoltage>=375)actbatterypercentage = map(currentbatteryvoltage, 375, 386, 50, 59);  //  50; 
    else if(currentbatteryvoltage>=367)actbatterypercentage = map(currentbatteryvoltage, 367, 374, 40, 49);  //  40; 
    else if(currentbatteryvoltage>=360)actbatterypercentage = map(currentbatteryvoltage, 360, 366, 30, 39);  //  30; 
    else if(currentbatteryvoltage>=345)actbatterypercentage = map(currentbatteryvoltage, 345, 359, 20, 29);  //  20; 
    else if(currentbatteryvoltage>=330)actbatterypercentage = map(currentbatteryvoltage, 330, 344, 10, 19);  //  10;
    else if(currentbatteryvoltage>=300)actbatterypercentage = map(currentbatteryvoltage, 300, 329, 0, 9);  //  1;
    else actbatterypercentage = 0;

    // track percentage, one step up/down at the time
    if(old_battery_percentage<0)battery_percentage = actbatterypercentage;
    if(battery_percentage < actbatterypercentage)battery_percentage++;
    if(battery_percentage > actbatterypercentage)battery_percentage--;

    if(old_battery_percentage != battery_percentage)
    { old_battery_percentage = battery_percentage;
      sprintf(content, "%d%%", battery_percentage);
      lv_label_set_text(ui_BatteryLevelHome, content);
      lv_label_set_text(ui_BatteryLevel, content);
    }
    if(request<0)return currentbatteryvoltage/10; // was just a periodic battery update request
  }

  // show battery icon with level
  ShowWeatherData(false); // hide all weather data, icon and texts

  sprintf(content, "%d%%", battery_percentage);
  lv_label_set_text(ui_BatteryLevelHome, content);
  lv_label_set_text(ui_BatteryLevel, content);


  lv_obj_clear_flag(uic_BatteryLevelHome, LV_OBJ_FLAG_HIDDEN); 
    
  if(battery_percentage > 90)
  { lv_obj_add_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
  }
  else if(battery_percentage > 50)
  { lv_obj_add_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
  }
  else
  { lv_obj_clear_flag(ui_Battery_Icon_Low, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_Battery_Icon_Medium, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_Battery_Icon_High, LV_OBJ_FLAG_HIDDEN);
  } 
  return currentbatteryvoltage/10;
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

// same as lv_label_set_text() but do it in all uppercase
void lv_label_set_text_uppercase(lv_obj_t * label, char *text)
{ char content[128];
  GetAllUpperCase(content, text);
  lv_label_set_text(label, content);
}



