// favorites
// Select or store a station
// 
// list is loaded from favorites.txt
// and also stored back into favorites.txt
#include "FS.h"
#include "SD_MMC.h"
#include "database.h"
#include "favorites.h"

void station1234(lv_event_t * e, uint16_t station)
{ static uint16_t longpressed;
  lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { //Serial.printf("hey-favorites-Station->event=%s %d\n", "LV_EVENT_LONG_PRESSED, longpressed");
    longpressed++;
    if(longpressed == 5)
    { if(Stations.playing >=0)
      { // all in one array
        strcpy(Stations.StationNUG[station+MAX_STATIONS].name, Stations.StationNUG[Stations.playing].name);
        strcpy(Stations.StationNUG[station+MAX_STATIONS].url, Stations.StationNUG[Stations.playing].url);
        Stations.StationNUG[station+MAX_STATIONS].gps_ns = Stations.StationNUG[Stations.playing].gps_ns;
        Stations.StationNUG[station+MAX_STATIONS].gps_ew = Stations.StationNUG[Stations.playing].gps_ew;
        strcpy(Stations.StationNUG[station+MAX_STATIONS].town, Stations.StationNUG[Stations.playing].town);
        strcpy(Stations.StationNUG[station+MAX_STATIONS].countrycode, Stations.StationNUG[Stations.playing].countrycode);
        strcpy(Stations.StationNUG[station+MAX_STATIONS].countryname, Stations.StationNUG[Stations.playing].countryname);
        
        if(station==0)
        { lv_label_set_text(ui_StationPresetName1, Stations.StationNUG[station+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_GREEN);
        }  
        if(station==1)
        { lv_label_set_text(ui_StationPresetName2, Stations.StationNUG[station+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_GREEN);
        }  
        if(station==2)
        { lv_label_set_text(ui_StationPresetName3, Stations.StationNUG[station+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_GREEN);
        }  
        if(station==3)
        { lv_label_set_text(ui_StationPresetName4, Stations.StationNUG[station+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_GREEN);
        }  

        for(int16_t n=0; n<4; n++)
        { if(n!=station)SetLed(n, 0);
        }          
        Serial.printf("Favorites STORE station %s under Preset %d\n", Stations.StationNUG[station+MAX_STATIONS].name, station);
        SaveFavorites();
        beepforMs(1000);
      }
    }  
  }

  if(event_code == LV_EVENT_CLICKED) 
  { if(longpressed==0)
    { if(strlen(Stations.StationNUG[station+MAX_STATIONS].url)>0) // only respond to loaded buttons
      { Serial.printf("Favorites Select station %s under Preset %d\n", Stations.StationNUG[station+MAX_STATIONS].name, station);
        AddStationToQueueForGlobe(station+MAX_STATIONS); // presets come just after the regular list of stations
        lv_label_set_text(ui_Station_Name, Stations.StationNUG[station+MAX_STATIONS].name);
        if(station==0)
        { lv_label_set_text(ui_StationPresetName1, Stations.StationNUG[0+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_RED);
        }
        if(station==1)
        { lv_label_set_text(ui_StationPresetName2, Stations.StationNUG[1+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_RED);
        }
        if(station==2)
        { lv_label_set_text(ui_StationPresetName3, Stations.StationNUG[2+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_RED);
        }
        if(station==3)
        { lv_label_set_text(ui_StationPresetName4, Stations.StationNUG[3+MAX_STATIONS].name); 
          SetLed(station, UI_THEME_COLOR_RED);
        }
        for(int16_t n=0; n<MAX_FAVORITES; n++)
        { if(n!=station)SetLed(n, 0);
        }          
        beep(e);
      }  
    }
    longpressed = 0;
  }
}

void station1(lv_event_t * e)
{  station1234(e, 0);
}

void station2(lv_event_t * e)
{  station1234(e, 1);
}

void station3(lv_event_t * e)
{  station1234(e, 2);
}

void station4(lv_event_t * e)
{  station1234(e, 3);
}

// just save the favorites to file
void SaveFavorites(void)
{ File favfile;
  char favfilename[] = {"/favorites.txt"};
  char *p;
  char sometext[128];

  if (!SD_MMC.begin("/sdcard", true, false))return; // no card

  SD_MMC.remove(favfilename); // delete old file
  favfile = SD_MMC.open(favfilename, FILE_WRITE);
  if(favfile)
  { for(int16_t n = 0; n<MAX_FAVORITES; n++)
    { if((p=strchr(Stations.StationNUG[n+MAX_STATIONS].name, '\n'))!=NULL)*p=0;
      if((p=strchr(Stations.StationNUG[n+MAX_STATIONS].url, '\n'))!=NULL)*p=0;
      if((p=strchr(Stations.StationNUG[n+MAX_STATIONS].name, '\r'))!=NULL)*p=0;
      if((p=strchr(Stations.StationNUG[n+MAX_STATIONS].url, '\r'))!=NULL)*p=0;
      sprintf(sometext, "\"name\": \"%s\"", Stations.StationNUG[n+MAX_STATIONS].name);
      favfile.println(sometext);  
      sprintf(sometext, "\"url\": \"%s\"", Stations.StationNUG[n+MAX_STATIONS].url);
      favfile.println(sometext);  
      sprintf(sometext, "\"gps\": \"%f,%f\"", Stations.StationNUG[n+MAX_STATIONS].gps_ns, Stations.StationNUG[n+MAX_STATIONS].gps_ew); // write gps to file
      favfile.println(sometext);  
      sprintf(sometext, "\"town\": \"%s\"", Stations.StationNUG[n+MAX_STATIONS].town);
      favfile.println(sometext);  
      sprintf(sometext, "\"countrycode\": \"%s\"", Stations.StationNUG[n+MAX_STATIONS].countrycode);
      favfile.println(sometext);  
      sprintf(sometext, "\"countryname\": \"%s\"", Stations.StationNUG[n+MAX_STATIONS].countryname);
      favfile.println(sometext);  
    }  
  }
  favfile.close();
  SD_MMC.end();
}

void LoadFavorites(void)
{ File favfile;
  char favfilename[] = {"/favorites.txt"};
  char *p;
  char text[140];  
  int16_t favcount;

  if (!SD_MMC.begin("/sdcard", true, false))return; // no card

  favfile = SD_MMC.open(favfilename, FILE_READ);
  if(favfile)
  { favcount = -1;
    while(favfile.available()) 
    { String data = favfile.readStringUntil('\n');
      strcpy(text, data.c_str());
      if((p=strchr(text, '\n'))!=NULL)*p=0;
      if((p=strchr(text, '\r'))!=NULL)*p=0;
      if((p = strrchr(text, '\"')) != NULL)*p=0; // get rid of the last "
      if((p = strstr(text, "\"name\": ")) != NULL) 
      { favcount++;
        p+=9; // jump forward to start of name
        strncpy(Stations.StationNUG[favcount+MAX_STATIONS].name, p, sizeof(Stations.StationNUG[0].name));
        Stations.StationNUG[favcount+MAX_STATIONS].name[sizeof(Stations.StationNUG[0].name)-1]=0;
      }

      //Serial.printf("Preset % d text = <%s>\n", favcount, text);
      
      if(favcount>=0 && favcount<MAX_FAVORITES) // max 4 presets
      {
        if((p = strstr(text, "\"url\": ")) != NULL) 
        { p+=8; // jump forward to start of name
          strcpy(Stations.StationNUG[favcount+MAX_STATIONS].url, p);
        }
        if((p = strstr(text, "\"gps\": ")) != NULL) 
        { p+=8; // jump forward to start of name
          sscanf(p, "%f,%f", &Stations.StationNUG[favcount+MAX_STATIONS].gps_ns, &Stations.StationNUG[favcount+MAX_STATIONS].gps_ew);
        }
        if((p = strstr(text, "\"town\": ")) != NULL) 
        { p+=9; // jump forward to start of name
          strcpy(Stations.StationNUG[favcount+MAX_STATIONS].town, p);
        }
        if((p = strstr(text, "\"countrycode\": ")) != NULL) 
        { p+=16; // jump forward to start of name
          strncpy(Stations.StationNUG[favcount+MAX_STATIONS].countrycode, p, 2);
          Stations.StationNUG[favcount+MAX_STATIONS].countrycode[3]=0;
        }
        if((p = strstr(text, "\"countryname\": ")) != NULL) 
        { p+=16; // jump forward to start of name
          strncpy(Stations.StationNUG[favcount+MAX_STATIONS].countryname, p, 49);
          Stations.StationNUG[favcount+MAX_STATIONS].countryname[49]=0;
        }
      }
    }
    for(int16_t station = 0; station<MAX_FAVORITES; station++)
    { //Serial.printf("Loaded Station %d = %s\n", station, Stations.StationNUG[station+MAX_STATIONS].name);
      if(station==0)lv_label_set_text(ui_StationPresetName1, Stations.StationNUG[0+MAX_STATIONS].name); 
      if(station==1)lv_label_set_text(ui_StationPresetName2, Stations.StationNUG[1+MAX_STATIONS].name); 
      if(station==2)lv_label_set_text(ui_StationPresetName3, Stations.StationNUG[2+MAX_STATIONS].name); 
      if(station==3)lv_label_set_text(ui_StationPresetName4, Stations.StationNUG[3+MAX_STATIONS].name); 
    }  
  }
  // turn of preset leds
  SetLed(1, 0);
  SetLed(2, 0);
  SetLed(3, 0);
  SetLed(4, 0);
  Lvgl_Loop();
  favfile.close();
  SD_MMC.end();
  Serial.println("Finished loading favorites/presets from SD");
}



void SetLed(int16_t preset, int16_t themecolor)
{ lv_obj_t *led;
  
  //Serial.printf("preset = %d themecolor = %d\n", preset, themecolor);
  if(preset == 0)led = uic_led1; 
  else if(preset == 1)led = uic_led2; 
  else if(preset == 2)led = uic_led3; 
  else if(preset == 3)led = uic_led4; 
  else return;

  if(themecolor == 0)
  { lv_obj_clear_flag(led, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(led, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  lv_obj_clear_flag(led, LV_OBJ_FLAG_HIDDEN);
  

  //Serial.printf("do preset = %d themecolor = %d\n", preset, themecolor);
  
  if(themecolor == UI_THEME_COLOR_RED)
  { ui_object_set_themeable_style_property(led, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_red);
    ui_object_set_themeable_style_property(led, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_red);
  }
  if(themecolor == UI_THEME_COLOR_GREEN)
  { ui_object_set_themeable_style_property(led, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_green);
    ui_object_set_themeable_style_property(led, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_green);
  }  

}




