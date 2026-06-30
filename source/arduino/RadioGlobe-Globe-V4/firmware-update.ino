// update of firmware
// new files are located at: github
// steps to take
// - check most recent version
// - download to SD if newwer
// - install from SD
// - update build date in eeprom


#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

uint16_t UpdateState = 0;

const char firmware_file[] = "/firmware.bin";

// Target file in Github release
char file_url[] = "https://github.com/Frank-Bemelman/radioglobe-esp32-2025/releases/download/RadioGlobe-Firmware/RadioGlobe-Globe.bin";

// worthless
void PlayComplete(char* soundfile)
{ DataFromGlobe.D_QueueStationIndex = -1; 
  stream.stopSong(); // stop whatever stream or file was playing
  Speakers(SPEAKERS_ON);
  SetVolumeMapped(DataFromDisplay.volumevalue); 
  delay(2000);
  stream.connectToFile(SD, soundfile); // play it 
  while(stream.isRunning())
  { stream.loop();
    delay(25);
  }
  int delaytime = 180;
  while(delaytime--)
  { stream.loop();
    delay(25);
  }  
}


uint8_t UpdateFirmware(uint8_t state)
{ char GithubTimeStamp[32];
  File file;

  while(true)
  { Serial.printf("UpdateFirmware process state %d\n", state);
    switch(state)
    { case 0: // nothing to do
       return state;
       break;

     case 1:
       //GetTimeStampGitHubReleaseV2(file_url, GithubTimeStamp)
       AddToQueueForDisplay("UPDATING GLOBE", MESSAGE_STATION_NAME);
       AddToQueueForDisplay("Checking For New Firmware", MESSAGE_SONG_TITLE);
       AddToQueueForDisplay(GlobeSettings.ssid , MESSAGE_SSID_FOR_GLOBE);
       AddToQueueForDisplay(GlobeSettings.password, MESSAGE_PASSWORD_FOR_GLOBE);
       //PlayComplete("/GLOBEMUSIC/SOUNDS/checking_new_firmware.mp3"); // sound effect
       //PlayComplete("/GLOBEMUSIC/SOUNDS/Download Blues.mp3"); // sound effect
       
       delay(1000);

       if(!GetTimeStampGitHubReleaseV2(file_url, GithubTimeStamp))
       { AddToQueueForDisplay("No Globe Update Available", MESSAGE_SONG_TITLE);
         delay(1000);
         AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
         return 0;
       } 
       state++;
       break;

     case 2:
       // sooner or later
       // check if github has a newer version than currently running
       if(!CheckIfNewer(GithubTimeStamp, build_timestamp_only))
       { Serial.printf("Updater: Github %s is older than running version %s - quit update!\n", GithubTimeStamp, build_timestamp_only);
         AddToQueueForDisplay("Globe Firmware Is The Latest", MESSAGE_SONG_TITLE);
         delay(1000);
         AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
         return 0;
       }
       Serial.printf("Updater: Github %s is NEWER than running version %s - continue update!\n", GithubTimeStamp, build_timestamp_only);
       state++;
       break;
     case 3:
       // github is more recent
       // delete existing bin firmawere file first
       if(SD.remove(firmware_file))
       { Serial.printf("Updater -> old file: %s succesfully deleted!\n", firmware_file);
       }
       else
       { Serial.printf("Updater: no %s present on SD card!\n", firmware_file);
       }
       state++;
       break;
     case 4:
       AddToQueueForDisplay("New Globe Firmware Downloading", MESSAGE_SONG_TITLE);
       delay(1000);
       if(!DownloadGithubFile(file_url))
       { AddToQueueForDisplay("Globe Firmware Download Failed", MESSAGE_SONG_TITLE);
         delay(1000);
         AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
         return 0;
       }        
       Serial.printf("Updater: new firmware downloaded to SD card!\n");

       state++;
       break;

     case 5:
       DataFromGlobe.D_QueueStationIndex = -1; 
       stream.stopSong(); // stop whatever stream or file was playing
       Speakers(SPEAKERS_ON);
       SetVolumeMapped(DataFromDisplay.volumevalue); 
       stream.connectToFile(SD, "/GLOBEMUSIC/SOUNDS/Download Blues.mp3"); // play it 
       //  stream.connectToFile(SD, "/GLOBEMUSIC/JUKEBOX/D04-Gloria Estefan-Higher.mp3"); // play it 
       state++;
       break;
 
     case 6:
       AddToQueueForDisplay("Update Globe ESP32 Processor", MESSAGE_SONG_TITLE);
       delay(1000);
       if(!UpdateFromSD(file_url))
       { AddToQueueForDisplay("Update Globe ESP32 Failed", MESSAGE_SONG_TITLE);
         delay(1000);
         AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
         return 0;
       }        
       state++;
       break;
     
     case 10:
       return 0;
       break;
     default:
       state++;
       break;
    }
  }  
}


bool UpdateFromSD(char *file_url)
{ File firmware = SD.open("/firmware.bin");
  
  Serial.printf("Updater: start flashing new firmware from SD card!\n");
  if (!firmware) 
  { Serial.println("Failed to open firmware.bin");
    return false;
  }

  size_t firmwareSize = firmware.size();
  if (firmwareSize == 0) 
  { Serial.println("UpdateFromSD -> File is empty");
    return false;
  }

  // Begin the update process, passing the size of the file
  if (!Update.begin(firmwareSize)) 
  { Serial.println("UpdateFromSD -> Not enough space for update");
    return false;
  }

  // Stream the firmware from the SD card to the OTA partition
  Serial.println("Updating firmware, please wait...");
  if (Update.writeStream(firmware) != firmwareSize) 
  {  Serial.println("UpdateFromSD -> Update failed");
     return false;
  }

    // Finalize the update
  if (Update.end()) 
  { AddToQueueForDisplay("Restarting RadioGlobe...", MESSAGE_SONG_TITLE);
    delay(1000);
    AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
    delay(1000); // allow some tine to send pending queue messages to puck
    Serial.println("UpdateFromSD -> Update Success! Rebooting...");
    ESP.restart();
  } 
  else 
  { Serial.println("UpdateFromSD -> Error Occurred #: " + String(Update.getError()));
    return false;
  }

  return true;
}


uint8_t buff[8192];  


bool CheckIfNewer(const char *gstamp, const char* bstamp)
{ int g;
  int b;
  char months[12][4] = {{"Jan"}, {"Feb"}, {"Mar"}, {"Apr"}, {"May"}, {"Jun"}, {"Jul"}, {"Aug"}, {"Sep"}, {"Oct"}, {"Nov"}, {"Dec"}};
  Serial.printf("Check if Github Build Timestamp <%s> is newer than current build %s\n", gstamp, bstamp );
  // format is -> Jun 24 2026 - 10:03:29
  // check year
  g = (gstamp[7] - 0x30) * 1000;
  g += (gstamp[8] - 0x30) * 100;
  g += (gstamp[9] - 0x30) * 10;
  g += (gstamp[10] - 0x30);
  b = (bstamp[7] - 0x30) * 1000;
  b += (bstamp[8] - 0x30) * 100;
  b += (bstamp[9] - 0x30) * 10;
  b += (bstamp[10] - 0x30);
  Serial.printf("gy = %d by = %d\n", g, b);
  if(b>g)return false; // local version is newer
  if(b<g)return true; // local version is older
  // check month
  for(int m = 0; m<12; m++)
  { if(strncmp(gstamp, months[m], 3)==0)g=m;
    if(strncmp(bstamp, months[m], 3)==0)b=m;
  }
  Serial.printf("gm = %d bm = %d\n", g, b);
  if(b>g)return false; // local version is newer
  if(b<g)return true; // local version is older
  
  // check day, beware leading spaces 
  int g_tiental = (gstamp[4] == ' ') ? 0 : (gstamp[4] - 0x30);
  int b_tiental = (bstamp[4] == ' ') ? 0 : (bstamp[4] - 0x30);
  g = g_tiental * 10 + (gstamp[5] - 0x30);
  b = b_tiental * 10 + (bstamp[5] - 0x30);
  Serial.printf("gd = %d bd = %d\n", g, b);
  if(b>g)return false; // local version is newer
  if(b<g)return true; // local version is older
  
  // check hour
  g = (gstamp[14] - 0x30) * 10;
  g += (gstamp[15] - 0x30);
  b = (bstamp[14] - 0x30) * 10;
  b += (bstamp[15] - 0x30);
  Serial.printf("gh = %d bh = %d\n", g, b);
  if(b>g)return false; // local version is newer
  if(b<g)return true; // local version is older
  
  // check minute
  g = (gstamp[17] - 0x30) * 10;
  g += (gstamp[18] - 0x30);
  b = (bstamp[17] - 0x30) * 10;
  b += (bstamp[18] - 0x30);
  Serial.printf("gmin = %d bmin = %d\n", g, b);
  if(b>g)return false; // local version is newer
  if(b<g)return true; // local version is older
  
  if(b==g)return false; // running version is the same, no need to update
  
  return true;
}


bool DownloadGithubFile(char *url)
{ File sdfile;
  NetworkClientSecure client;
  HTTPClient http;
  uint32_t count = 0;
  bool fulldownload = false;

  // Bypass strict SSL certificate validation for simplicity
  client.setInsecure(); 
  
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  Serial.println("GitHub connect for download...");

  if(http.begin(client, url)) 
  { int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) 
    { int contentLength = http.getSize();
      Serial.printf("Github Firmware download size: %d bytes\n", contentLength);

      sdfile = SD.open(firmware_file, FILE_WRITE);

      // Read data from stream to save ESP32 heap memory
      NetworkClient* stream = http.getStreamPtr();

      // read all data from server
      while (http.connected() && (contentLength > 0 || contentLength == -1)) 
      { // get available data size
        size_t size = stream->available();

        if (size) 
        { // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          if(contentLength > 0) 
          { contentLength -= c;
            if(contentLength == 0)fulldownload = true;
          }
          count+=c;
          Serial.printf("Github Firmware downloaded sofar: %d bytes\n", count);
          // write it to Serial
          // Serial.write(buff, c);
          // write to SD card
          sdfile.write(buff,c);
        }
        delay(1);
      }  
      sdfile.close();
    } 
    http.end(); 
  }
 
  Serial.printf("Github Firmware downloaded: %d bytes\n", count);
  if(fulldownload)Serial.printf("Github Firmware downloaded: SUCCES!!!\n", count);
  return fulldownload;
}

bool GetTimeStampGitHubReleaseV2(char *url, char *timestamp)
{ NetworkClientSecure client;
  HTTPClient http;
  uint32_t count = 0;
  bool fulldownload = false;
  uint16_t idx = 0;
  bool build_label_found = false;
  bool build_label_complete = false;
  char found_timestamp[40];
  int n=0;
          

  // Bypass strict SSL certificate validation for simplicity
  client.setInsecure(); 
  
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  Serial.println("GitHub connect for download...");

  if(http.begin(client, url)) 
  { int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) 
    { int contentLength = http.getSize();
      Serial.printf("Github Firmware download size: %d bytes\n", contentLength);

      // Read data from stream to save ESP32 heap memory
      NetworkClient* stream = http.getStreamPtr();

      // read all data from server
      while (http.connected() && (contentLength > 0 || contentLength == -1) && !build_label_complete) 
      { // get available data size
        size_t size = stream->available();
        if (size) 
        { // read up to buffer size
          int cnt = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          if(contentLength > 0) 
          { contentLength -= cnt;
            if(contentLength == 0)fulldownload = true;
          }
          count+=cnt;

          // check for label
          n=0;
          while((n<cnt) && !build_label_complete)
          { uint8_t c = buff[n++];

            if(!build_label_complete)
            { if(!build_label_found)
              { //Serial.print(c);
                if(c==build_label[idx])
                { found_timestamp[idx]=c;
                  idx++; 
                  found_timestamp[idx]=0;
                  //Serial.println(found_timestamp);
                  if(idx == strlen(build_label))
                  { build_label_found = true;
                    Serial.printf("Github Build Label found\n");
                  }
                }
                else
                { idx = 0;
                }
              }  
              else // label found
              { // expand label
                found_timestamp[idx]=c;
                idx++;
                found_timestamp[idx]=0;
                if((strlen(found_timestamp) >= strlen(build_timestamp)) || c==0)
                { build_label_complete = true;
                  Serial.printf("Github Timestamp = %s\n", found_timestamp);
                  strcpy(timestamp, &found_timestamp[strlen(build_label)+1]);
                }
              }
            }
          }
          



          Serial.printf("Github Firmware downloaded sofar: %d bytes\n", count);
          // write it to Serial
          // Serial.write(buff, c);
        }
        delay(1);
      }  
    } 
    http.end(); 
  }
 
  Serial.printf("Github Label checked with downloaded: %d bytes\n", count);
  return build_label_complete;
}



