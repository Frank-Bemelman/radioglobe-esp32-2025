// functionality for SD card

// there are max 254 countries
// array with countries 2 letter codes for which a folder exists on the SD card
#define MAXCOUNTRIES 254
// max 25 tracks to fill the playlist with
#define MAXSINGLETRACKS 10
#define MAXALBUMSTRACKS 15
#define MAXTRACKPATHLEN 100

char CountriesOnSD[MAXCOUNTRIES][2];
char CountryCodeSelectorSD[3];
char StartPathSD[32];
int16_t SingleTracksIdx;
int16_t ArtistFoldersIdx;
char SingleTracks[100][64]; // in a country folder
char ArtistFolders[100][32]; // in a country folder
char Playlist[25][MAXTRACKPATHLEN];

// called from setup()
bool CheckSD(void)
{ if (!SD.begin(SD_CS, *hspi, 80000000)) // 40000000 works and so does 80000000
  { Serial.println("[ERROR] No SD card present");
    return false;
  } 
  else 
  {
    Serial.println("[INFO] SD card present");
    Serial.print("[INFO] Capacity SD: ");
    Serial.print(SD.cardSize() / (1024 * 1024));
    Serial.println("MB");
    ReadMusicFolderFromSD();
    return true;
  }
}

// called from CheckSD() when SD card detected
void ReadMusicFolderFromSD(void)
{ // directory SD card should contain a \GLOBEMUSIC folder
  // in this folder, should be a list of subfolders, named with 2 letter countrycodes
  // XX folder is for nautic areas
  // UNSORTED folder is for music collections without country associated
  // Each of these folders (may) contain playable files, mp3, wav etc
  // Each of these folders (may)(also) contain new folders, typically named after albums name

  // the \GLOBEMUSIC folder contents is nice to have, as it tells what countries are available
  // so we only read that folder to collect countries
  File root = SD.open("/GLOBEMUSIC");
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  File file = root.openNextFile();
  uint16_t country_idx = 0;
  size_t filenamelength;
  while (file && country_idx<MAXCOUNTRIES) {
    if (file.isDirectory()) 
    { Serial.print("  DIR : ");
      Serial.println(file.name());
      // is it a country folder?
      if(strlen(file.name())==2)
      { memcpy(&CountriesOnSD[country_idx], file.name(), 2);
        country_idx++;
        CountriesOnSD[country_idx][0] = 0;
      }
      else CountriesOnSD[country_idx][0] = 0; // end list
    } 
    else 
    { Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }

  Serial.println(CountriesOnSD[0]);
}


void StartPlayFromSD(void)
{ bool found = false;
  Serial.println("Play from SD as requested by puck ");
  Serial.print("CountriesOnSD[0] -> "); Serial.println(CountriesOnSD[0]);
  Serial.print("CountryCodeSelectorSD -> ");Serial.println(CountryCodeSelectorSD);
  uint16_t country_idx = 0;
  while((CountriesOnSD[country_idx][0] != 0) && !found)
  { //Serial.printf("CountriesOnSD[%d] = %.2s\n", country_idx, CountriesOnSD[country_idx]);
    if(memcmp(CountriesOnSD[country_idx], CountryCodeSelectorSD, 2) == 0)
    { // country folder exists on SD
      found = true;
      break;
    }
    country_idx++;
  }
  
  if(found)
  { sprintf(StartPathSD, "/GLOBEMUSIC/%s", CountryCodeSelectorSD);
    GetFoldersAndTracksFromSD();
  }
  else
  { sprintf(StartPathSD, "/GLOBEMUSIC/JUKEBOX", CountryCodeSelectorSD); // or from UNSORTED?? MOVE JUKEBOX to UNSORTED??
    GetFoldersAndTracksFromSD();
  }

  PlaySomethingFromSD();

}

void GetFoldersAndTracksFromSD(void)
{ SingleTracksIdx = 0;
  ArtistFoldersIdx = 0;
  Serial.print("Get first from ");Serial.println(StartPathSD);
  
  File root = SD.open(StartPathSD);
  if (!root) {
    Serial.println("Failed to open StartPathSD directory");
    return;
  }
  
  // read names of all folders and names of all single tracks in start folder
  File file = root.openNextFile();
  while (file && (ArtistFoldersIdx<100) &&  (SingleTracksIdx<100))
  { if (file.isDirectory()) 
    { Serial.print("  DIR : ");
      Serial.println(file.name());
      strcpy(ArtistFolders[ArtistFoldersIdx], file.name());
      ArtistFoldersIdx++;
    } 
    else 
    { if(strcasestr(file.name(), "mp3") || strcasestr(file.name(), "wav") ) // only accept music files
      { Serial.print("  -FILE: ");
        Serial.print(file.name());
        Serial.print("  -SIZE: ");
        Serial.println(file.size());
        strcpy(SingleTracks[SingleTracksIdx], file.name());
        SingleTracksIdx++;
      }
    }
    file = root.openNextFile();
  }
}


void PlaySomethingFromSD(void)
{ uint16_t n;
  uint16_t randomidx;
  uint16_t tracks = 0;
  char filepath[256];

  stream.stopSong(); // stop whatever stream or file was playing
  // we have 0-?? single tracks
  // we also have 0-?? artist or collection folders containing album folders
  // let's try to collect 25 random songs to create a playlist
  // max 10 from the single tracks
  // max 15 from the albums

  randomidx = random(0, SingleTracksIdx);
  Serial.printf("randomidx = %d\n", randomidx);
  for(n=0;n<MIN(MAXSINGLETRACKS, SingleTracksIdx);n++)
  { if(strlen(SingleTracks[randomidx]) < MAXTRACKPATHLEN)
    { strcpy(Playlist[n], SingleTracks[randomidx]);
      Serial.printf("Playlist added random #%02d -> %s\n", randomidx, Playlist[tracks]);
      tracks++;
    }
    randomidx++;
    randomidx %= SingleTracksIdx;
  }

  //Serial.printf("Playlist tracks = %d\n", tracks);

  n = tracks;
  while(n < (MAXSINGLETRACKS + MAXALBUMSTRACKS))
  { // go find more tracks in ArtistFolders[]
    // go dig into folders in StartPathSD
    // if no folders in there
    n++;
  }

  n = tracks;
  while(n<(MAXSINGLETRACKS + MAXALBUMSTRACKS)) 
  { // go find more tracks in ArtistFolders[]
    strcpy(Playlist[n], "EMPTY");
    n++;
  }

  n = 0;
  // for(n=0; n<(MAXSINGLETRACKS + MAXALBUMSTRACKS); n++)
  while(n < (MAXSINGLETRACKS + MAXALBUMSTRACKS))
  { Serial.printf("Playlist #%02d -> %s\n", n, Playlist[n]);
    n++;
  }

  Serial.printf("%s/%s\n", StartPathSD, Playlist[0]);
  sprintf(filepath, "%s/%s", StartPathSD, Playlist[0]);
  stream.connecttofile(SD, filepath); // play it

  Serial.printf("Done\n");

}
