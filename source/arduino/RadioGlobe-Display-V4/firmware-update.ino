// update of firmware
// new files are located at: github
// steps to take
// - check most recent version
// - download to SD if newwer
// - install from SD
// 
//#include <HttpsOTAUpdate.h>
//#include <Update.h>
//#include <Updater_Signing.h>


//#include <WiFi.h>
#include <HTTPClient.h>


#include <WiFi.h>
#include <NetworkClient.h>
#include <ESP_SSLClient.h>
#include <Update.h>

NetworkClient base_client;
ESP_SSLClient ssl_client;
String serverTimestamp = "";


//#include <esp_now.h>

uint16_t UpdateState = 0;
char Wifi_SSID[64] = "WNAP10";
char Wifi_PASSWORD[64] ="ALIBABA4711";

const char firmware_file[] = "/firmware.bin";

// Target file in Github release
char file_url[] = "https://github.com/Frank-Bemelman/radioglobe-esp32-2025/releases/download/RadioGlobe-Firmware/RadioGlobe-Display.bin";
// Prototype om te kunnen schakelen

uint8_t UpdateFirmware(uint8_t state)
{ char GithubTimeStamp[32];
  File file;

  //if(strlen(Wifi_SSID)==0)return 0;
  Serial.printf("Start UpdateFirmware process with state %d\n", state);

  while(true)
  { Serial.printf("UpdateFirmware process state %d\n", state);
    switch(state)
    { case 0: // nothing to do
       return state;
       break;

     case 1:
       lv_label_set_text(ui_Station_Name, "Updating Puck");
       lv_label_set_text(ui_Status_Line, "RESTART MAY TAKE 3 MINUTES");
       lv_label_set_text(ui_Station_Title, "Checking For New Puck Firmware");
       Lvgl_Loop();
       delay(2000);

       if(strlen(Wifi_SSID))StartWifi();
       else 
       { lv_label_set_text(ui_Station_Title, "No Wifi Credentials - QUIT");
         Lvgl_Loop();
         delay(2000);
         lv_label_set_text(ui_Station_Name, "");
         lv_label_set_text(ui_Status_Line, "");
         lv_label_set_text(ui_Station_Title, "");
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_turquoise);
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_turquoise);
         lv_obj_set_style_bg_color(ui_SerialNumberButton, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_opa(ui_SerialNumberButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
         return 0;
       } 

       if(WiFi.isConnected())
       { Serial.printf("Updater: we have a wifi\n");
         lv_label_set_text(ui_Station_Title, "Wifi Connected");
         Lvgl_Loop();
         delay(2000);
       }
       else
       { Serial.printf("Updater: NO wifi\n");
         lv_label_set_text(ui_Station_Title, "No Wifi Connection - STOPPED");
         lv_label_set_text(ui_Status_Line, "");
         Lvgl_Loop();
         delay(2000);
         lv_label_set_text(ui_Station_Name, "");
         lv_label_set_text(ui_Station_Title, "");
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_turquoise);
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_turquoise);
         lv_obj_set_style_bg_color(ui_SerialNumberButton, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_opa(ui_SerialNumberButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);         
         return 0;
       }

       if(!checkFirmwareVersie("github.com", "/Frank-Bemelman/radioglobe-esp32-2025/releases/download/RadioGlobe-Firmware/RadioGlobe-Display.bin"))
       { WiFi.disconnect();
         lv_label_set_text(ui_Station_Title, "Puck Firmware Is The Latest");
         lv_label_set_text(ui_Status_Line, "");
         Lvgl_Loop();
         delay(2000);
         lv_label_set_text(ui_Station_Name, "");
         lv_label_set_text(ui_Station_Title, "");
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_turquoise);
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_turquoise);
         lv_obj_set_style_bg_color(ui_SerialNumberButton, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_opa(ui_SerialNumberButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);         
         return 0;
       } 
       state++;
       break;

     case 2:
       lv_label_set_text(ui_Station_Title, "Downloading New Puck Firmware..");
       Lvgl_Loop();
       delay(2000);
       state++;
       break;

     case 3:
       lv_label_set_text(ui_Station_Title, "Update Puck ESP32 Processor..");
       Lvgl_Loop();
       loop_esp_now(); // last round
       delay(1000);
       BacklightValue = 0; // screen goes bonkers while updating flash - no need to upset ourselves with that
       Set_Backlight(BacklightValue); // screen goes bonkers while updating flash - no need to upset ourselves with that
       Serial.printf("Backlight Off during firmware flash\n");
       delay(500);
       state++;
       break;

     case 4:
       BacklightValue = 0; // screen goes bonkers while updating flash - no need to upset ourselves with that
       Set_Backlight(BacklightValue); // screen goes bonkers while updating flash - no need to upset ourselves with that
       delay(500);
       state++;
       break;

     case 5:
       // reboot included when succes
       if(!startVolledigeOtaDownload("github.com", "/Frank-Bemelman/radioglobe-esp32-2025/releases/download/RadioGlobe-Firmware/RadioGlobe-Display.bin"))
       { BacklightValue = DEFAULT_BACKLIGHT;
         lv_label_set_text(ui_Status_Line, "");
         lv_label_set_text(ui_Station_Title, "Update Puck Failed!!");
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR, _ui_theme_color_turquoise);
         ui_object_set_themeable_style_property(ui_SettingButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA, _ui_theme_alpha_turquoise);
         lv_obj_set_style_bg_color(ui_SerialNumberButton, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_opa(ui_SerialNumberButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);         
       }
       state++;
       break;

     case 6:
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

void StartWifi(void)
{ //disableESPNOW();
  //Serial.println("ESPNOW disabled");
  
  Serial.println("StartWifi()");
  
  WiFi.mode(WIFI_STA);
  
  delay(100);
  
  Serial.println(WiFi.localIP());
  WiFi.begin(Wifi_SSID, Wifi_PASSWORD); 
  Serial.println("Connecting to WiFi");
  delay(1000);
  uint16_t timeout = 200; // 15 seconds max
  while (!WiFi.isConnected() && timeout--) 
  { Serial.print('.');
    delay(100);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
  
  delay(10);
  Serial.print("Vrij RAM geheugen : ");
  Serial.println(ESP.getFreeHeap());
  Serial.printf("Totale PSRAM: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Vrije PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("Grootste vrije aaneengesloten blok RAM is: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}






// =======================================================================
// FASE 1: ALLEEN CONTROLEREN (Download maximaal 4KB en breek daarna af)
// =======================================================================
bool checkFirmwareVersie(String host, String path) 
{   ssl_client.setClient(&base_client);
    ssl_client.setInsecure();
    heap_caps_malloc_extmem_enable(1024); 
    ssl_client.setBufferSizes(16384, 2048); 

    Serial.printf("[CHECK] Verbinden met %s voor timestamp check...\n", host.c_str());
    if (!ssl_client.connect(host.c_str(), 443)) {
        Serial.println("[CHECK] Verbinding mislukt.");
        return false;
    }

    ssl_client.print("GET " + path + " HTTP/1.1\r\n");
    ssl_client.print("Host: " + host + "\r\n");
    ssl_client.print("Connection: close\r\n\r\n");

    // Wacht op data
    unsigned long timeout = millis();
    while (ssl_client.available() == 0) {
        if (millis() - timeout > 10000) { ssl_client.stop(); return false; }
    }

    // Handhaaf de GitHub Redirect (302)
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
            return checkFirmwareVersie(newHost, newPath); // Volg de redirect voor de check;
        }
    }

    // Sla de HTTP headers van de downloadserver over
    bool endOfHeaders = false;
    while (ssl_client.available() && !endOfHeaders) {
        String line = ssl_client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) endOfHeaders = true;
    }

    // Start het scannen van de eerste bytes in een tijdelijke array van 4KB op de heap.
    size_t scanLimit = 4096;
    uint8_t* scanBuffer = (uint8_t*) malloc(scanLimit);
    size_t scanIndex = 0;
    unsigned long lastDataTime = millis();

    while (scanIndex < scanLimit && ssl_client.connected()) {
        while (ssl_client.available() == 0) {
            if (millis() - lastDataTime > 5000) break;
            delay(1);
        }
        if (ssl_client.available() > 0) {
            scanBuffer[scanIndex++] = ssl_client.read();
            lastDataTime = millis();
        }
    }

    // Gooi de netwerkverbinding direct dicht. We hebben de benodigde kilobytes binnen!
    ssl_client.stop();

    // Zoek naar het "Puck Build" label in de binnengehaalde 4KB
    String matchTarget = BUILD_LABEL;
    int targetLen = matchTarget.length();
    int matchIndex = 0;
    int gevondenAt = -1;

      

    for (size_t i = 0; i < scanIndex; i++) {
        if (scanBuffer[i] == matchTarget[matchIndex]) {
            matchIndex++;
            if (matchIndex == targetLen) {
                // CORRECTIE: i is de index van de allerlaatste letter ('d' van Build)
                // De timestamp begint EXACT op de positie i + 1 (de spatie) of i + 2 (de 'J')
                gevondenAt = i + 1; 
                break;
            }
        } else {
            matchIndex = (scanBuffer[i] == matchTarget[0]) ? 1 : 0;
        }
    }

    bool firmwareIsNieuw = true;
    
    if (gevondenAt != -1) {
        serverTimestamp = "";
        
        // We lezen 23 bytes uit om eventuele voorloopspaties netjes te vangen
        for (int i = 0; i < 23; i++) {
            if ((gevondenAt + i) < scanIndex) {
                serverTimestamp += (char)scanBuffer[gevondenAt + i];
            }
        }
        
        // Haal onnodige spaties aan het begin en einde weg
        serverTimestamp.trim(); 

        Serial.printf("[CHECK] Server build: '%s'\n", serverTimestamp.c_str());
        Serial.printf("[CHECK] Lokale build: '%s'\n", BUILD_TIMESTAMP_ONLY);

        
    } else {
        Serial.println("[CHECK] Waarschuwing: Geen timestamp gevonden in eerste 4KB. Update wordt nu gestart.");
    }

    free(scanBuffer); // Ruim de tijdelijke scan-array direct op uit de heap

    // FASE 2 triggeren indien goedgekeurd
    if(CheckIfNewer(serverTimestamp.c_str(), BUILD_TIMESTAMP_ONLY))
    {
        Serial.println("[CHECK] Nieuwe firmware gedetecteerd!");
        return true;
        // Omdat we de originele host en path nodig hebben (github.com), roep je deze functie aan
        // met je oorspronkelijke startgegevens.
        //startVolledigeOtaDownload("github.com", "/jouw-username/jouw-repo/releases/download/v1.0/firmware.bin");
    }
    else Serial.println("[CHECK] Github firmware ouder of gelijk...");
    return false;
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







