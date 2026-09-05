// functionality for SD card

// there are max 254 countries
// array with countries 2 letter codes for which a folder exists on the SD card
#define MAXCOUNTRIES 254
#define MAXTRACKPATHLEN 256
// max 25 tracks to fill the playlist with
#define MAXSONGTRACKS 25


char CountryCodeSelectorSD[3];
char StartPathSD[256];
char Playlist[MAXSONGTRACKS][MAXTRACKPATHLEN];

uint16_t PlaylistTracks = 0;
uint16_t TrackFromPlayListToPlay = 0;



// called from setup()
bool CheckSD(void)
{ if (!SD.begin(SD_CS, *hspi, 40000000)) // 40000000 works and so does 80000000, no difference though
  { Serial.println("[ERROR] No SD card present");

    if(GlobeSettings.globe_sd_gb != 0)
    { // wrong in config, correct it
      GlobeSettings.globe_sd_gb = 0;
      EEPROM.put(0x0, GlobeSettings);
      EEPROM.commit();
    }
    return false;
  } 
  else 
  { uint16_t GB;
    GB = ((SD.cardSize()/1000000)+500)/1000;
    if(GB%2)GB++;
    Serial.println("[INFO] SD card present");
    Serial.printf("[INFO] Capacity SD: %d GB\n", GB);

    if(GlobeSettings.globe_sd_gb != GB)
    { // wrong in config
      GlobeSettings.globe_sd_gb = GB;
      EEPROM.put(0x0, GlobeSettings);
      EEPROM.commit();
    }
    return true;
  }
}

// todo - make it fail safe when directory can't be opened, no tracks can be collected etc...
// called by puck message (and subsequent when filehas ended)
void StartPlayFromSD(void)
{ static char PrevCountryCodeSelectorSD[3] = "";
  Serial.printf("Play from SD as requested by puck using CountryCodeSelectorSD -> %s\n", CountryCodeSelectorSD);

  if(GlobeSettings.globe_sd_gb == 0)
  { Serial.printf("StartPlayFromSD() -> GlobeSettings.globe_sd_gb indicates No SD card!!!\n");
    return;
  }
  
  // force a reload from SD
  if(strcmp(PrevCountryCodeSelectorSD, CountryCodeSelectorSD) != 0)
  { strcpy(PrevCountryCodeSelectorSD, CountryCodeSelectorSD); 
    bMusicMode = false;
    bMusicModePrev = false;
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
    AddToQueueForDisplay("", MESSAGE_STATUS_LINE); // remove 'READING SD CARD'

    DataFromGlobe.D_QueueStationIndex = -1; // forget radiostation playing
    PlaylistTracks=0;
    CollectFilePathsForCountry(CountryCodeSelectorSD);
  }

 
  if(PlaylistTracks>0)PlaySomethingFromSD();
}


void PlaySomethingFromSD(void)
{ char *p;

  TrackFromPlayListToPlay %= PlaylistTracks;
  
  Serial.printf("Play idx %d\n", TrackFromPlayListToPlay);
  Serial.printf("Play from SD Playlist -> %s\n", Playlist[TrackFromPlayListToPlay]);

  PlayFromPlaylistByIndex(TrackFromPlayListToPlay);
  Serial.printf("PlaySomethingFromSD - Done\n");
}

void PlayFromPlaylistByIndex(uint16_t idx)
{ char message[16];
  char path_filename[256];
  if(idx < PlaylistTracks)
  {
    SetVolumeMapped(0);

    if(stream.isRunning())
    { stream.stopSong(); // stop whatever stream or file was playing
      bSomethingPlays = false;
  
    }  

    stream.connectToFile(SD, Playlist[idx]); // play this path/file

    if(stream.isRunning())
    { char *p;
      char *q;
      bSomethingPlays = true;
      strcpy(path_filename, Playlist[idx]+1); // copy without the leading '/' from path /GLOBEMUSIC/NL/....
      if((p=strrchr(Playlist[idx], '/'))!= NULL)
      { // path/file -> use file
        strcpy(ActiveSongTitle, p+1);
        if((q=strrchr(ActiveSongTitle, '.'))!= NULL)*q=0; // remove file mp3 or whatever extension
        AddToQueueForDisplay(ActiveSongTitle, MESSAGE_SONG_TITLE);
    
        if((p=strrchr(path_filename, '/'))!= NULL)*p=0;
        //sprintf(ActiveStationTitle, "SD-Card %s", path_filename);
        sprintf(ActiveStationTitle, "%s", path_filename);
        AddToQueueForDisplay(ActiveStationTitle, MESSAGE_STATION_NAME);

        sprintf(message, "%d", idx);
        AddToQueueForDisplay(message, MESSAGE_SET_ROLLER_INDEX); // adjust roller on puck
      }
    }

    TrackFromPlayListToPlay = idx+1; // prepare for next song

  }      
}


int DirNested = 0;

void CollectFilePathsForCountry(char *countrycode)
{ if(strlen(countrycode))sprintf(StartPathSD, "/GLOBEMUSIC/%s", countrycode);
  else strcpy(StartPathSD, "/GLOBEMUSIC/JUKEBOX");

  // use alternative jukebox path if no directories for this country exists
  File root = SD.open(StartPathSD);
  if (!root) 
  { Serial.printf("Failed to open StartPathSD directory -> %s\n", StartPathSD);
    strcpy(StartPathSD, "/GLOBEMUSIC/JUKEBOX");
  }
  
  Serial.printf("Go get the tracks for %s\n", StartPathSD);
  // from this start path, dive in and collect max 25 random music files, dive in subdirectories if need be
  PlaylistTracks = 0; 
  TrackFromPlayListToPlay = 0; // always start at top of list
  DirNested = 0;
  CollectTracksPaths(StartPathSD); // collect MAXSONGTRACKS (or possibly less) tracks
  ShuffleTracks();
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

void ShuffleTracks(void)
{ uint16_t idx;
  uint16_t sourceidx;
  char tracktoswap[MAXTRACKPATHLEN];
  if(PlaylistTracks<2)return; // nothing to shuffle
  // shuffle the list, start with last track and swap with a another random track
  idx = PlaylistTracks-1;
  Serial.printf("Shuffle %d tracks\n", PlaylistTracks);
  while(idx)
  { strcpy(tracktoswap, Playlist[idx]);
    sourceidx = random(0, idx);
    strcpy(Playlist[idx], Playlist[sourceidx]);
    strcpy(Playlist[sourceidx], tracktoswap);
    Serial.printf("Swapped idx %d and %d\n", idx, sourceidx);
    idx--;
  }
}


void SendTracksToPuck(void)
{ // max 25 songs send to puck
  uint16_t idx = 0;
  while(idx<PlaylistTracks)
  { if(strlen(Playlist[idx]) < QUEUEMESSAGELENGTH)
    { Serial.printf("Track %d Filepath Send To Puck -> %s\n", idx, Playlist[idx]);
      AddToQueueForDisplay(Playlist[idx], MESSAGE_PLAYLIST_SONG_ARTIST);
    }
    else
    { Serial.printf("Filepath Too long -> %s\n", Playlist[idx]);
    }
    idx++;
  }
}

