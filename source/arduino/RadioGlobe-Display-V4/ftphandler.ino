// FTP sever, to be able to use Filezilla for reading/writing SD card content
// Start the FTP sever in the setup menu with secret code BOBOB

FtpServer ftp;
bool bFtpActive = false;
uint16_t FtpBootState = 0;





uint16_t HandleFtpBootState(uint16_t FtpBootState)
{ char safeBuffer[64]; // Fixed size buffer for texts
  IPAddress myIP = WiFi.localIP();

  while(true)
  { switch(FtpBootState)
    { case 0: // nothing to do
        return FtpBootState;
        break;

      case 1:
        //esp_now_deinit();
        if(Puck_SD_GB == 0)
        { lv_label_set_text(ui_TextSecretCodeToUnlock, "NO SD CARD");
          delay(100);
          Lvgl_Loop();
          delay(2000);
          FtpBootState = 10;
          return FtpBootState;
        }  
        lv_obj_add_flag(ui_HomeButton4, LV_OBJ_FLAG_HIDDEN); // hide home button
        FtpBootState++;
        return FtpBootState;
        break;
     
      case 2: // start wifi
        Serial.printf("HandleFtpBootState %d\n", FtpBootState);
        lv_label_set_text(ui_TextSecretCodeToUnlock, "FTP SERVER STARTING");
        Lvgl_Loop();
        delay(2000);
        Serial.printf("SSID=%s PW=%s\n", Wifi_SSID, Wifi_PASSWORD);
        if(strlen(Wifi_SSID))StartWifi();
        else 
        { lv_label_set_text(ui_TextSecretCodeToUnlock, "NO WIFI CREDENTIALS");
          delay(100);
          Lvgl_Loop();
          delay(2000);
          FtpBootState = 10;
          return FtpBootState;
        } 
        FtpBootState++;
        return FtpBootState;
        break;

      case 3: // check wifi  
        Serial.printf("HandleFtpBootState %d\n", FtpBootState);
        if(WiFi.isConnected())
        { Serial.printf("HandleFtpBootState: we have a wifi\n");
          lv_label_set_text(ui_TextSecretCodeToUnlock, "WIFI CONNECTED");
          delay(100);
          Lvgl_Loop();
          delay(2000);
        }
        else
        { Serial.printf("HandleFtpBootState: NO wifi\n");
          lv_label_set_text(ui_TextSecretCodeToUnlock, "NO WIFI CONNECTION");
          delay(100);
          Lvgl_Loop();
          delay(2000);
          lv_obj_clear_flag(ui_HomeButton4, LV_OBJ_FLAG_HIDDEN); // show home icon
          FtpBootState = 10;
          return FtpBootState;
        }
        FtpBootState++;
        return FtpBootState;
        break;

      case 4: // FTP START
        Serial.printf("HandleFtpBootState %d\n", FtpBootState);
        ftp.begin("globe", "globe");
        Serial.println("FTP gestart!");
        bFtpActive = true;
        myIP = WiFi.localIP();
        snprintf(safeBuffer, sizeof(safeBuffer), "USE FTP AT %d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
        lv_label_set_text(ui_TextSecretCodeToUnlock, safeBuffer);
        FtpBootState++;
        return FtpBootState;
        break;  

      case 5: // stick here as long as FTP is active
        return FtpBootState;
        break;

      case 6: // close FTP in steps
        Serial.printf("HandleFtpBootState %d\n", FtpBootState);
        lv_label_set_text(ui_TextSecretCodeToUnlock, "FTP SERVER CLOSING");
        Lvgl_Loop();
        delay(2000);
        WiFi.disconnect();
        FtpBootState++;
        return FtpBootState;
        break;

      case 10: // closing everything, all back to normal
        Serial.printf("HandleFtpBootState %d\n", FtpBootState);
        lv_obj_clear_flag(ui_HomeButton4, LV_OBJ_FLAG_HIDDEN); // show home icon
        lv_label_set_text(ui_TextSecretCodeToUnlock, "SECRET CODE TO UNLOCK");
        //setup_esp_now();
        bFtpActive = false;
        FtpBootState = 0;
        return FtpBootState;
        break;  
      
      default:
        FtpBootState++;
        return FtpBootState;
        break;  
    }   
  }
  return FtpBootState;
}