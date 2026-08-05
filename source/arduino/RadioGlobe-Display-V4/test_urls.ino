// the idea is to test urls from the database while the globe is turned off (asleep)
// periodically called when power is off
// starting at N coordinate 90, and walk through W180-E180
// moving down to S 90
// tests one station at the time

// uses AddToQueueForDisplay("https://somestation.mp3", MESSAGE_TEST_URL);
// that will log to /tested-urls.log if the url was problematic
// later, I can use that file to clean up my database manually

// first call, if no /tested-urls.log is present, reset NS/EW to start with (90, -180) and store
// create /tested-urls.log
//
// 
// next call, read content of NS/EW map on SD card, if any, write to new file /test-urls-to-do.txt
// next call, pick first from /test-urls-to-do.txt and test it
// next call, pick next one from that file and test it
// next call, when all done, increment NS/EW and store it, delete /test-urls-to-do.txt
// rinse and repeat
//
// when all done, do nothing

bool bUrlTestRunning = false;
static uint32_t Url_Test_State = 0;
// prepare puck for tests during power off
// start with secret code ELGEL 
void url_test_init(void)
{ if(Puck_SD_GB)
  { SD_MMC.remove("/tested-urls.log");
    // start at the north pole and work our way downwards to the south pole
    DisplaySettings.testNS = 90;
    DisplaySettings.testEW = -180;
    SaveDisplaySettings();
    AppendToLogFile("/tested-urls.log", "Test of Urls Started!");
    Url_Test_State = 0;
  }
}


void loop_testurl(void)
{ static File root;
  File file;
  char readbuffer[128];
  size_t bytesread;
  char *p;
  static uint32_t filenumber_to_open;
  static uint32_t urlnumber_to_open;
  static uint32_t filenumber_actual;
  static uint32_t urlnumber_actual;
  static char dirpath[64];
  static char safetybuf[128];

  if(bPowerStatus || bUrlTestRunning)
  { return; // not now, only when asleep we test
  }

  // one step at the time...
  //Serial.printf("Test state = %d\n", test_state);
  switch(Url_Test_State)
  { case 0:
      // test presence of /tested-urls.log
      strcpy(safetybuf, "keep it safe");
      if(Puck_SD_GB)
      { if(!SD_MMC.exists("/tested-urls.log"))
        { Url_Test_State = 10;
          break;
        }
      }
      else 
      { Url_Test_State=10;
        break;
      }
      
      // test to force certain start point
      // DisplaySettings.testNS = 42;
      // DisplaySettings.testEW = -120;

      Url_Test_State++;
      break;
    
    case 1:  
      Serial.printf("Let's try NS%d - EW%d\n", DisplaySettings.testNS, DisplaySettings.testEW);
      // read map 
      if(Puck_SD_GB)
      { snprintf(dirpath, sizeof(dirpath), "/%c/%d/%d/%c/%d/%d", (DisplaySettings.testNS<0)?'S':'N', abs(DisplaySettings.testNS)/10, abs(DisplaySettings.testNS)%10, (DisplaySettings.testEW<0)?'W':'E', abs(DisplaySettings.testEW)/10, abs(DisplaySettings.testEW)%10);
        root = SD_MMC.open(dirpath);
        if (!root) 
        { //Serial.printf("Failed to open directory %s\n", dirpath);
          DisplaySettings.testEW++;
          if(DisplaySettings.testEW>180)
          { DisplaySettings.testEW = -180;
            DisplaySettings.testNS--;
            if(DisplaySettings.testNS<-90)
            { Url_Test_State = 10;
            }
          }
        }
        else 
        { Url_Test_State++;
        }
      }
      break;

    case 2:
      Serial.printf("Directory %s succesfully opened\n", dirpath);
      SaveDisplaySettings(); // store DisplaySettings.testNS and DisplaySettings.testEW in case we have to resume later
      filenumber_actual = 0;
      urlnumber_actual = 0;
      filenumber_to_open = 0;
      urlnumber_to_open = 0;
      Url_Test_State++;
      break;  

    case 3:
      // find filenumber number X and url number X and test it
      Serial.printf("Check file %d and url %d in directory %s\n", filenumber_to_open, urlnumber_to_open, dirpath);
     
      if(Puck_SD_GB)
      { root = SD_MMC.open(dirpath);
        filenumber_actual = 0;
        if (root) 
        { file = root.openNextFile();
          while(file)
          { if(!file.isDirectory())
            { if(filenumber_actual == filenumber_to_open)
              { Serial.printf("Now read file %d with url %d to test\n", filenumber_to_open, urlnumber_to_open);
                urlnumber_actual = 0;
                while(file.available())
                { bytesread = file.readBytesUntil('\n', readbuffer, sizeof(readbuffer)-1);
                  readbuffer[bytesread]=0;
                  //Serial.println(readbuffer);
                  if(p = strrchr(readbuffer, '\"'))*p=0; // get rid of the last "
                  if(p = strstr(readbuffer, "\"url\": \""))
                  { // example:  "url": "https://stream.radio-fratz.de/stream_high.mp3
                    p+=8; // jump forward to start of url
                    if(urlnumber_actual == urlnumber_to_open)
                    { Serial.printf("Url %d to test now -> %s\n", urlnumber_to_open, p);
                      bUrlTestRunning = true;
                      AddToQueueForGlobe(p, MESSAGE_TEST_URL); 
                      urlnumber_to_open++;
                      file.close();  
                      root.close();  
                      return;
                    }
                    urlnumber_actual++;
                  }  
                } 
                filenumber_to_open++;   
                urlnumber_to_open = 0; 
                file.close();  
                root.close();  
                return;            
              }
              filenumber_actual++;
            }
            file.close(); 
            file = root.openNextFile();
            
          }   
          // last file handled, next one
          root.close();
          DisplaySettings.testEW++;
          if(DisplaySettings.testEW>180)
          { DisplaySettings.testEW = -180;
            DisplaySettings.testNS--;
            if(DisplaySettings.testNS<-90)
            { Url_Test_State = 10;
            }
          }
          Serial.print("datetime.hour = ");
          Serial.println(datetime.hour);
          snprintf(dirpath, sizeof(dirpath), "/%c/%d/%d/%c/%d/%d", (DisplaySettings.testNS<0)?'S':'N', abs(DisplaySettings.testNS)/10, abs(DisplaySettings.testNS)%10, (DisplaySettings.testEW<0)?'W':'E', abs(DisplaySettings.testEW)/10, abs(DisplaySettings.testEW)%10);
          Serial.print("Next Directory to tackle = ");
          Serial.print("datetime.hour = ");
          Serial.println(datetime.hour);
          Serial.println(dirpath);
          Url_Test_State = 1;
        }
        SaveDisplaySettings();
      }
      else Url_Test_State = 10;
      break;  

    case 10:
      // we're done
      bUrlTestRunning = false;

      break;  
     
    default:
      break;
  }
}

// prior to rebuilding database directory tree, it's a good idea to get rid of the current directories and content
// after that, the database can be rebuild, without orphan station.txt files in the way
void RemoveNSdirectories(void)
{ if(Puck_SD_GB)
  { if(!SD_MMC.exists("/stations150K.json"))
    { Serial.printf("Missing: /stations150K.json -> Remove N and S directories\n");
      DeleteDir(SD_MMC, "/N");
      DeleteDir(SD_MMC, "/S");
    }  
  }  
}

// tradiitonal remove of directory tree
// could probably made faster if we use the worldmap with the dots to find them
bool DeleteDir(fs::FS &fs, const char *dirname) 
{ File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory, can't delete");
    return false;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory, can't delete as directory");
    root.close();
    return false;
  }

  File file = root.openNextFile();
  while (file) 
  { if (file.isDirectory()) 
    { Serial.print("  DIR : ");
      Serial.println(file.name());
      char path[256];
      strcpy(path, file.path());
      file.close();
      char message[64];

      snprintf(message, sizeof(message), "Deleting %s", path);
      lv_label_set_text(ui_Station_Name, message);
      Lvgl_Loop();

      if (!DeleteDir(fs, path)) 
      { root.close();
        return false; // quit when directory could not be removed
      }
    } 
    else // a file to remove
    { Serial.print("REMOVE FILE: ");
      Serial.println(file.name());
      fs.remove(file.path());
      file.close();
    }
    file = root.openNextFile();
  }
  root.close();
  return(fs.rmdir(dirname)); 

}



