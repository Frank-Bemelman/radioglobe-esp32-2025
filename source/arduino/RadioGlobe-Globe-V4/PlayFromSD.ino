// functionality for SD card

// there are max 254 countries
// array with countries 2 letter codes for which a folder exists on the SD card
#define MAXCOUNTRIES 254
// max 25 tracks to fill the playlist with
#define MAXSINGLETRACKS 10
#define MAXALBUMSTRACKS 15
#define MAXTRACKPATHLEN 256
#define MAXSONGTRACKS 100


char CountryCodeSelectorSD[3];
char StartPathSD[256];
int16_t SingleTracksIdx;
int16_t ArtistFoldersIdx;
char SingleTracks[100][64]; // in a country folder
char ArtistFolders[100][32]; // in a country folder
char Playlist[MAXSONGTRACKS][MAXTRACKPATHLEN];

uint16_t PlaylistTracks = 0;
uint16_t firstTrackInRoller = 0;
uint16_t totalTracksInRoller = 0;
uint16_t playTracksInRoller = 0;



// called from setup()
bool CheckSD(void)
{ if (!SD.begin(SD_CS, *hspi, 40000000)) // 40000000 works and so does 80000000, no difference though
  { Serial.println("[ERROR] No SD card present");
    return false;
  } 
  else 
  {
    Serial.println("[INFO] SD card present");
    Serial.print("[INFO] Capacity SD: ");
    Serial.print(SD.cardSize() / (1024 * 1024));
    Serial.println("MB");
    return true;
  }
}


// called by puck message (and subsequent when filehas ended)
void StartPlayFromSD(void)
{ static char PrevCountryCodeSelectorSD[3] = "";
  Serial.println("Play from SD as requested by puck ");

  // force a reload from SD
  if(strcmp(PrevCountryCodeSelectorSD, CountryCodeSelectorSD) !=NULL)
  { strcpy(PrevCountryCodeSelectorSD, CountryCodeSelectorSD); 
    bMusicMode = false;
    bMusicModePrev = false;
  }

  if(0) // for test purposes
  { stream.stopSong(); // stop whatever stream or file was playing
    Speakers(SPEAKERS_ON);
    Serial.println("Play 1000Kz test.wav from SD");
    stream.connectToFile(SD, "/1000hz10s.wav"); // play it
    return;
  }  

  if(bMusicMode)Serial.printf("MusicMode true\n");
  else Serial.printf("MusicMode false\n");

  bMusicMode = true;
  if(bMusicModePrev != bMusicMode)
  { bMusicModePrev = bMusicMode;
    Serial.printf("Do it for country %s\n", CountryCodeSelectorSD);
    AddToQueueForDisplay("1", MESSAGE_MUSIC_MODE);
    strcpy(ActiveStationTitle, "SD-Card Music Files");
    AddToQueueForDisplay(ActiveStationTitle, MESSAGE_STATION_NAME);
    DataFromGlobe.D_QueueStationIndex = -1; // forget radiostation playing
    PlaylistTracks=0;
    if(GlobeSettings.sdcard_present)CollectFilePathsForCountry(CountryCodeSelectorSD);
    else Serial.printf("No SD card???\n");
    
  }

 
  if(PlaylistTracks>0)PlaySomethingFromSD();
}


void PlaySomethingFromSD(void)
{ uint16_t idx;
  char *p;

  playTracksInRoller %= totalTracksInRoller;
  idx = firstTrackInRoller + playTracksInRoller++;
  
  Serial.printf("Play from SD Playlist -> %s\n", Playlist[idx]);

  SetVolumeMapped(0);

  if(stream.isRunning())
  { stream.stopSong(); // stop whatever stream or file was playing
    // chunkplayer.switchToMp3Mode();
  }  

  stream.connectToFile(SD, Playlist[idx]); // play it

  if(stream.isRunning())
  { // wait short time with volume 0 to avoid audible clicks/snippets between station switching
    // currentMillis = millis();
    // while (millis() - currentMillis < 250)
    // { stream.loop();
    // }    
    // get songname from entire filepath
    char *p;
    char *q;
    if((p=strrchr(Playlist[idx], '/'))!= NULL)
    { strcpy(ActiveSongTitle, p+1);
      if((q=strrchr(ActiveSongTitle, '.'))!= NULL)*q=0; // remove file extension
      AddToQueueForDisplay(ActiveSongTitle, MESSAGE_SONG_TITLE);
    }  
  }
  Serial.printf("Done\n");
}


int DirNested = 0;

void CollectFilePathsForCountry(char *countrycode)
{ if(strlen(countrycode))sprintf(StartPathSD, "/GLOBEMUSIC/%s", countrycode);
  else strcpy(StartPathSD, "/GLOBEMUSIC/JUKEBOX");

//  File root = SD.open(StartPathSD);
//  if (!root) 
//  { Serial.printf("Failed to open StartPathSD directory -> %s\n", StartPathSD);
//    strcpy(StartPathSD, "/GLOBEMUSIC/JUKEBOX");
//  }
  
  Serial.printf("Go get the tracks for %s\n", StartPathSD);
  // from this start path, dive in and collect max 25 random music files, dive in subdirectories if need be
  PlaylistTracks = 0; 
  playTracksInRoller = 0;
  DirNested = 0;
  CollectTracksPaths(StartPathSD);
  SendTracksToPuck();
  
}

char subdirectories[100][64];

char ScrollToAdd[256];


void CollectTracksPaths(char *path)
{ char trackdirfile[256];
  char txtline[256];
  uint16_t subdiridx = 0;

  Serial.println(path);
  
  sprintf(trackdirfile, "%s/track-dir.txt", path);

  File root = SD.open(path);
  if (!root) 
  { Serial.printf("Failed to open directory -> %s\n", path);
    return;
  }


  
  File trackdirtxt = SD.open(trackdirfile);
  if (!trackdirtxt) // create it if not exsisisting
  { Serial.printf("Failed to open -> %s\n", trackdirfile);
    trackdirtxt = SD.open(trackdirfile, "w", true);
    if (!trackdirtxt)return;

    // write list of files in directory
      // read names of all folders and names of all single tracks in start folder
    File file = root.openNextFile();

    while (file)
    { if (file.isDirectory()) 
      { Serial.print("  DIR : ");
        Serial.println(file.name());
        trackdirtxt.printf("D-%s\n", file.name());
      } 
      else 
      { if(strcasestr(file.name(), "mp3") || strcasestr(file.name(), "wav") || strcasestr(file.name(), "flac") ) // only accept music files
        { Serial.print("  -FILE: ");
          Serial.print(file.name());
          Serial.print("  -SIZE: ");
          Serial.println(file.size());
          trackdirtxt.printf("F-%s\n", file.name());
        }
      }
      file = root.openNextFile();
    }
    trackdirtxt.close();  
    trackdirtxt = SD.open(trackdirfile);
  }
  
  if(!trackdirtxt) return;

  while(trackdirtxt.available())
  { // read all of them{ int n;
    uint16_t n;
    n = trackdirtxt.readBytesUntil('\n', txtline, sizeof(txtline));
    txtline[n]=0;
    Serial.println(txtline);

    if(txtline[0]=='D')
    { if(subdiridx<100)
      { //strcpy(subdirectories[subdiridx], "/");
        strcpy(subdirectories[subdiridx++], &txtline[2]);
      }
      else break;
    }  
    else if(txtline[0]=='F')
    { if(PlaylistTracks<MAXSONGTRACKS)
      { Serial.printf("Added %d -> %s to path -> %s\n", PlaylistTracks, &txtline[2], path);
     
        sprintf(ScrollToAdd, "%s/%s", path, &txtline[2]);
        strcpy(Playlist[PlaylistTracks++], ScrollToAdd);
//        if(strlen(ScrollToAdd)<=QUEUEMESSAGELENGTH)
//        { AddToQueueForDisplay(ScrollToAdd, MESSAGE_PLAYLIST_SONG_ARTIST);
//        }
      }
      else break;
    }
  }

  



  trackdirtxt.close(); 


  if(PlaylistTracks<(25-1)) // need more tracks, open another, deeper directory
  { Serial.printf("We have %d tracks collected\n", PlaylistTracks);
    if(subdiridx) // we have directories
    { Serial.printf("We have %d subdirectories\n", subdiridx);
      DirNested++;

      sprintf(trackdirfile, "%s/%s", path, subdirectories[random(0, subdiridx)]);
      if(DirNested<3)CollectTracksPaths(trackdirfile);
    }
  }
}  


void SendTracksToPuck(void)
{ // add 25 songs at random
  uint16_t startidx;
  startidx = random(0, PlaylistTracks);
  uint16_t formaxsongs = 0;

  totalTracksInRoller = MIN(25, PlaylistTracks);
  startidx = random(0, totalTracksInRoller);
  firstTrackInRoller = startidx;
  
  if(PlaylistTracks) // could be zero, for directory only containing subdirectories
  { while(1)
    { if(strlen(Playlist[startidx]) < QUEUEMESSAGELENGTH)
      { Serial.printf("Filepath Send To Puck -> %s\n", Playlist[startidx]);
        AddToQueueForDisplay(Playlist[startidx], MESSAGE_PLAYLIST_SONG_ARTIST);
        formaxsongs++;
      }
      else
      { Serial.printf("Filepath Too long -> %s\n", Playlist[startidx]);
      }
      startidx++;
      startidx %= PlaylistTracks;
      if(formaxsongs>=MIN(25, PlaylistTracks))break;
    }  
  }
}

