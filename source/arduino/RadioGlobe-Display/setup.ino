// handling of events on Setup Screen, beeps and long button presses




// cog wheel on Home Screen
void SetupEnter(lv_event_t * e)
{ if(strcmp(SecretCode, "GLOBE") != 0)
  { lv_scr_load(ui_PasswordScreen);
    return;
  }
  // activate Setup Screen
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
    lv_obj_clear_flag(uic_MapBanner, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clear_flag(uic_Database_Dir_Path, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_flag(uic_Database_GPS_Position, LV_OBJ_FLAG_HIDDEN); 


    lv_label_set_text(ui_Database_Progress, "");
    lv_label_set_text(ui_Database_Dir_Path, "");
    lv_label_set_text(ui_Database_Output_File, "Long Press Button To Start");
    lv_obj_add_state(uic_MapBanner, LV_STATE_DISABLED); 
    lv_obj_add_state(uic_MapCursor, LV_STATE_DISABLED); 
    
    lv_scr_load(ui_DatabaseScreen);
    Lvgl_Loop();  
    strcpy(SecretCode,"12345"); // have to enter again to be sure
//    bCheckDatabase = true; // starts automatically 
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

// returns higher value when longer pressed
uint16_t isLongPressed(lv_event_t * e)
{ static uint16_t longpressed=0;
  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { longpressed++;
  }
  if(event_code == LV_EVENT_CLICKED) 
  { longpressed = 0;
  }
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
{ strncpy(SecretCode, &SecretCode[1], 5); 
  SecretCode[4] = 'O';
  Serial.println(SecretCode);
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

void PowerCycle(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    Serial.printf("Power button long pressed\n");
    if(bPowerStatus == true)
    { // Power off
      bPowerStatus = false;
      ui_object_set_themeable_style_property(uic_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_red);
      ui_object_set_themeable_style_property(uic_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_red);
      lv_obj_invalidate(uic_Power_Off_Icon);
      ui_object_set_themeable_style_property(uic_Big_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_red);
      ui_object_set_themeable_style_property(uic_Big_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_red);
      lv_obj_invalidate(uic_Big_Power_Off_Icon);
      lv_refr_now(NULL);
      Lvgl_Loop();  

      
      AddToQueueForGlobe("OFF", MESSAGE_POWERDOWN);
      // power down, backlight down
      while(BacklightValue)
      { Set_Backlight(--BacklightValue);    
        delay(15);
      }
      delay(100);
      lv_scr_load(ui_Power);
    }
    else
    { // Power om
      bPowerStatus = true;
      ui_object_set_themeable_style_property(uic_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
      ui_object_set_themeable_style_property(uic_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
      lv_obj_invalidate(uic_Power_Off_Icon);
      ui_object_set_themeable_style_property(uic_Big_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
      ui_object_set_themeable_style_property(uic_Big_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
      lv_obj_invalidate(uic_Big_Power_Off_Icon);
      lv_obj_add_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN);
      lv_refr_now(NULL);
      Lvgl_Loop();  

      AddToQueueForGlobe("ON", MESSAGE_POWERUP);
      delay(1000);
      lv_scr_load(ui_Home);
    }
  }

//    lv_label_set_text(ui_DatabaseProgress, "");
//    lv_label_set_text(ui_DatabaseProgress1, "Long Press Button To Start");
//    lv_obj_add_state(uic_MapBanner, LV_STATE_DISABLED); 
//    lv_obj_add_state(uic_MapCursor, LV_STATE_DISABLED); 
    
//    lv_scr_load(ui_Power);
    Lvgl_Loop();  
//    bCheckDatabase = true; // starts automatically 
}

void SaveVolTone(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { // tell globe to save
    AddToQueueForGlobe("SAVE VOLUME AND TONE CONTROLS", MESSAGE_STORE_VOLUME_AND_TONE);
    beepforMs(1000);
  }
}



// on Home screen, 'now playing' pressed
void StationInfo(lv_event_t * e)
{ char content[256];
  if(isLongPressed(e)==5)
  { // use database screen to show where station is located on worldmap with gps coordinates
    beepforMs(1000);

  }
  else
  { beepforMs(100);
//    if(Stations.playing>=0)
    { Serial.printf("Station %d Info clicked from Home screen\n", Stations.playing);
      lv_obj_add_flag(uic_RebuildDatabase, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(uic_RebuildDatabaseButtonText, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(uic_MapBanner, LV_OBJ_FLAG_HIDDEN); 
      lv_obj_add_flag(uic_Database_Dir_Path, LV_OBJ_FLAG_HIDDEN); 
      lv_obj_clear_flag(uic_Database_GPS_Position, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(uic_Database_Town_Name, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_state(uic_MapCursor, LV_STATE_DISABLED); 
      lv_obj_clear_flag(uic_Database_Flag, LV_OBJ_FLAG_HIDDEN);
      lv_scr_load(ui_DatabaseScreen);


// size of squareline flag is 20736 (hex 5100), 3 bytes per pixel for a size of 96x72
// that is  20740 bytes, plus 4 for header
// LV_IMG_PX_SIZE_ALPHA_BYTE -> one pixel uses 3 bytes



      if(Stations.playing<(MAX_STATIONS+MAX_FAVORITES))
      { //Serial.printf("Countrycode = %s\n", Stations.StationNUG[Stations.playing].countrycode);
        //FindCountryNameByCode(Stations.StationNUG[Stations.playing].countryname, Stations.StationNUG[Stations.playing].countrycode);
        sprintf(content,"Greetings From  %s", Stations.StationNUG[Stations.playing].countryname);
        lv_label_set_text(ui_Database_Town_Name, content);
        lv_label_set_text(ui_Database_Progress, Stations.StationNUG[Stations.playing].name);  
        lv_obj_set_pos(uic_MapCursor, (int)Stations.StationNUG[Stations.playing].gps_ew, -(int)Stations.StationNUG[Stations.playing].gps_ns);
        sprintf(content, "GPS NS %2.6f - EW %3.6f", Stations.StationNUG[Stations.playing].gps_ns, Stations.StationNUG[Stations.playing].gps_ew);
        lv_label_set_text(ui_Database_GPS_Position, content);
        sprintf(content,"You Are In  %s", Stations.StationNUG[Stations.playing].town);
        lv_label_set_text(ui_Database_Output_File, content);
        ShowTheStations();
      }
      lv_scr_load(ui_DatabaseScreen);
      bInfoScreen = true;
    }
  }
}


// immage descriptor and data 
lv_img_dsc_t my_global_img;
uint8_t my_global_img_data[96*72*3]; // 20736 bytes

void ShowFlag(char *countrycode)
{ char lowercasecode[8];
  char path[32];
  strcpy(lowercasecode, countrycode);
  for(int i = 0; lowercasecode[i]; i++)
  { lowercasecode[i] = tolower(lowercasecode[i]);
  }
  
  sprintf(path, "/flags-bin/%s.bin", lowercasecode );

  my_global_img.header.always_zero = 0;
  my_global_img.header.w = 96;
  my_global_img.header.h = 72;
  my_global_img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
  my_global_img.data = my_global_img_data;

  SD_MMC.begin("/sdcard", true, false);
  File fp = SD_MMC.open(path, FILE_READ);
  fp.read((uint8_t *)my_global_img_data, 4); // header, basically skipping it, header is already filled above
  fp.read((uint8_t *)my_global_img_data, sizeof(my_global_img_data)); // header
  fp.close();
  SD_MMC.end();

  lv_img_set_src(uic_Database_Flag, &my_global_img);
  lv_img_set_src(uic_Home_Flag, &my_global_img);

}


static lv_img_dsc_t StationMap_img_dsc;

void ShowTheStations(void)
{ // this took me a whole day
  StationMap_img_dsc.header.always_zero = 0;
  StationMap_img_dsc.header.w = 384;
  StationMap_img_dsc.header.h = 180;
  StationMap_img_dsc.header.cf = LV_IMG_CF_ALPHA_1BIT; 
  StationMap_img_dsc.data = StationsMap.pixeldata;
  // lv_img_set_src(uic_WorldMap, &StationMap_img_dsc);
  // lv_obj_t * canvas = lv_canvas_create(lv_scr_act());
  lv_obj_t * canvas = lv_canvas_create(ui_WorldMap);
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_img_recolor(&style, lv_color_hex(0xff0000));
  lv_style_set_img_recolor_opa(&style, LV_OPA_100); 
  lv_obj_add_style(canvas, &style, LV_PART_MAIN | LV_STATE_DEFAULT); 
  lv_canvas_set_buffer(canvas, StationsMap.pixeldata, 384, 180, LV_IMG_CF_ALPHA_1BIT);
  // wow
}
