// update of firmware
// new files are located at: github
// steps to take
// - check most recent version
// - download to SD if newwer
// - install from SD
// - update build date in eeprom


//#include <WiFi.h>
//#include <HTTPClient.h>
//#include <NetworkClientSecure.h>

#include <HTTPClient.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <ESP_SSLClient.h>
#include <Update.h>

NetworkClient base_client;
ESP_SSLClient ssl_client;



uint16_t UpdateState = 0;

const char firmware_file[] = "/firmware.bin";

// Target file in Github release
char file_url[] = "https://github.com/Frank-Bemelman/radioglobe-esp32-2025/releases/download/RadioGlobe-Firmware/RadioGlobe-Globe.bin";
char path_url[] = "/Frank-Bemelman/radioglobe-esp32-2025/releases/download/RadioGlobe-Firmware/RadioGlobe-Globe.bin";


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

extern bool CheckForNewGlobeUpdate(void)
{ char GithubTimeStamp[32];
  if(!GetTimeStampGitHubReleaseV2(file_url, GithubTimeStamp))return false;
  if(!CheckIfNewer(GithubTimeStamp, build_timestamp_only))return false;
  return true;
}

uint8_t UpdateFirmware(uint8_t state)
{ static char GithubTimeStamp[32];
  File file;

  delay(1000); // make it a timer in loop()

  while(true)
  { Serial.printf("UpdateFirmware process state %d\n", state);
    switch(state)
    { case 0: // nothing to do
       return state;
       break;

     case 1:
       stream.stopSong();
       PixelUpdate(0, 0xFF0000, 0x000000, 10000); // solid red     
       AddToQueueForDisplay("Updating Globe", MESSAGE_STATION_NAME);
       AddToQueueForDisplay("MAY TAKE A MINUTE", MESSAGE_STATUS_LINE);
       AddToQueueForDisplay("Checking Github For New Firmware", MESSAGE_SONG_TITLE);
       AddToQueueForDisplay(GlobeSettings.ssid , MESSAGE_SSID_FOR_GLOBE);
       AddToQueueForDisplay(GlobeSettings.password, MESSAGE_PASSWORD_FOR_GLOBE);
       state++;
       return state;
       break;

       case 2:
       PixelUpdate(0, 0xFF0000, 0x000000, 10000); // solid red     
       if(!GetTimeStampGitHubReleaseV2(file_url, GithubTimeStamp))
       { // could not find a bin file with datestamp
         AddToQueueForDisplay("Continue With Puck", MESSAGE_STATION_NAME);
         AddToQueueForDisplay("No Globe Firmware On Github", MESSAGE_SONG_TITLE);
         AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
         PixelUpdate(0, 0x000000, 0x000000, 10000); // all off
         return 0;
       } 
       state++;
       return state;
       break;

     case 3:
       // sooner or later
       // check if github has a newer version than currently running
       PixelUpdate(0, 0xFF0000, 0x000000, 10000); // solid red     
       if(!CheckIfNewer(GithubTimeStamp, build_timestamp_only))
       { Serial.printf("Updater: Github %s is older than running version %s - quit update!\n", GithubTimeStamp, build_timestamp_only);
         AddToQueueForDisplay("", MESSAGE_STATUS_LINE);
         AddToQueueForDisplay("Globe Firmware Is The Latest", MESSAGE_SONG_TITLE);
         state  = 11; // continue with puck check
         PixelUpdate(0, 0x000000, 0x000000, 10000); // all off
         return state;
       }
       Serial.printf("Updater: Github %s is NEWER than running version %s - continue update!\n", GithubTimeStamp, build_timestamp_only);
       state++;
       return state;
       break;

     // last step in actual globe update, and puck is triggered
     case 4:
       PixelUpdate(4, 0xFF0000, 0x000000, 60000); // red rotating
       AddToQueueForDisplay("New Globe Firmware Downloading", MESSAGE_SONG_TITLE);
       AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
       //if(!DownloadGithubFileToSD(file_url))
       if(!startVolledigeOtaDownload("github.com", path_url)) // reboots on succes
       { AddToQueueForDisplay("Globe Firmware Download Failed", MESSAGE_SONG_TITLE);
         return 0; 
       }        
       break;
    

     // continue with puck only
     case 11: 
       AddToQueueForDisplay("", MESSAGE_UPDATE_PUCK);
       return 0; 
       
     default:
       state++;
       break;
    }
  }  
}


bool UpdateFromSD(const char *firmware_file)
{ File firmware = SD.open(firmware_file);
  
  Serial.printf("Updater: start flashing new firmware from SD card!\n");
  if (!firmware) 
  { Serial.printf("Failed to open %s\n", firmware_file);
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


bool DownloadGithubFileToSD(char *url)
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

      if(sdfile)
      { // Read data from stream to save ESP32 heap memory
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
  Serial.printf("GitHub connect to url: %s for download...", url);

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
  else
  { Serial.printf("Could not open Github url: %s\n", url); 
  }
 
  Serial.printf("Github Label checked with downloaded: %d bytes\n", count);
  return build_label_complete;
}


// =======================================================================
// FASE 2: DE ECHTE UPDATE (Volledig downloaden vanaf byte 0 naar Flash)
// =======================================================================
bool startVolledigeOtaDownload(String host, String path) {
    ssl_client.setClient(&base_client);
    ssl_client.setInsecure();
    heap_caps_malloc_extmem_enable(1024); 
    ssl_client.setBufferSizes(16384, 2048); 

    Serial.printf("[OTA] Verbinding maken voor complete download: %s...\n", host.c_str());
    if (!ssl_client.connect(host.c_str(), 443)) {
        Serial.println("[OTA] Verbinding mislukt.");
        return false;
    }

    ssl_client.print("GET " + path + " HTTP/1.1\r\n");
    ssl_client.print("Host: " + host + "\r\n");
    ssl_client.print("Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (ssl_client.available() == 0) {
        if (millis() - timeout > 10000) { ssl_client.stop(); return false; }
    }

    // Volg redirect ook in fase 2
    String firstLine = ssl_client.readStringUntil('\n');
    if (firstLine.indexOf("302") != -1 || firstLine.indexOf("301") != -1) {
        String newHost = "", newPath = "";
        while (ssl_client.available()) {
            String line = ssl_client.readStringUntil('\n');
            if (line.startsWith("Location: ") || line.startsWith("location: ")) {
                String fullUrl = line.substring(10); fullUrl.trim();
                if (fullUrl.startsWith("https://")) {
                    String urlWithoutProtocol = fullUrl.substring(8);
                    int firstSlash = urlWithoutProtocol.indexOf('/');
                    newHost = urlWithoutProtocol.substring(0, firstSlash);
                    newPath = urlWithoutProtocol.substring(firstSlash);
                }
                break;
            }
        }
        ssl_client.stop();
        if (newHost != "") {
            return startVolledigeOtaDownload(newHost, newPath);
        }
    }

    size_t bestandGrootte = 0;
    bool endOfHeaders = false;
    while (ssl_client.available() && !endOfHeaders) {
        String line = ssl_client.readStringUntil('\n');
        // Maak eerst een kopie in kleine letters
        String lowerLine = line;
        lowerLine.toLowerCase();
        
        // Controleer nu de gekopieerde regel
        if (lowerLine.startsWith("content-length:")) {
            bestandGrootte = line.substring(15).toInt();
        }
        line.trim();
        if (line.length() == 0) endOfHeaders = true;
    }

    if (bestandGrootte == 0) {
        Serial.println("[OTA] Fout: Geen Content-Length bekend.");
        ssl_client.stop();
        return false;
    }

    // Start nu pas de officiële Arduino Update-engine
    if (!Update.begin(bestandGrootte, U_FLASH)) {
        Serial.printf("[OTA] Fout bij starten engine: %s\n", Update.errorString());
        ssl_client.stop();
        return false;
    }

    Serial.println("[OTA] Schrijven naar flash-partitie gestart...");

    uint8_t* downloadBuffer = (uint8_t*) malloc(2048);
    size_t totaalBinnen = 0;
    unsigned long lastDataTime = millis();

    while (totaalBinnen < bestandGrootte) {
        while (ssl_client.available() == 0) {
            if (millis() - lastDataTime > 15000) {
                Serial.println("\n[OTA] Fout: Verbinding stilgevallen.");
                free(downloadBuffer); Update.end(); ssl_client.stop(); return false;
            }
            delay(1);
        }
        lastDataTime = millis();

        int teLezenBytes = min((size_t)2048, bestandGrootte - totaalBinnen);
        int bytesGelezen = ssl_client.read(downloadBuffer, teLezenBytes); 
        
        if (bytesGelezen > 0) {
            Update.write(downloadBuffer, bytesGelezen);
            totaalBinnen += bytesGelezen;
            
            if (totaalBinnen % 102400 == 0 || totaalBinnen == bestandGrootte) { 
                Serial.printf("[OTA] Voortgang: %d / %d bytes (%.1f%%)\n", totaalBinnen, bestandGrootte, (float)totaalBinnen / bestandGrootte * 100.0);
            }
        }
    }

    free(downloadBuffer);
    ssl_client.stop();

    if (Update.end(true)) {
        Serial.println("\n[OTA SUCCESS] Systeem succesvol bijgewerkt! Rebooten...");
        delay(1000);
        ESP.restart();
    } else {
        Serial.printf("[OTA FOUT] Validatie mislukt: %s\n", Update.errorString());
    }
    return false;
}

