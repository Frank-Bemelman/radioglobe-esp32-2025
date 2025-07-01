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

