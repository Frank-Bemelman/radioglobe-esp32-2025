// functions for database of radio stations
// makes use of stations.json file stored on SD card

#include "FS.h"
#include "SD_MMC.h"
#include "database.h"
#include "countrycodes.h"

#include <ctype.h>


bmpfile StationsMap;  // to maintain a quick lookup map of available directories     
//bmpfile NauticMap;    // to have a map of water vs land that have actual timezones according to google
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
  char filename[32];

  Serial.printf("BuildDatabaseNow()\n");

  lv_label_set_text(ui_Database_Output_File, "");
  lv_obj_clear_flag(uic_RebuildDatabase, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(uic_RebuildDatabaseButtonText, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_state(uic_RebuildDatabase, LV_STATE_DISABLED);
  lv_obj_add_flag(uic_HomeButton3, LV_OBJ_FLAG_HIDDEN);
  //lv_label_set_text(ui_MapBanner, "All\nThose\nBeautiful\nInternet\nRadio\nStations\nAround\nThe\nWorld\n");

  if (!SD_MMC.begin("/sdcard", true, false))
  { lv_label_set_text(ui_Database_Dir_Path, "No SD Card Found!");
    Lvgl_Loop();
    delay(1000);
    while(timeout)
    {  delay(1000);
       sprintf(content, "leaving in %d seconds", timeout);
       lv_label_set_text(ui_Database_Dir_Path, content);
       Lvgl_Loop();
       timeout --;
    }
    delay(1000);
    lv_scr_load(ui_Home); 
    lv_label_set_text(ui_Database_Dir_Path, "");
  }
  else
  { lv_label_set_text(ui_Database_Dir_Path, "SD Card Found!");
    Lvgl_Loop();
  
    //strcpy(filename, "/stations.json"); // don't fortget the forward slash
    //strcpy(filename, "/stations1K.json"); // don't fortget the forward slash
    //strcpy(filename, "/stations20K.json"); // don't fortget the forward slash
    strcpy(filename, "/stations150K.json"); // don't fortget the forward slash

    ReadDatabase(SD_MMC, filename); // don't fortget the forward slash
    WriteStationsBitmapFile(SD_MMC, "/stationsmap.bmp");
    Serial.println("Einde verhaal database maken 2");
  }
  SD_MMC.end();
  Serial.println("Einde verhaal database maken 3");
  bCheckDatabase = false;
}

void ReadDatabase(fs::FS &fs, char *filename)
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

  if(1)
  { File root = fs.open(filename, FILE_READ);
    if (!root) 
    { sprintf(sometext, "Failed To Open %s", filename);
      lv_label_set_text(ui_Database_Dir_Path, sometext);
      Lvgl_Loop();
      Serial.println(sometext);
      delay(2000); 
      return;
    }
    sprintf(sometext, "Succes opening %s", filename);
    lv_label_set_text(ui_Database_Dir_Path, sometext);
    Lvgl_Loop();
    delay(2000); 
  
    filesize = root.size(); // used for progress percentage calculation
    // Serial.printf("filesize of %s is %ld\n", filename, filesize);

    lv_obj_set_pos(uic_MapCursor, ew, -ns);
    // Start the animation on the map
    //lv_obj_clear_state(uic_MapBanner, LV_STATE_DISABLED);
    lv_obj_clear_state(uic_MapCursor, LV_STATE_DISABLED);

    // invalidate map red dots, make it blank
    memset(StationsMap.pixeldata, 0x0, sizeof(StationsMap.pixeldata));

    while(root.available())//  && urlcount < 500)
    { bool UpdateGrapics;
      currentMillis = millis();
      if (currentMillis - startMillis >= 1000)  // every 5S screen update
      { startMillis = currentMillis;
        UpdateGrapics = true;
        fileposition = root.position();
        percentagedone = fileposition * 100 / filesize;
        sprintf(sometext, "Done %d%% urls=%ld", percentagedone, urlcount);
        lv_label_set_text(ui_Database_Progress, sometext);
        Lvgl_Loop();
      }

      bytesread = root.readBytesUntil(0x0a, oneline, sizeof(oneline)-1);
      oneline[bytesread]=0;
      //Serial.printf("bytesread= %d lineread = %s\n", bytesread, oneline);
      switch(parsefase)
      { case 0:
          if((p = strstr(oneline, "\": {")) != NULL) // city,country
          { // example
            //    "Winston-Salem,US": {
            // results in: Winston-Salem_US.txt  
            if((p=strchr(oneline, '\"'))!=NULL)
            { strcpy(outputfilename, p+1);
              if((p=strchr(outputfilename, '\"')) != NULL)*p=0;
              if((p=strchr(outputfilename, ',')) != NULL)*p='_';
              // seen a city, country entry like this "Biel/Bienne,CH": {
              // which causes havoc when filesystem sees this as a directory path
              // fix by replacing the / for a -
              if((p=strchr(outputfilename, '/')) != NULL)*p='-';
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
            if(floatn>0)ns=(int)(floatn+0.5);
            else ns=(int)(floatn-0.5);
            parsefase = 2;
          }        
          break;
        case 2: // create directory
          if((p = strstr(oneline, "\"e\": ")) != NULL)
          { //Serial.println(p);
            sscanf(&p[5], "%f", &floate);
            if(floate>0)ew=(int)(floate+0.5);
            else ew=(int)(floate-0.5);
            sprintf(dirpath, "/%c/%d/%d/%c/%d/%d", (ns<0)?'S':'N', abs(ns)/10, abs(ns)%10, (ew<0)?'W':'E', abs(ew)/10, abs(ew)%10);
            //Serial.println(dirpath);
            CreateAllDirInPath(fs, dirpath);
            SetPixelInMap(ns, ew);
            if(UpdateGrapics)lv_obj_set_pos(uic_MapCursor, ew, -ns);
            parsefase = 3;
          }
          break;
        case 3:  // create file
          if((p = strstr(oneline, "\"urls\": [")) != NULL) // start of list
          { sprintf(sometext, "Folder %s", dirpath);
            if(UpdateGrapics)lv_label_set_text(ui_Database_Dir_Path, sometext);
            if(UpdateGrapics)lv_label_set_text(ui_Database_Output_File, outputfilename);
            sprintf(sometext, "/%s/%s", dirpath, outputfilename);
            // SD_MMC.remove(sometext); // delete old file
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

    urls.close();
    root.close();
    //lv_obj_add_state(uic_MapBanner, LV_STATE_DISABLED); 
    lv_obj_add_state(uic_MapCursor, LV_STATE_DISABLED); 

    percentagedone = 100;
    sprintf(sometext, "Done %d%% urls=%ld", percentagedone, urlcount);
    lv_label_set_text(ui_Database_Progress, sometext);
    sprintf(sometext, "- %d Files Created -", urlcount);
    lv_label_set_text(ui_Database_Dir_Path, sometext);
    Lvgl_Loop();

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
    { //Serial.printf("Failed to open directory %s so make it!\n", pathcopy);
      if(SD_MMC.mkdir(pathcopy))
      { //Serial.printf("%s created\n", pathcopy);
      }
      //else Serial.printf("%s creation error\n", pathcopy);
    }
    *d = *s; // *s is '/' in case more directories todo
    // Serial.printf("*s = %d\n", (int)*s);
  }
}      

#define EXPAND_STOP 10 // was 45 - original radioglobe by Jude Pullen uses 2 degrees expansion, too small imo
bool SnapToNearestStation(int16_t for_ns, int16_t for_ew, int16_t *to_ns, int16_t *to_ew, uint16_t hit_count)
{ // convert/round to whole degrees to be used for finding a position with an existing directory path and station file
  int16_t ns;
  int16_t ew;
  int16_t expand;
  int16_t try_ns;
  int16_t try_ew;
  int16_t try_ltx;
  int16_t try_lty;
  int16_t try_rbx;
  int16_t try_rby;
  uint16_t match_count = 0;
  int16_t expand_stop;

  
  ns = for_ns;
  ew = for_ew;
  
  Serial.printf("[Database.ino ~ 352] SnapToNearestStation NS = %d  EW = %d\n", ns, ew);
  if(DisplaySettings.expand_search>100)DisplaySettings.expand_search=100;
  expand_stop = DisplaySettings.expand_search+1;
  Serial.printf("expand_stop = %d\n", expand_stop);
  
  // start at the exact position
  // expand as needed by 1 degree
  // hit_count parameter to find 1st or 2nd or 3rd pixel in map
  // to be used if 1st or next pixel gives a dissapointing small amount of stations

  try_ns = ns;
  try_ew = ew;
  expand = 0;

  
  while((expand < expand_stop) && (match_count!=hit_count))
  { // define the square to scan
    try_lty = ns - expand;
    try_ltx = ew - expand;
    try_rby = ns + expand;
    try_rbx = ew + expand;

    if(TestPixelInMap(try_ns, try_ew) && match_count==0)
    { match_count++;
      expand++;
    }
    else // loop around
    { try_ns = try_lty; 
      try_ew = try_ltx; 
      while(try_ew<try_rbx && (match_count!=hit_count)) // go right 
      { try_ew++;
        if(TestPixelInMap(try_ns, try_ew))
        { match_count++;
        }  
      }
      while(try_ns<try_rby && (match_count!=hit_count)) // go down 
      { try_ns++;
        if(TestPixelInMap(try_ns, try_ew))
        { match_count++;
        }  
      }
      while(try_ew>try_ltx && (match_count!=hit_count)) // go left 
      { try_ew--;
        if(TestPixelInMap(try_ns, try_ew))
        { match_count++;
        }  
      }
      while(try_ns>try_lty && (match_count!=hit_count)) // go up 
      { try_ns--;
        if(TestPixelInMap(try_ns, try_ew))
        { match_count++;
        }  
      }
      expand++;
    }
  }

  if(expand >= expand_stop)
  { Serial.printf("No station with expand %d -> NS -> %d WE -> %d\n", expand, try_ns, try_ew);
    return false; 
  }
  Serial.printf("Stations found within expand %d -> NS -> %d WE -> %d\n", expand, try_ns, try_ew);
  *to_ns = try_ns;
  *to_ew = try_ew;
  return true;
}

// find stations at current calibrated position
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

  // results from SnapToNearestStation()
  bool mapfind = false;
  int16_t mapfind_ns;
  int16_t mapfind_ew;

  int32_t mSstartsearch;
  uint16_t randomrange = 0;
  
 
  char dirpath[64];
  File root;
  File file;
  char content[64];
  
  

  // convert to whole degrees to be used for finding directory path to url file
  ns = DataFromDisplay.ns_cal;
  ew = DataFromDisplay.ew_cal;
  ns+= 5;
  ew+= 5;
  ns/=10;
  ew/=10;

  Stations.count = 0;
  Stations.requested -1;
  char firstcountrycode[3] = ""; // near borders, different countries can be in the directory, decided to stick with the first found


  mSstartsearch = millis();

  //mapfind = SnapToNearestStation(ns, ew, &mapfind_ns, &mapfind_ew, 1);
   
  //nieuw, probeer minimaal 10 stations te vinden, blijf witte pixels zoeken
  uint16_t hitcount = 0;
  while(Stations.count < 10 && hitcount<10)
  { hitcount++;
    mapfind = SnapToNearestStation(ns, ew, &mapfind_ns, &mapfind_ew, hitcount);
    if(mapfind == 0)
    { // game over
      break;
    }
    CollectStationsAtGps(mapfind_ns, mapfind_ew, firstcountrycode);
    if(randomrange==0)randomrange = Stations.count; // count from first succesful find
    Serial.printf("Collecting Attempt %d gave %d stations\n", hitcount, Stations.count);
  }

  
  Serial.printf("End search -> %ld\n", millis() - mSstartsearch);
  
  

  if(Stations.count)
  { sprintf(content, "%d Stations Found", Stations.count);
    lv_label_set_text(ui_Station_Name, content);
    
    AddToQueueForGlobe("Reset Your DataFromGlobe.D_QueueStationIndex to -1", MESSAGE_NEW_LIST_LOADED);
    
    // pick a random station from the list
    uint16_t random_station; 
    random_station = random(0,randomrange);
    Stations.connect_attempts = 0;
    ForceGlobeStationGPSupdate = true; 
    AddStationToQueueForGlobe(random_station);

    // turn of preset leds
    SetLed(1, 0);
    SetLed(2, 0);
    SetLed(3, 0);
    SetLed(4, 0);
  }  
  else
  { lv_label_set_text(ui_Station_Name, "No Stations Found");
      
    // normally, timezone is requested for the station that is requested
    // if no stations are found, set the timezone according to the coordinates from the globe
    Stations.requested = -1; 
    AddStationToQueueForGlobe(-1); // no new station to play really, but will trigger timezone/geolocation requests
  }

  
  // dump to serial port
  //for(int n = 0; n<Stations.count; n++)
  //{ Serial.printf("[%d] N=%s\nU=%s\n", n, Stations.StationNUG[n].name, Stations.StationNUG[n].url);
  //}

  Stations.connect_attempts = 0;
  ScrollNeedsReload = true;
}

void CollectStationsAtGps(int16_t mapfind_ns, int16_t mapfind_ew, char *firstcountrycode)
{ File root;
  File file;
  char dirpath[64];
  char content[64];
  int16_t dir_ns;
  int16_t dir_ew;
  char oneline[128];
  size_t bytesread;
  char *p;

  if (SD_MMC.begin("/sdcard", true, false))
  { dir_ns = mapfind_ns;
    dir_ew = mapfind_ew;
    sprintf(dirpath, "/%c/%d/%d/%c/%d/%d", (dir_ns<0)?'S':'N', abs(dir_ns)/10, abs(dir_ns)%10, (dir_ew<0)?'W':'E', abs(dir_ew)/10, abs(dir_ew)%10);
    Serial.println(dirpath);
    //lv_label_set_text(ui_Station_Title, dirpath);
  
    Lvgl_Loop();

    root = SD_MMC.open(dirpath);
    if (!root) 
    { Serial.println("Failed to open directory");
      return;
    }
    
    file = root.openNextFile();
//    while (file && Stations.count<(MAX_STATIONS-1))
// dit gaat op tilt, met name bij de reload scroll als meer dan ?? stations, zeker geen 100 of 150 mogelijk 
// dus even naar 25 max


    while (file && Stations.count<25) 
    { if(!file.isDirectory()) 
      { //Serial.print("STATIONS FILE: ");
        //Serial.print(file.name());
        //Serial.print(" FILE SIZE: ");
        //Serial.println(file.size());
        lv_label_set_text(ui_Station_Title, file.name());
        char town[32] = "";
        char countrycode[3] = "";
        char countryname[50]="";
        
        strcpy(town, file.name());
        // filenames are constructed from name of town, underscore, and 2 letter land code, like -> Naaldwijk_NL.txt
        if((p=strchr(town, '_')) != NULL)
        { *p++ = 0;
          //Serial.printf("sizeof(Countrylist) = %ld\n", sizeof(CountryList) / sizeof(country_info));
          strncpy(countrycode, p, 2);
          countrycode[2]=0;

          if(strlen(firstcountrycode) == 0)strcpy(firstcountrycode, countrycode);

          if(!FindCountryNameByCode(countryname, countrycode))
          { Serial.printf("Not a valid countrycode %s !!!!!!!!!!!!!!!\n", countrycode);
          }
          //Serial.printf("Found country: <%s>\n", countryname);
        }

        Lvgl_Loop();

        if(strcmp(firstcountrycode, countrycode) == 0) // same country
        {
        while(file.available() && Stations.count<25)
        { bytesread = file.readBytesUntil(0x0a, oneline, sizeof(oneline)-1);
          oneline[bytesread]=0;
          if((p = strrchr(oneline, '\"')) != NULL)*p=0; // get rid of the last "
          // Serial.println(oneline);
          if((p = strstr(oneline, "\"name\": \"")) != NULL)
          { // example:  "name": "Dr P4 Syd"
            // Serial.println(p+9);
            p+=9; // jump forward to start of name
            *(p+31)=0; 
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
            // Serial.println(p+8);
            p+=8; // jump forward to start of gps coordinates
            sscanf(p, "%f,%f", &Stations.StationNUG[Stations.count].gps_ns, &Stations.StationNUG[Stations.count].gps_ew);

            //if((p = strstr(StationArray[stationsfound].url, ".m3u")) == NULL) // no links to m3u file
            //{ if((p = strstr(StationArray[stationsfound].url, ".pls")) == NULL) // no links in pls file
            //  { if((p = strstr(oneline, "mp3")) != NULL)stationsfound++; 
            //    else if((p = strstr(oneline, "MP3")) != NULL)stationsfound++;
            //  }  
            //}
            //else if((p = strstr(oneline, "aac")) != NULL)
            strcpy(Stations.StationNUG[Stations.count].town, town);
            strcpy(Stations.StationNUG[Stations.count].countrycode, countrycode);
            strcpy(Stations.StationNUG[Stations.count].countryname, countryname);
            // in all honesty, ignore if country and ignore this url if already in list
            if(strcmp(firstcountrycode, countrycode) == 0) // same country
            { //Serial.printf("%s same as %s\n", firstcountrycode, countrycode);
              if(CheckIfUniqueUrl())Stations.count++;
            }
            else
            { Serial.printf("%s differs from %s\n", firstcountrycode, countrycode);
              //if(CheckIfUniqueUrl())Stations.count++;
            }
          }
        }
        }
        //file = root.openNextFile();
      }
      file = root.openNextFile();
    } 
    SD_MMC.end();  
  }
}

bool CheckIfUniqueUrl(void)
{ uint16_t idx=0;
  if(!Stations.count)return true;
  while(idx<Stations.count)
  { if(strcmp(Stations.StationNUG[Stations.count].url, Stations.StationNUG[idx].url)==0)return false; // url already in list of stations
    idx++;
  }
  return true;
}

void SetPixelInMap(int16_t ns, int16_t ew)
{ // ew varies between -180 and 180
  // -180 to be stored in [359]
  ew += 180;  // convert -180 to 179 range to 0-359
  ns += 90;   // convert -90 to 90 range to 0-180
  if(ns>179)ns=179;
  if(ew>359)ew-=360;
  StationsMap.pixeldata[((180-ns)*48)+(ew/8)] |= 0x80>>(ew%8); //48 bytes, rounded up from 45 to get a 4-byte multiple
}

bool TestPixelInMap(int16_t ns, int16_t ew)
{ // ew varies between -180 and 179
  // ew varies between 90 and -89
  // -180 to be stored in [359]
  ew += 180;  // convert -180 to 179 range to 0-359
  ns += 90;   // convert -90 to 90 range to 0-180
  if(ns>179)ns=179;
  if(ew>359)ew-=360;
  if(ns<1)return false;
  if(ew<1)return false;
  
  //Serial.printf("Pixeltest ns=%d ew=%d %02X vs %02X\n", ns, ew, (uint16_t)StationsMap.pixeldata[((180-ns)*48)+(ew/8)], (uint16_t)(0x80>>(ew%8)));
  if(StationsMap.pixeldata[((180-ns)*48)+(ew/8)] & (0x80>>(ew%8)))return true;
  return false;
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
{ int16_t clicktime;
  lv_event_code_t event_code = lv_event_get_code(e);

  clicktime = isLongPressedV2(e);
  Serial.printf("RadioGlobeClick() clicktime = %d\n", clicktime);
  
  if(clicktime==0) // normal click
  { if(!bMusicMode)
    { Serial.printf("!bMusicMode RadioGlobeClick() Stations.count = %d\n", Stations.count);
      if(Stations.count == 0)
      { SetFlag("xxxx");
        lv_obj_add_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // hide city name until new country code is received
        lv_obj_add_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // hide country name until new country code is received

        //lv_label_set_text(ui_Station_Name, "Searching..."); // pointless, searching goes so fast you won't see this
        lv_label_set_text(ui_Station_Title, "");
        Lvgl_Loop();  
        FindNewStation();
        ReloadScroll();
      }
    }
    else
    { Serial.printf("bMusicMode RadioGlobeClick() Stations.count = %d\n", Stations.count);
      ReloadScroll();
    }
    lv_scr_load(ui_StationSelectScreen);
    startMillis = millis(); 
    Serial.printf("RadioGlobeClick() return\n");
  }  

  if(clicktime==5) // long press
  { Serial.printf("RadioGlobeClick LV_EVENT_LONG_PRESSED_REPEAT \n");
    
    if(!bMusicMode)
    { Serial.printf("!bMusicMode RadioGlobeClick LV_EVENT_LONG_PRESSED_REPEAT \n");
      beepforMs(50);
      //if(isLongPressed(e)==5)
      { SetLed(0,0); SetLed(1,0); SetLed(2,0); SetLed(3,0);
        AddToQueueForGlobe("", MESSAGE_GLOBE_PLAY_SD);
      }  
    }  
  }
  
}

void ReloadScroll(void)
{ uint16_t len;
  uint16_t space;
  uint16_t n;
  char *p;

  if(!ScrollNeedsReload)return;
  ScrollNeedsReload = false;
  space = sizeof(rolldata);
  if(bMusicMode)strcpy(rolldata, "No Files");
  else strcpy(rolldata, "No Stations");
  lv_label_set_text(ui_StationRollerSelected, "0-0");
  lv_roller_set_selected(uic_StationRoller, 0, LV_ANIM_ON);

  for(n = 0; n<Stations.count; n++)
  { if((len = strlen(Stations.StationNUG[n].name))>0)
    { //Serial.printf("n=%d space=%d len=%d\n", n, space, len);
      //Serial.printf("N=%s U=%s\n", Stations.StationNUG[n].name, Stations.StationNUG[n].url);
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

  if(bMusicMode)
  { lv_label_set_text(ui_StationRollerComment, "Playlist From SD-Card"); // usually says connected to station or something
  }
  else 
  { lv_label_set_text(ui_StationRollerComment, ""); 
  }
  
  lv_label_set_text(uic_StationRollerPlace, "");


  lv_roller_set_options(uic_StationRoller, rolldata, LV_ROLLER_MODE_NORMAL);
  if(Stations.count)
  { char content[32];
    if((Stations.requested >=0) && (Stations.requested<Stations.count))
    { sprintf(content, "%d-%d", Stations.requested+1, Stations.count); // top label 1-150 in stations roller
      Serial.printf("lv_roller_set_selected -> %d\n", Stations.requested);
      lv_roller_set_selected(uic_StationRoller, Stations.requested, LV_ANIM_ON);
    }  
    else 
    { sprintf(content, "0-%d", Stations.count); // top label 1-150 in stations roller
      lv_roller_set_selected(uic_StationRoller, 0, LV_ANIM_ON);
    }
    lv_label_set_text(ui_StationRollerSelected, content);
  }
  Serial.printf("Reload Scroll (%d stations) done.\n", Stations.count);
}






void StationScroll(lv_event_t * e)
{ char content[256];
  static uint16_t RollSelection;
  uint16_t index;
  lv_event_code_t event_code = lv_event_get_code(e);

  // events go from PRESSED while scrolling. Stop&Release results in VALUE_CHANGED + CLICKED
  // another short click gives PRESSED+CHANGED+CLICKED

  index = lv_roller_get_selected(uic_StationRoller);
  sprintf(content, "%d-%d", index+1, Stations.count);
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
      { if(index < Stations.count) // just check in case
        { Stations.playing = -1;
          Stations.requested = index; 
          if(!bMusicMode)
          { sprintf(content, "Connecting to %s", Stations.StationNUG[Stations.requested].name);
            lv_label_set_text(ui_StationRollerComment, content); 
            Stations.connect_attempts = 0;
            AddStationToQueueForGlobe(Stations.requested);
          }  
          else AddFileToQueueForGlobe(Stations.requested);
        }
        beepforMs(50);
      }  
      RollSelection = index;
    }
  }
  if(event_code == LV_EVENT_PRESSED) 
  { startMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  }
}

uint16_t AddToScroll(char * songartist)
{ char *p;
  char *q;

  if(Stations.count >= 25)return Stations.count;
  //Serial.printf("songartist -> %s\n", songartist);

  strcpy(Stations.StationNUG[Stations.count].url, songartist);
  // get song name from url
  if((p=strrchr(songartist, '/'))!= NULL)
  { if((q=strrchr(songartist, '.'))!= NULL)*q=0;
    Serial.printf("songartist has / -> %s\n", songartist);
    //Serial.printf("songartist without / and without ext / -> %s count = %d\n", p+1, Stations.count);
    // example  /GLOBEMUSIC/HT/Emeline_Michel/Emeline_Michel_-_Reine_De_Coeur_-_2008/08_Yon_Ti_Mo.mp3
    p++; // skip the '/'

    if(isdigit(*p) && isdigit(*(p+1)) && *(p+2)==' ')p+=3; // skip the track number if part of songname
    else if( isalpha(*p) && isdigit(*(p+1)) && isdigit(*(p+2)) && *(p+3)=='-')p+=4; // skip the track number of jukebox files
    RemoveUTF8Unprintables(p); // converts to extended ascii where possible (todo make font table extended too)
    strcpy(Stations.StationNUG[Stations.count].name, p);
  } 
  else
  { strcpy(Stations.StationNUG[Stations.count].name, "NULL");
  }
  Stations.count++;
  ScrollNeedsReload = true;
  return Stations.count;
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


void AddStationToQueueForGlobe(int16_t station)
{ // prepare gps position and url for the station we want to hear and get timezone for
  static char lastgpsrequest[32];
  char message[QUEUEMESSAGELENGTH];
  char content[128];

  if(station < 0) // no actual station to play really, but will trigger timezone/geolocation requests
  { lv_label_set_text(uic_Home_City, "");
    //on map
    sprintf(content, "GPS NS %2.1f - EW %3.1f", (float)DataFromDisplay.ns_cal/10, (float)DataFromDisplay.ew_cal/10);
    lv_label_set_text(ui_Database_GPS_Position, content);
    lv_obj_set_pos(uic_MapCursor, (int)DataFromDisplay.ew_cal/10 - 16, -(int)DataFromDisplay.ns_cal/10); // moved 16 to left, dot is painted left-top corner?
    lv_label_set_text(ui_Database_Output_File, ""); // remove typical "You are in ..." from map

    AddToQueueForGlobe("", MESSAGE_GET_TIMEZONE); // will use DataFromDisplay.ns_cal, DataFromDisplay.ew_cal as coordinates
    AddToQueueForGlobe("", MESSAGE_GET_GEOLOCATION); // will use DataFromDisplay.ns_cal, DataFromDisplay.ew_cal as coordinates
  }
  else if(station<MAX_STATIONS+MAX_FAVORITES)
  { bMusicMode = false;
    DataFromDisplay.D_QueueStationIndex = station;
    Stations.requested = station; 
    Stations.playing = -1;

    if(station<MAX_STATIONS)Stations.connect_attempts++;
    DataFromDisplay.D_StationGpsNS = Stations.StationNUG[station].gps_ns;
    DataFromDisplay.D_StationGpsEW = Stations.StationNUG[station].gps_ew;
    sprintf(message, "%f-%f", DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW);

    lv_label_set_text(ui_Station_Name, Stations.StationNUG[station].name);
    lv_label_set_text(ui_Status_Line, "CONNECTING");
    lv_label_set_text(ui_Station_Title, "");

    // check if gps position is a new one
    if(strcmp(lastgpsrequest, message) != NULL || (ForceGlobeStationGPSupdate == true))
    { strcpy(lastgpsrequest, message);
      ForceGlobeStationGPSupdate = false;

      // early prepare globe to load new directory from this country with music urls from SD
      AddToQueueForGlobe(Stations.StationNUG[station].countrycode, MESSAGE_RELOAD_SD_WITH_COUNTRY);

      AddToQueueForGlobe("", MESSAGE_GET_GEOLOCATION_BY_GPS);
      AddToQueueForGlobe("", MESSAGE_GET_TIMEZONE_BY_GPS);

      Serial.printf("Flag = %s\n", Stations.StationNUG[station].countrycode);

      // show immedeately what is known
      // set flags early - possibly corrected later, when globe thinks it is different 
      SetFlag(Stations.StationNUG[station].countrycode);
      lv_obj_clear_flag(uic_Home_Flag, LV_OBJ_FLAG_HIDDEN); // as it was hidden by a new search start

      Serial.printf("Town = %s\n", Stations.StationNUG[station].town);
      lv_label_set_text(uic_Home_City, Stations.StationNUG[station].town);
      lv_obj_clear_flag(uic_Home_City, LV_OBJ_FLAG_HIDDEN); // it was hidden by a new search start

      Serial.printf("Country = %s\n", Stations.StationNUG[station].countryname);
      strcpy(content, Stations.StationNUG[station].countryname);
      lv_label_set_text(uic_Home_Country, AllUpperCase(content));
      lv_obj_clear_flag(uic_Home_Country, LV_OBJ_FLAG_HIDDEN); // it was hidden by a new search start
      lv_label_set_text(uic_Clock_Country, AllUpperCase(content));
      lv_obj_clear_flag(uic_Clock_Country, LV_OBJ_FLAG_HIDDEN); // unhide country name 


      strcpy(GlobePositionCountryCode, Stations.StationNUG[station].countrycode);

      // also update the world map screen
      sprintf(content,"Greetings From  %s", AllUpperCase(Stations.StationNUG[station].countryname));
      lv_label_set_text(ui_Database_Town_Name, content);
      lv_label_set_text(ui_Database_Progress, ""); // erase exchange currency and rate
      lv_obj_set_pos(uic_MapCursor, (int)Stations.StationNUG[station].gps_ew - 16, -(int)Stations.StationNUG[station].gps_ns); // moved 16 to left, dot is painted left-top corner?
      sprintf(content, "GPS NS %2.4f - EW %3.4f", Stations.StationNUG[station].gps_ns, Stations.StationNUG[station].gps_ew);
      lv_label_set_text(ui_Database_GPS_Position, content);
      sprintf(content,"You Are In  %s", Stations.StationNUG[station].town);
      lv_label_set_text(ui_Database_Output_File, content);


      ShowWeatherData(false); // will be restored later, when globe has fetched new weather info
    }

    if(bPowerStatus == true)
    { AddToQueueForGlobe(Stations.StationNUG[station].url, MESSAGE_START_THIS_STATION);
    }

  }
}

void AddFileToQueueForGlobe(uint16_t station)
{ //DataFromDisplay.D_QueueStationIndex = station;
  //Stations.requested = station; 
  //Stations.playing = -1;
  char message[16];
  
  if(station<Stations.count)
  { if(bPowerStatus == true)
    { //AddToQueueForGlobe(Stations.StationNUG[station].url, MESSAGE_START_THIS_FILE);
      sprintf(message, "%d", station);
      AddToQueueForGlobe(message, MESSAGE_START_FILE_BY_INDEX);
    }
    //lv_label_set_text(ui_Station_Title, Stations.StationNUG[station].name); // rebuild songname from url
    lv_label_set_text(ui_Station_Title, ""); // for now
  }
}


void WriteStationsBitmapFile(fs::FS &fs, char* filename)
{ Serial.printf("WriteStationsBitmapFile -> %s\n", filename);
  File bitmap = fs.open(filename, FILE_WRITE);
  if(bitmap) 
  { Serial.printf("WriteStationsBitmapFile opened -> %s\n", filename);
    char *p = (char*) &StationsMap;
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
    for(n=179;n>=0;n--) // 180 latitudal lines
    { p = (char*)&StationsMap.pixeldata[(n*48)];
      for(w=0;w<48;w++)
      { bitmap.write(*p++);
      }
    }
  }
  bitmap.close();
  Serial.printf("WriteStationsBitmapFile closed -> %s\n", filename);
}


void ReadStationsBitmapFile(fs::FS &fs, char* filename)
{ File bitmap = fs.open(filename, FILE_READ);
  char *p = (char*) &StationsMap;
  p+=2; // skip filler in structure
  if(!bitmap)
  { Serial.printf("No File %s found\n", filename);
    return;
  }
  Serial.printf("File %s opened\n", filename);
  while(bitmap.available())
  { int n = 54+8;
    while(n)
    { char c = bitmap.read();
      *p++ = c;
      n--;
    }

    int w;
    for(n=179;n>=0;n--) // 180 latitudal lines
    { p = (char*)&StationsMap.pixeldata[(n*48)];
      for(w=0;w<48;w++)
      { char c = bitmap.read();
        *p++ = c;
      }
    }
  }
  bitmap.close();
  // WriteStationsBitmapFile(fs, "/stationsmap-copy.bmp"); // test, make a copy
}


void AppendBadStationToFile(fs::FS &fs, char* filename, char *url)
{ File badfile = fs.open(filename, FILE_APPEND);

  if(!badfile)
  { Serial.printf("Could not open Bad Station File %s for append\n", filename);
    badfile.close();
    badfile = fs.open(filename, FILE_WRITE);
    badfile.printf("File %s created\n", filename);  
    Serial.printf("File %s created\n", filename);  
    badfile.close();
    return;
  }
  Serial.printf("Station File %s appended with %s\n", filename, url);
  badfile.println(url);  
  badfile.close();
}

bool FindCountryNameByCode(char *countryname, char*countrycode)
{ uint16_t n=0;
  static char prevcountrycode[3] = "";
  static char prevcountryname[50] = "";
  
  if(strcmp(prevcountrycode, countrycode)==NULL)
  { strcpy(countryname, prevcountryname);
    return true;
  }
  while(n<(sizeof(CountryList) / sizeof(country_info)))
  { //Serial.printf("CountryList[%d].name = %s countrycode %s\n", n, CountryList[n].name, CountryList[n].code);
    if(strncmp(countrycode, CountryList[n].code, 2)==NULL)
    { strncpy(prevcountrycode, CountryList[n].code, 2);
      prevcountrycode[2]=0;
      strncpy(prevcountryname, CountryList[n].name, 49);
      prevcountryname[49]=0;
      //Serial.println(countryname);
      strcpy(countryname, prevcountryname);
      return true;
    }
    n++;
  }
  strcpy(countryname, "");
  return false;
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



char * AllUpperCase(char *s)
{ int i, slash_count = 0;
  int len = strlen(s);

  // replace all occurencies of '/' with " / "
  // and make all characters uppercase 
  // example "TZ - Asia/Calcutta" becomes -> "TZ - ASIA / CALCUTTA"

  // Serial.println(s);
  // count occurencies of '/'
  for (i = 0; i < len; i++) 
  { if (s[i] == '/')
    slash_count++;
  }

  // new length
  int new_len = len + slash_count * 2;
  s[new_len--] = '\0';

  // Serial.println(new_len);

  // work backwards
  for (i = len - 1; i >= 0; i--) 
  { //Serial.printf("%d = '%c'\n", i, s[i]);
    if (s[i] == '/') 
    { s[new_len--] = ' ';
      s[new_len--] = '/';
      s[new_len--] = ' ';
    } 
    else 
    { s[new_len--] = toupper(s[i]);
    }
  }
  // Serial.println(s);
  return s;
}
