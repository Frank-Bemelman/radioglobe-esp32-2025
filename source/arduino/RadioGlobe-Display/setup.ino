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
    lv_label_set_text(ui_DatabaseProgress, "");
    lv_label_set_text(ui_DatabaseProgress1, "Long Press Button To Start");
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
      while(backlightvalue)
      { Set_Backlight(--backlightvalue);    
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
    if(Stations.playing>=0)
    { Serial.printf("Station Info clicked from Home screen\n");
      lv_obj_clear_state(uic_MapBanner, LV_STATE_DISABLED); 
      lv_obj_add_flag(uic_RebuildDatabase, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(uic_RebuildDatabaseButtonText, LV_OBJ_FLAG_HIDDEN);


      if(Stations.playing<MAX_STATIONS)
      { sprintf(content, "GPS NS %2.6f - EW %3.6f", Stations.StationNUG[Stations.playing].gps_ns, Stations.StationNUG[Stations.playing].gps_ew);
        lv_label_set_text(ui_DatabaseProgress, Stations.StationNUG[Stations.playing].name);  
        lv_obj_set_pos(uic_MapCursor, (int)Stations.StationNUG[Stations.playing].gps_ew, -(int)Stations.StationNUG[Stations.playing].gps_ns);
        sprintf(content,"The URL Of This Station Is\n%s\n", Stations.StationNUG[Stations.playing].url);
        lv_label_set_text(ui_MapBanner, content);

      }
      else
      { sprintf(content, "GPS NS %2.6f - EW %3.6f", Favorites[Stations.playing-MAX_STATIONS].gps_ns, Favorites[Stations.playing-MAX_STATIONS].gps_ew);
        lv_label_set_text(ui_DatabaseProgress, Favorites[Stations.playing-MAX_STATIONS].name);  
        lv_obj_set_pos(uic_MapCursor, (int)Favorites[Stations.playing-MAX_STATIONS].gps_ew, -(int)Favorites[Stations.playing-MAX_STATIONS].gps_ns);
        sprintf(content,"The URL Of This Preset Station Is\n%s\n", Stations.StationNUG[Stations.playing-MAX_STATIONS].url);
        lv_label_set_text(ui_MapBanner, content);
      }
      lv_label_set_text(ui_DatabaseProgress1, content);


      lv_scr_load(ui_DatabaseScreen);
    }
  }
}


