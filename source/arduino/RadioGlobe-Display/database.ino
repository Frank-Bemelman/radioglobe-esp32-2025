// functions for database of radio stations
// makes use of stations.json file stored on SD card

#include "FS.h"
#include "SD_MMC.h"

typedef struct stationdata
{ char name[32];
  char url[64];
};

stationdata StationArray[250];

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

void CheckDatabase(lv_event_t * e)
{ bCheckDatabase = true;
}




// create a directory structure per NS degree and EW degree
// in theory 64800 files to store
// ./N/N<digit/10>/<digit%10>/E<digit/10>/<digit%10>
// ./N/N<digit/10>/<digit%10>/W<digit/10>/<digit%10>
// ./S/S<digit/10>/<digit%10>/E<digit/10>/<digit%10>
// ./S/S<digit/10>/<digit%10>/W<digit/10>/<digit%10>
// each directory containing filenames like N24W25urls.txt containing list of stations for that coordinate

void BuildDatabaseFromSD(void)
{ uint16_t timeout = 5;
  char content[128];

  if(strcmp(SecretCode, "GLOBE") != 0)
  { lv_scr_load(ui_PasswordScreen);
    bCheckDatabase = false;
    return;
  }

  Serial.printf("CheckDatabase clicked from SetupScreen\n");
  lv_scr_load(ui_DatabaseScreen);
  Lvgl_Loop();

  SD_MMC.end();
//  if(!SD_MMC.setPins(SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN,-1,-1,-1)){
//    printf("SD MMC: Pin change failed!\r\n");
//    return;
//  }
  //if (!SD_MMC.begin()) 
  if (!SD_MMC.begin("/sdcard", true, false))
  { lv_label_set_text(ui_DatabaseProgress, "No SD Card Found!");
    Lvgl_Loop();
    delay(1000);
    while(timeout)
    {  delay(1000);
       sprintf(content, "leaving in %d seconds", timeout);
       lv_label_set_text(ui_DatabaseProgress , content);
       Lvgl_Loop();
       timeout --;
    }
    delay(1000);
  }
  else
  { lv_label_set_text(ui_DatabaseProgress, "SD Card Found!");
    Lvgl_Loop();
    delay(2000);
  
    //listDir(SD_MMC, "/", 0);
    ReadDatabase("/stations.json"); // don't fortget the forward slash
    //listDir(SD_MMC, "/", 10);

    Serial.println("Einde verhaal database maken 2");

  }
  bCheckDatabase = false;
//  lv_label_set_text(ui_DatabaseProgress, "");
//  lv_scr_load(ui_Home); 
  Serial.println("Einde verhaal database maken 3");
  SD_MMC.end();

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
  int16_t ns;
  int16_t ew;
  File urls;

  File root = SD_MMC.open(filename, FILE_READ);
  if (!root) 
  { sprintf(sometext, "Failed To Open %s", filename);
    lv_label_set_text(ui_DatabaseProgress, sometext);
    Lvgl_Loop();
    Serial.println(sometext);
    delay(2000); 
    return;
  }
  sprintf(sometext, "Succes opening %s", filename);
  lv_label_set_text(ui_DatabaseProgress, sometext);
  Lvgl_Loop();

  
  filesize = root.size();
  Serial.printf("filesize of %s is %ld\n", filename, filesize);

  bool readcoords = false;
  uint16_t citycount = 0;

  while(root.available())//  && urlcount < 500)
  { currentMillis = millis();
    if (currentMillis - startMillis >= 500)  // every 5S screen update
    { startMillis = currentMillis;
      fileposition = root.position();
      percentagedone = fileposition * 100 / filesize;
      sprintf(sometext, "Processed %d%% urls=%ld", percentagedone, urlcount);
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
        { Serial.println(p);
          sscanf(&p[5], "%f", &floatn);
          if(floatn>0)floatn+=0.5;
          else floatn-=0.5;
          ns = (int)floatn;
          parsefase = 2;
        }        
        break;
      case 2:
        if((p = strstr(oneline, "\"e\": ")) != NULL)
        { Serial.println(p);
          sscanf(&p[5], "%f", &floate);
          if(floate>0)floate+=0.5;
          else floate-=0.5;
          ew = (int)floate;
          sprintf(dirpath, "/%c/%d/%d/%c/%d/%d", (ns<0)?'S':'N', abs(ns)/10, abs(ns)%10, (ew<0)?'W':'E', abs(ew)/10, abs(ew)%10);
          Serial.println(dirpath);
          CreateAllDirInPath(SD_MMC, dirpath);
          parsefase = 3;
        }
        break;
      case 3:  
        if((p = strstr(oneline, "\"urls\": [")) != NULL)
        { sprintf(sometext, "%s/%s", dirpath, outputfilename);
          SD_MMC.remove(sometext); // delete old file
          urls = SD_MMC.open(sometext, FILE_WRITE);
          parsefase = 4;
        }
        break;
      case 4:  
        if((p = strstr(oneline, "\"name\": ")) != NULL)
        { Serial.println(p);
          urls.println(p);  
          parsefase = 5;
        }
        if((p = strstr(oneline, "]")) != NULL)
        { Serial.println(p);
          urls.close();
          parsefase = 0;
        }
        break;
      case 5:  
        if((p = strstr(oneline, "\"url\": ")) != NULL)
        { Serial.println(p);
          urls.println(p);  
          urlcount++;
          parsefase = 4;
        }
        break;
      default:
        break;  
    }
  }  
  root.close();
  
  // long filename test, and yes it works
  //urls = SD_MMC.open("/wateenleukefile.txt", FILE_WRITE);
  //urls.close();

  percentagedone = 100;
  sprintf(sometext, "Processed %d%% urls=%ld", percentagedone, urlcount);
  lv_label_set_text(ui_DatabaseProgress, sometext);
  Lvgl_Loop();

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

void expertmode(lv_event_t * e)
{ lv_scr_load(ui_PasswordScreen);
}

static int16_t randomstation = 0;

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
  int16_t stationsfound;
  
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
          lv_label_set_text(ui_Station_Title, "No Stations Found");
          Lvgl_Loop();
          delay(2000);
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
      stationsfound = 0;
      while (file) 
      { if(!file.isDirectory()) 
        { Serial.print("  FILE: ");
          Serial.print(file.name());
          Serial.print("  SIZE: ");
          Serial.println(file.size());
          lv_label_set_text(ui_Station_Title, file.name());
          Lvgl_Loop();

          while(file.available())//  && urlcount < 500)
          {  bytesread = file.readBytesUntil(0x0a, oneline, sizeof(oneline)-1);
             oneline[bytesread]=0;
             if((p = strrchr(oneline, '\"')) != NULL)*p=0; // get rid of the last "
             if((p = strstr(oneline, "\"name\": \"")) != NULL)
             { Serial.print("NNNNN");
               Serial.println(p+9);
               strcpy(StationArray[stationsfound].name, p+9);
             }  
             else if((p = strstr(oneline, "\"url\": \"")) != NULL)
             { Serial.print("UUUUU");
               Serial.println(p+8);
               strcpy(StationArray[stationsfound].url, p+8);
               if((p = strstr(oneline, "mp3")) != NULL)stationsfound++;
               //else if((p = strstr(oneline, "aac")) != NULL)stationsfound++;

             }  
          }
          // mark end of list 
          strcpy(StationArray[stationsfound].name, "");
          strcpy(StationArray[stationsfound].url, "");
        }
        file = root.openNextFile();
      }
      sprintf(sometext, "%d Stations Found", stationsfound);
      lv_label_set_text(ui_Station_Name, sometext);
      Lvgl_Loop();
    }
    SD_MMC.end();  
    delay(1000);
  }
  else
  { lv_label_set_text(ui_Station_Title, "No SD Card");
    Lvgl_Loop();
    delay(2000);
  }

  
  
  
  switch(randomstation)
  { case 0:
      strcpy(DataFromDisplay.RadioUrlRequest,  "http://stream.srg-ssr.ch/m/rsj/mp3_128");
      randomstation++;
      break;
    case 1:
      strcpy(DataFromDisplay.RadioUrlRequest,  "https://icecast.omroep.nl/radio1-sb-mp3");
      randomstation++;
      break;           
    case 2:
      strcpy(DataFromDisplay.RadioUrlRequest,  "https://185.74.70.31/vocenustrale-128.mp3");
      randomstation = 0;
      break;
    default:
      break;
  }

  strcpy(DataFromDisplay.RadioUrlRequest, StationArray[0].url);
  lv_label_set_text(ui_Station_Name, StationArray[0].name);
  Lvgl_Loop();
  delay(1000);
}





