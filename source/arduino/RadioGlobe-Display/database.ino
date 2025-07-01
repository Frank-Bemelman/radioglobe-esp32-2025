// functions for database of radio stations
// makes use of stations.json file stored on SD card

#include "FS.h"
#include "SD_MMC.h"
#include "database.h"

bmpfile StationsMap;  // to maintain a quick lookup map of available directories     
bmpfile NauticMap;    // to have a map of water vs land that have actual timezones according to google
stations_arraybin Stations;

char rolldata[(64+2)*MAX_STATIONS];


void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}



// REBUILD DATABASE BUTTON
void RebuildDatabase(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    Serial.printf("REBUILD DATABASE BUTTON\n");
    bCheckDatabase = true;
  }
}

// create a directory structure per NS degree and EW degree
// in theory 64800 files to store
// ./N/N<digit/10>/<digit%10>/E<digit/10>/<digit%10>
// ./N/N<digit/10>/<digit%10>/W<digit/10>/<digit%10>
// ./S/S<digit/10>/<digit%10>/E<digit/10>/<digit%10>
// ./S/S<digit/10>/<digit%10>/W<digit/10>/<digit%10>
// each directory containing filenames like N24W25urls.txt containing list of stations for that coordinate

void BuildDatabaseNow(void)
{ uint16_t timeout = 5;
  char content[128];

  Serial.printf("BuildDatabaseNow()\n");

  lv_obj_add_state(uic_RebuildDatabase, LV_STATE_DISABLED);
  lv_obj_add_flag(uic_HomeButton3, LV_OBJ_FLAG_HIDDEN);
  

  SD_MMC.end();
  if (!SD_MMC.begin("/sdcard", true, false))
  { lv_label_set_text(ui_DatabaseProgress1, "No SD Card Found!");
    Lvgl_Loop();
    delay(1000);
    while(timeout)
    {  delay(1000);
       sprintf(content, "leaving in %d seconds", timeout);
       lv_label_set_text(ui_DatabaseProgress1 , content);
       Lvgl_Loop();
       timeout --;
    }
    delay(1000);
    lv_scr_load(ui_Home); 
    lv_label_set_text(ui_DatabaseProgress1, "");
  }
  else
  { lv_label_set_text(ui_DatabaseProgress1, "SD Card Found!");
    Lvgl_Loop();
    delay(1000);
  
    //ReadDatabase("/stations.json"); // don't fortget the forward slash
    //ReadDatabase("/stations1K.json"); // don't fortget the forward slash

    //ReadDatabase("/stations20K.json"); // don't fortget the forward slash
  
    ReadDatabase("/stations150K.json"); // don't fortget the forward slash
    //listDir(SD_MMC, "/", 10);

    Serial.println("Einde verhaal database maken 2");

  }
  Serial.println("Einde verhaal database maken 3");
  SD_MMC.end();
  bCheckDatabase = false;

}

void ReadDatabase(char *filename)
{ char sometext[128];
  char oneline[128];
  char outputfilename[64];
  char dirpath[64];
  char *p;
  size_t bytesread;
  uint32_t filesize = 0;
  uint32_t fileposition = 0;
  uint16_t percentagedone;
  uint32_t startMillis = millis();  
  uint32_t currentMillis;
  uint16_t parsefase = 0;
  uint32_t urlcount = 0;
  float floatn;
  float floate;
  int16_t ns=0;
  int16_t ew=0;
  File urls;

  File root = SD_MMC.open(filename, FILE_READ);
  if (!root) 
  { sprintf(sometext, "Failed To Open %s", filename);
    lv_label_set_text(ui_DatabaseProgress1, sometext);
    Lvgl_Loop();
    Serial.println(sometext);
    delay(2000); 
    return;
  }
  sprintf(sometext, "Succes opening %s", filename);
  lv_label_set_text(ui_DatabaseProgress1, sometext);
  Lvgl_Loop();
  delay(2000); 
  
  filesize = root.size();
  Serial.printf("filesize of %s is %ld\n", filename, filesize);

  bool readcoords = false;
  uint16_t citycount = 0;

  lv_obj_set_pos(uic_MapCursor, ew, -ns);
  // Start the animation on the map
  lv_obj_clear_state(uic_MapBanner, LV_STATE_DISABLED);
  lv_obj_clear_state(uic_MapCursor, LV_STATE_DISABLED);



  while(root.available())//  && urlcount < 500)
  { currentMillis = millis();
    if (currentMillis - startMillis >= 100)  // every 5S screen update
    { startMillis = currentMillis;
      fileposition = root.position();
      percentagedone = fileposition * 100 / filesize;
      sprintf(sometext, "Done %d%% urls=%ld", percentagedone, urlcount);
      lv_label_set_text(ui_DatabaseProgress, sometext);
      Lvgl_Loop();
    }

    bytesread = root.readBytesUntil(0x0a, oneline, sizeof(oneline)-1);
    oneline[bytesread]=0;
    //Serial.printf("bytesread= %d lineread = %s\n", bytesread, oneline);
    switch(parsefase)
    { case 0:
        if((p = strstr(oneline, "\": {")) != NULL) // city,country
        { // example
          //    "Winston-Salem,US-NC": {
          // results in: Winston-Salem_US-NC.txt  
          if((p=strchr(oneline, '\"'))!=NULL)
          { strcpy(outputfilename, p+1);
            if((p=strchr(outputfilename, '\"')) != NULL)*p=0;
            if((p=strchr(outputfilename, ',')) != NULL)*p='_';
            strcat(outputfilename, ".txt");
            Serial.println(outputfilename);
          }
          else strcpy(outputfilename, "noname.txt");
          parsefase = 1;
        }
        break;
      case 1:
        if((p = strstr(oneline, "\"n\": ")) != NULL)
        { //Serial.println(p);
          sscanf(&p[5], "%f", &floatn);
          if(floatn>0)floatn+=0.5;
          else floatn-=0.5;
          ns = (int)floatn;
          parsefase = 2;
        }        
        break;
      case 2:
        if((p = strstr(oneline, "\"e\": ")) != NULL)
        { //Serial.println(p);
          sscanf(&p[5], "%f", &floate);
          if(floate>0)floate+=0.5;
          else floate-=0.5;
          ew = (int)floate;
          sprintf(dirpath, "/%c/%d/%d/%c/%d/%d", (ns<0)?'S':'N', abs(ns)/10, abs(ns)%10, (ew<0)?'W':'E', abs(ew)/10, abs(ew)%10);
          Serial.println(dirpath);
          CreateAllDirInPath(SD_MMC, dirpath);
          SetPixelInMap(ns, ew);
          lv_obj_set_pos(uic_MapCursor, ew, -ns);
          parsefase = 3;
        }
        break;
      case 3:  
        if((p = strstr(oneline, "\"urls\": [")) != NULL) // start of list
        { sprintf(sometext, "%s/%s", dirpath, outputfilename);
          lv_label_set_text(ui_DatabaseProgress1, sometext);
          SD_MMC.remove(sometext); // delete old file
          urls = SD_MMC.open(sometext, FILE_WRITE);
          parsefase = 4;
        }
        break;
      case 4:  
        if((p = strstr(oneline, "\"name\": ")) != NULL) // name found
        { //Serial.println(p);
          urls.println(p);  // write name to file 
          parsefase = 5;
        }
        if((p = strstr(oneline, "]")) != NULL) // end of file reached
        { //Serial.println(p);
          urls.close();
          parsefase = 0;
        }
        break;
      case 5:  
        if((p = strstr(oneline, "\"url\": ")) != NULL) // url found
        { //Serial.println(p);
          urls.println(p);  // write to file 
          parsefase = 6;
        }
        break;
      case 6:  
        sprintf(sometext, "\"gps\": \"%f,%f\"", floatn, floate); // write gps to file
        urls.println(sometext);  
        urlcount++;
        parsefase = 4;
        break;

      default:
        break;  
    }
  }  

  lv_obj_add_state(uic_MapBanner, LV_STATE_DISABLED); 
  lv_obj_add_state(uic_MapCursor, LV_STATE_DISABLED); 

  root.close();
  
  // long filename test, and yes it works
  //urls = SD_MMC.open("/wateenleukefile.txt", FILE_WRITE);
  //urls.close();

  percentagedone = 100;
  sprintf(sometext, "Done %d%% urls=%ld", percentagedone, urlcount);
  lv_label_set_text(ui_DatabaseProgress, sometext);
  sprintf(sometext, "- %d Files Created -", urlcount);
  lv_label_set_text(ui_DatabaseProgress1, sometext);
  Lvgl_Loop();

  if((p=strchr(filename, '.')) != NULL)
  { *p=0;
    strcat(filename, ".bmp");
  }

  File bitmap = SD_MMC.open("/stationsmap.bmp", FILE_WRITE);
  if(bitmap) 
  { char *p = (char*) &StationsMap;
    p+=2; // skip filler in structure
    Serial.printf("StationsMap-2 size = %ld\n", sizeof(StationsMap)-2);
    Serial.printf("StationsMap.bmpheader size = %ld\n", sizeof(StationsMap.bmpheader));
    Serial.printf("StationsMap.bmpct size     = %ld\n", sizeof(StationsMap.bmpct));
    Serial.printf("StationsMap.pixeldata size = %ld\n", sizeof(StationsMap.pixeldata));

    int n = 54+8;
    // write header
    while(n)
    { bitmap.write(*p++);
      n--;
    }

   // write file, bottom up
   int w;
   for(n=179;n>=0;n--)
   { p = (char*)&StationsMap.pixeldata[(n*48)];
     for(w=0;w<48;w++)
     { bitmap.write(*p++);
     }
   }
    bitmap.close();
  }       

  //tellme();

  lv_obj_clear_state(uic_RebuildDatabase, LV_STATE_DISABLED);
  lv_obj_clear_flag(uic_HomeButton3, LV_OBJ_FLAG_HIDDEN);
  
  Serial.println("Database rebuild completed!");
}



void CreateAllDirInPath(fs::FS &fs, char *path) 
{ char pathcopy[64];
  char *s;
  char *d;
  File root;
  s = path;
  d = pathcopy;

  

  //Serial.printf("Create these directories %s\n", path);
  *d++ = *s++; // copy first '/'
  while(*s)
  { while(*s)
    { *d++ = *s++; // copy intil '/' found
      if(*s == '/')
      { break;
      }
    }
    *d = 0; // terminate string

    //Serial.printf("Full path %s\n", path);
    //Serial.printf("Try to open directory %s!\n", pathcopy);
    root = fs.open(pathcopy);
    if(!root) 
    { Serial.printf("Failed to open directory %s so make it!\n", pathcopy);
      if(SD_MMC.mkdir(pathcopy))
      { Serial.printf("%s created\n", pathcopy);
      }
      else Serial.printf("%s creation error\n", pathcopy);
    }
    *d = *s; // *s is '/' in case more directories todo
    // Serial.printf("*s = %d\n", (int)*s);
  }
}      



void FindNewStation(void)
{ bool dirscan = true;
  bool dirfound = false;
  int16_t ns;
  int16_t ew;
  int16_t dir_ns;
  int16_t dir_ew;
  int16_t center;
  char oneline[128];
  size_t bytesread;
  char *p;
  
  char dirpath[64];
  File root;
  File file;
  char sometext[64];

  lv_label_set_text(ui_Station_Name, "Tuning...");
  lv_label_set_text(ui_Station_Title, "");
  Lvgl_Loop();

  // convert to whole degrees to be used for finding directory path to url file
  ns = DataFromDisplay.ns_cal;
  ew = DataFromDisplay.ew_cal;
  if(ns>0)ns+=5;
  else ns-=5;
  if(ew>0)ew+=5;
  else ew-=5;
  ns/=10;
  ew/=10;

  // search for url file around center [1]
  //           10 11 12  
  //        21  9  2  3 13 
  //        20  8  1  4 14
  //        19  7  6  5 15
  //           18 17 16 

  Stations.count = 0;
  Stations.requested -1;
      
  if (SD_MMC.begin("/sdcard", true, false))
  { center = 1;
    while(dirscan)
    { switch(center)
      { case 1:
          dir_ns = ns;
          dir_ew = ew;
          break;
        case 2:
          dir_ns = ns+1;
          dir_ew = ew;
          break;
        case 3:
          dir_ns = ns+1;
          dir_ew = ew+1;
          break;
        case 4:
          dir_ns = ns;
          dir_ew = ew+1;
          break;
        case 5:
          dir_ns = ns-1;
          dir_ew = ew+1;
          break;
        case 6:
          dir_ns = ns-1;
          dir_ew = ew;
          break;
        case 7:
          dir_ns = ns-1;
          dir_ew = ew-1;
          break;
        case 8:
          dir_ns = ns;
          dir_ew = ew-1;
          break;
        case 9:
          dir_ns = ns+1;
          dir_ew = ew-1;
          break;
  //           10 11 12  
  //        21  9  2  3 13 
  //        20  8  1  4 14
  //        19  7  6  5 15
  //           18 17 16 

        case 10:
          dir_ns = ns+2;
          dir_ew = ew-1;
          break;
        case 11:
          dir_ns = ns+2;
          dir_ew = ew;
          break;
        case 12:
          dir_ns = ns+2;
          dir_ew = ew+1;
          break;
        case 13:
          dir_ns = ns+1;
          dir_ew = ew+2;
          break;
        case 14:
          dir_ns = ns;
          dir_ew = ew+2;
          break;
        case 15:
          dir_ns = ns-1;
          dir_ew = ew+2;
          break;
  //           10 11 12  
  //        21  9  2  3 13 
  //        20  8  1  4 14
  //        19  7  6  5 15
  //           18 17 16 
        case 16: dir_ns = ns-2; dir_ew = ew+1; break;
        case 17: dir_ns = ns-2; dir_ew = ew;   break;
        case 18: dir_ns = ns-2; dir_ew = ew-1; break;
        case 19: dir_ns = ns-1; dir_ew = ew-2; break;
        case 20: dir_ns = ns;   dir_ew = ew-2; break;
        case 21: dir_ns = ns+1; dir_ew = ew-2; break;

        default:
          // game over
          lv_label_set_text(ui_Station_Name, "No Stations Found");
          strcpy(Stations.StationNUG[0].name, "");
          strcpy(Stations.StationNUG[0].url, "");
          Lvgl_Loop();
          dirscan = false;
          break;  
      }

      sprintf(dirpath, "/%c/%d/%d/%c/%d/%d", (dir_ns<0)?'S':'N', abs(dir_ns)/10, abs(dir_ns)%10, (dir_ew<0)?'W':'E', abs(dir_ew)/10, abs(dir_ew)%10);
      Serial.println(dirpath);
      lv_label_set_text(ui_Station_Title, dirpath);
      Lvgl_Loop();

      root = SD_MMC.open(dirpath);
      if (!root) 
      { Serial.println("Failed to open directory");
        center++;
      }
      else
      { dirscan = false;
        dirfound = true;
      }  
    }

    if(dirfound)
    { file = root.openNextFile();
      while (file && Stations.count<149) 
      { if(!file.isDirectory()) 
        { Serial.print("  FILE: ");
          Serial.print(file.name());
          Serial.print("  SIZE: ");
          Serial.println(file.size());
          lv_label_set_text(ui_Station_Title, file.name());
          Lvgl_Loop();

          while(file.available() && Stations.count<MAX_STATIONS)//  && urlcount < 500)
          {  bytesread = file.readBytesUntil(0x0a, oneline, sizeof(oneline)-1);
             oneline[bytesread]=0;
             if((p = strrchr(oneline, '\"')) != NULL)*p=0; // get rid of the last "
             Serial.println(oneline);
             if((p = strstr(oneline, "\"name\": \"")) != NULL)
             { // example:  "name": "Dr P4 Syd"
               // Serial.println(p+9);
               p+=9; // jump forward to start of name
               strcpy(Stations.StationNUG[Stations.count].name, p);
             }  
             else if((p = strstr(oneline, "\"url\": \"")) != NULL)
             { // example:  "url": "https://stream.radio-fratz.de/stream_high.mp3"
               // Serial.println(p+8);
               p+=8; // jump forward to start of url
               strcpy(Stations.StationNUG[Stations.count].url, p);
             }  
             else if((p = strstr(oneline, "\"gps\": \"")) != NULL)
             { // example:  "url": "https://stream.radio-fratz.de/stream_high.mp3"
               Serial.println(p+8);
               p+=8; // jump forward to start of url
               sscanf(p, "%f,%f", &Stations.StationNUG[Stations.count].gps_ns, &Stations.StationNUG[Stations.count].gps_ew);

               //if((p = strstr(StationArray[stationsfound].url, ".m3u")) == NULL) // no links to m3u file
               //{ if((p = strstr(StationArray[stationsfound].url, ".pls")) == NULL) // no links in pls file
               //  { if((p = strstr(oneline, "mp3")) != NULL)stationsfound++; 
               //    else if((p = strstr(oneline, "MP3")) != NULL)stationsfound++;
               //  }  
               //}
               //else if((p = strstr(oneline, "aac")) != NULL)

               Stations.count++;
             }  
          }
        }
        file = root.openNextFile();
      }
      sprintf(sometext, "%d Stations Found", Stations.count);
      lv_label_set_text(ui_Station_Name, sometext);
    }

    SD_MMC.end();  
  }
  else
  { lv_label_set_text(ui_Station_Name, "No SD Card");
  }

  // for test only   
  // strcpy(DataFromDisplay.RadioUrlRequest,  "http://stream.srg-ssr.ch/m/rsj/mp3_128");
  // strcpy(DataFromDisplay.RadioUrlRequest,  "https://icecast.omroep.nl/radio1-sb-mp3");
  // strcpy(DataFromDisplay.RadioUrlRequest,  "https://185.74.70.31/vocenustrale-128.mp3");

  if(Stations.count)
  { 
    AddToQueueForGlobe("Reset Your DataFromGlobe.D_QueueStationIndex to -1", MESSAGE_NEW_LIST_LOADED);

    AddStationToQueueForGlobe(0);
    // turn of preset leds
    SetLed(1, 0);
    SetLed(2, 0);
    SetLed(3, 0);
    SetLed(4, 0);
  }  

  // dump to serial port
  //for(int n = 0; n<Stations.count; n++)
  //{ Serial.printf("[%d] N=%s\nU=%s\n", n, Stations.StationNUG[n].name, Stations.StationNUG[n].url);
  //}
}

void SetPixelInMap(int16_t ns, int16_t ew)
{ // ew varies between -180 and 180
  // -180 to be stored in [359]
  ew += 180;  // convert -180 to 179 range to 0-359
  ns += 90;   // convert -90 to 90 range to 0-180
  if(ns>180)ns=180;
  StationsMap.pixeldata[((180-ns)*48)+(ew/8)] |= 0x80>>(ew%8); //48 bytes, rounded up from 45 to get a 4-byte multiple
}


void tellme(void)
{ Serial.println(StationsMap.bmpheader.filler);
  Serial.println(StationsMap.bmpheader.type);
  Serial.println(StationsMap.bmpheader.size);
  Serial.println(StationsMap.bmpheader.reserved1);
  Serial.println(StationsMap.bmpheader.reserved2);
  Serial.println(StationsMap.bmpheader.offset);
  Serial.println(StationsMap.bmpheader.dib_header_size);
  Serial.println(StationsMap.bmpheader.width_px);
  Serial.println(StationsMap.bmpheader.height_px);
  Serial.println(StationsMap.bmpheader.num_planes);
  Serial.println(StationsMap.bmpheader.bits_per_pixel);
  Serial.println(StationsMap.bmpheader.compression);
  Serial.println(StationsMap.bmpheader.image_size_bytes);
  Serial.println(StationsMap.bmpheader.x_resolution_ppm);
  Serial.println(StationsMap.bmpheader.y_resolution_ppm);
  Serial.println(StationsMap.bmpheader.num_colors);
  Serial.println(StationsMap.bmpheader.important_colors);

  int n;
  int w;
  for(n=0;n<180;n++)
  { for(w=0;w<45;w++)
    { Serial.printf("%02x", (uint16_t)StationsMap.pixeldata[(n*48)+w]); //48 bytes, rounded up from 45 to get a 4-byte multiple
    }
    Serial.printf("\n");
  }
}


static uint32_t startMillis;
static uint32_t currentMillis;


void RadioGlobeClick(lv_event_t * e)
{ if(Stations.count == 0)
  { FindNewStation();
  }
  ReloadScroll();
  lv_scr_load(ui_StationSelectScreen);
  startMillis = millis(); 
}

void ReloadScroll(void)
{ //char rolldata[1024];
  uint16_t len;
  uint16_t space;
  uint16_t n;
  char *p;

  space = sizeof(rolldata);
  strcpy(rolldata, "No Stations");

  for(n = 0; n<Stations.count; n++)
  { if((len = strlen(Stations.StationNUG[n].name))>0)
    { //Serial.printf("n=%d space=%d len=%d\n", n, space, len);
      //Serial.printf("N=%s\nU=%s\n", StationArray[n].name, StationArray[n].url);
      //Serial.printf("space = %d\n", space);
      if((space-len)>2)
      { if(n==0)
        { p = rolldata;
        }
        else
        { *p++ = '\n'; // end previous line with a newline
        }  
        strcpy(p, Stations.StationNUG[n].name);
        p += len;
        space -= len;         
      }
      else break;
    }
    else break;
  }
  // Serial.println(rolldata);
  // Serial.println(n);
  // lv_roller_set_options(uic_StationRoller, rolldata, LV_ROLLER_MODE_INFINITE); (loop around mode but seems to crash too often)
  lv_roller_set_options(uic_StationRoller, rolldata, LV_ROLLER_MODE_NORMAL);
  startMillis = millis(); 
  Serial.printf("Reload Scroll (%d stations) done.\n", n);
}






void StationScroll(lv_event_t * e)
{ char content[256];
  static uint16_t RollSelection;
  uint16_t index;
  lv_event_code_t event_code = lv_event_get_code(e);

  // events go from PRESSED while scrolling. Stop&Release results in VALUE_CHANGED + CLICKED
  // another short click gives PRESSED+CHANGED+CLICKED

  index = lv_roller_get_selected(uic_StationRoller);
  sprintf(content, "%d-%d", index, Stations.count);
  lv_label_set_text(ui_StationRollerSelected, content); 
  
    if(event_code == LV_EVENT_CLICKED) 
    { index = lv_roller_get_selected(uic_StationRoller);
      currentMillis = millis(); 
      if(currentMillis - startMillis > 250)  //test to distuigish between scrolling and clicking
      { RollSelection = index;
      }
      else // short click
      { Serial.printf("index=%d\n", index);
        if(RollSelection == index)
        { if(index < MAX_STATIONS) // just check in case
          { Stations.playing = -1;
            Stations.requested = index; 
            sprintf(content, "%s - Tuning", Stations.StationNUG[Stations.requested].name);
            lv_label_set_text(ui_StationRollerComment, content); 
            AddStationToQueueForGlobe(Stations.requested);
          }
          Set_EXIO(EXIO_PIN8,High);
          delay(50);
          Set_EXIO(EXIO_PIN8,Low);
        }  
        RollSelection = index;
      }
    }
    if(event_code == LV_EVENT_PRESSED) 
    { startMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
    }
    
}



void StationRollPressed(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);
  //Serial.printf("hey->event=%s\n", "StationRollPressed()");
  StationScroll(e);
}

void StationRollVal(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);
  //Serial.printf("hey->event=%s\n", "StationRollVal()");
  StationScroll(e);
}

void StationRollClicked(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);
  //Serial.printf("hey->event=%s\n", "StationRollClicked");
  StationScroll(e);
}

void RollUp(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);
  //Serial.printf("hey->event=%s\n", "RollUp()");
  //StationScroll(e);
}

void RollDown(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);
  //Serial.printf("hey->event=%s\n", "Rolldown()");
  //StationScroll(e);
}


void AddStationToQueueForGlobe(uint16_t station)
{ // prepare gps position and url for the station we want to hear and get timezone for
  DataFromDisplay.D_QueueStationIndex = station;
  Stations.requested = station; 
  Stations.playing = -1;

  if(station<MAX_STATIONS)
  { DataFromDisplay.D_StationGpsNS = Stations.StationNUG[station].gps_ns;
    DataFromDisplay.D_StationGpsEW = Stations.StationNUG[station].gps_ew;
    AddToQueueForGlobe(Stations.StationNUG[station].name, MESSAGE_GET_TIMEZONE);
    AddToQueueForGlobe(Stations.StationNUG[station].url, MESSAGE_START_THIS_STATION);
    lv_label_set_text(ui_Station_Name, Stations.StationNUG[station].name);
    lv_label_set_text(ui_Station_Title, "");
  }
  else 
  { DataFromDisplay.D_StationGpsNS = Favorites[station-MAX_STATIONS].gps_ns;
    DataFromDisplay.D_StationGpsEW = Favorites[station-MAX_STATIONS].gps_ew;
    AddToQueueForGlobe(Favorites[station-MAX_STATIONS].name, MESSAGE_GET_TIMEZONE);
    AddToQueueForGlobe(Favorites[station-MAX_STATIONS].url, MESSAGE_START_THIS_STATION);
    lv_label_set_text(ui_Station_Name, Favorites[station-MAX_STATIONS].name);
    lv_label_set_text(ui_Station_Title, "");
  }
}

/*
Squareline Studio notes to self

extra characters in placed in font

settings font create:
fontname : font1
fontsize 20
Bpp 4
ASCII extended 0x20-0x2ff // expand as needed

Symbols 

ext-ascii 0x80-0xFF
¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ
ext-ascii 0x100-0x1FF
ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩ
ext-ascii 0x200-0x27F
ĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſƀƁƂƃƄƅƆƇƈƉƊƋ
ext-ascii 0x200-0x27F
ƌƍƎƏƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿǀǁǂǃǄǅǆǇǈǉǊǋǌǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟǠǡǢǣǤǥǦǧǨǩǪǫǬǭǮǯǰǱǲǳǴǵǶǷǸǹǺǻǼǽǾǿ

settings font create:
fontname : montserrat20extended
fontsize 20
Bpp 4
ASCII extended 0x20-0xff

Symbols : paste the above characters collections

*/



