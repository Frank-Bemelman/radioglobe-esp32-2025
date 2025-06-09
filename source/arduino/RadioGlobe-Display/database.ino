// functions for database of radio stations
// makes use of stations.json file stored on SD card
// install lib Json Streaming Parser 1.0.5. by Daniel Eichorn via library manager in Arduino IDE 2.3.6
// https://github.com/squix78/json-streaming-parser


#include "FS.h"
#include "SD_MMC.h"


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

  }
  bCheckDatabase = false;
//  lv_label_set_text(ui_DatabaseProgress, "");
//  lv_scr_load(ui_Home); 
  

}

void ReadDatabase(char *filename)
{ char content[128];
  File root = SD_MMC.open(filename);
  if (!root) 
  { sprintf(content, "Failed To Open %s", filename);
    lv_label_set_text(ui_DatabaseProgress, content);
    Lvgl_Loop();
    Serial.println(content);
    delay(2000); 
    return;
  }
  sprintf(content, "Succes opening %s", filename);
  lv_label_set_text(ui_DatabaseProgress, content);
  Lvgl_Loop();

  char oneline[128] = "";
  char *p;
  size_t bytesread;
  uint32_t filesize = 0;
  uint32_t fileposition = 0;
  uint16_t percentagedone;

  int counter = 0;
  uint16_t parsefase = 0;
  uint16_t linesread = 0;
  uint16_t stationsread = 0;
  uint32_t urlcount = 0;
  float floatn;
  float floate;
  int16_t ns;
  int16_t ew;
  char ns_dirname[32];
  char ew_dirname[32];

  filesize = root.size();
  Serial.printf("filesize of %s is %ld\n", filename, filesize);

  while(root.available()) // && counter < 10)
  { counter++; 
    fileposition = root.position();
    percentagedone = fileposition * 100 / filesize;

    if((GlobalTicker100mS%10)==0)
    { sprintf(content, "Processed %d%% urls=%ld", percentagedone, urlcount);
      lv_label_set_text(ui_DatabaseProgress, content);
      Lvgl_Loop();
    }
    if(root.find("\"coords\": {"))
    { linesread = 0;
      while(linesread<3)
      { bytesread = root.readBytesUntil(0x0a, oneline, sizeof(oneline));
        oneline[bytesread]=0;
        linesread++;
        if(bytesread>0) // serious data 
        { // check n or s
          // Serial.printf("lineread = %s", oneline);
          if((p = strstr(oneline, "\"n\": ")) != NULL)
          { Serial.println(p);
            sscanf(&p[5], "%f", &floatn);
            if(floatn>0)floatn+=0.5;
            else floatn-=0.5;
            ns = (int)floatn;
          }
          if((p = strstr(oneline, "\"e\": ")) != NULL)
          { Serial.println(p);
            sscanf(&p[5], "%f", &floate);
            if(floate>0)floate+=0.5;
            else floate-=0.5;
            ew = (int)floate;
          }
        }
      }  
    }

    if(root.available())
    {  // reconstruct coordinates folder structure for this "coords": find if not yet existing
      sprintf(content, "/%c/%d/%d/%c/%d/%d", (ns<0)?'S':'N', abs(ns)/10, abs(ns)%10, (ew<0)?'W':'E', abs(ew)/10, abs(ew)%10);
      Serial.println(content);
      CreateAllDirInPath(SD_MMC, content);

      // before making a new file with the urls for this find, delete the file and recreate
      strcat(content, "/urls.txt");
      SD_MMC.remove(content);
      File urls = SD_MMC.open(content, FILE_WRITE);
      urls.close();

      if (SD_MMC.exists(content)) 
      { Serial.printf("%s exists.\n", content);
      } 
      else 
      { Serial.printf("%s doesn't exist.\n", content);
      }

      // now get the stations into this file
      urls = SD_MMC.open(content, FILE_WRITE);
      if(root.find("\"urls\": ["))
      { stationsread = 0;
        while(stationsread<250) // actually lines read 
        { bytesread = root.readBytesUntil(0x0a, oneline, sizeof(oneline));
          oneline[bytesread]=0;
          stationsread++;
          if(bytesread>0) 
          { // Serial.printf("lineread = %s\n", oneline);
            if((p = strstr(oneline, "\"name\": ")) != NULL)
            { Serial.println(p);
              urls.println(p);
            }
            if((p = strstr(oneline, "\"url\": ")) != NULL)
            { Serial.println(p);
              urls.println(p);
              urlcount++;
            }
            if((p = strstr(oneline, "]")) != NULL)
            { Serial.println(p);
              stationsread = 1000;
            }
          }  
        }
      }
      urls.close();
    }
  }  
  root.close();

 

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





