/**
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for ondemand configportal, hold for 3 seconds for reset settings.
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define PORTALSWITCH_PIN 20 // pushbutton located on the back, short press  opens portal to connect to smartphone and configure wifi and puck mac, long press > 3 seconds is resets,all settings

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false; // change to true to use non blocking

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )
WiFiManagerParameter custom_mqtt; // global param ( for non blocking w params )
WiFiManagerParameter custom_puck_mac; // global param ( for non blocking w params )
char currentpuckmac[17];
char newpuckmac[17];


void AutoConnectSetup(void) 
{ char content[64]; 
  Serial.println("AutoConnectSetup() Starting");

  CreateHostName();

  WiFi.mode(WIFI_MODE_NULL); 
  delay(100);

  pinMode(PORTALSWITCH_PIN, INPUT_PULLUP); // input to button for opening portal
  
  //wm.resetSettings(); // wipe settings - for test purposes 

  if(wm_nonblocking) wm.setConfigPortalBlocking(false);

  // add a custom input field
  int customFieldLength = 40;


  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");
  
  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type
  
  // test custom html(radio)
  //const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><br><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  //new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input
  //wm.addParameter(&custom_field);
  
  wm.setSaveParamsCallback(saveParamCallback);

 // Add HA MQTT server IP parameter om te koppelen voor volume, aan/uit
  sprintf(content, "%d.%d.%d.%d", (uint16_t)GlobeSettings.ee_mqttserver_ip[0], (uint16_t)GlobeSettings.ee_mqttserver_ip[1], (uint16_t)GlobeSettings.ee_mqttserver_ip[2], (uint16_t)GlobeSettings.ee_mqttserver_ip[3]);
  new (&custom_mqtt) WiFiManagerParameter("custommqttid", "<br>HA MQTT SERVER IP", content, 15, "pattern='\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}'"); 
  wm.addParameter(&custom_mqtt);
 
 // Add Puck Mac parameter to connect to
 sprintf(currentpuckmac, "%02X:%02X:%02X:%02X:%02X:%02X",
                  PuckMac[0], PuckMac[1], PuckMac[2],
                  PuckMac[3], PuckMac[4], PuckMac[5]);
  new (&custom_puck_mac) WiFiManagerParameter("custompuckmacid", "<br>PUCK MAC ADDRESS", currentpuckmac, 17, "pattern='\\a{2}\\:\\a{2}\\:\\a{2}\\:\\a{2}\\:\\a{2}\\:\\a{2}'"); 
  wm.addParameter(&custom_puck_mac);
 
 
  // custom menu via array or vector
  // 
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  //std::vector<const char *> menu = {"wifi","info","param","sep","restart"};
  
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");


  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always

  wm.setConnectTimeout(10); // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(120); // auto close configportal after n seconds
  //wm.setCaptivePortalEnable(false); // disable captive portal redirection (auto redirection on smartphone)
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons
  
  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails
  

  
  wm.setHostname(HostName); 
  Serial.printf("Hostname set at %s\n",HostName);

  
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  Serial.setDebugOutput(true);  
  delay(500);
  
}

void CreateHostName(void)
{ uint64_t EfuseMac = ESP.getEfuseMac(); // gives us full mac address
  EfuseMac = (EfuseMac & 0x00000000FFFFFFFF) << 32 | (EfuseMac & 0xFFFFFFFF00000000) >> 32;
  EfuseMac = (EfuseMac & 0x0000FFFF0000FFFF) << 16 | (EfuseMac & 0xFFFF0000FFFF0000) >> 16;
  EfuseMac = (EfuseMac & 0x00FF00FF00FF00FF) << 8  | (EfuseMac & 0xFF00FF00FF00FF00) >> 8;
  EfuseMac = EfuseMac >> 16;
  //sprintf(HostName, "RADIOGLOBE-%12llX-%02d", EfuseMac, GlobeSettings.serialnumber);
  sprintf(HostName, "radioglobe-%02d", GlobeSettings.serialnumber); // easier to remember
}


void AutoConnect(void) 
{ String ssid;
  String password;
  char portalName[32];

  sprintf(portalName,"RadioGlobe%d", GlobeSettings.serialnumber);

  Serial.println("AutoConnect() - AutoConnect Starting");

  bool res;
  res = wm.autoConnect(portalName); // anonymous ap, SSID is RadioGlobe50 or higher
  //  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) 
  { Serial.println("AutoConnect()Failed to connect or hit timeout");
//    PlaySoundBite((uint8_t *)mp3_wifidisconnected, sizeof(mp3_wifidisconnected)); 
    delay(2000);
    Serial.println("AutoConnect() - Starting config portal");
    wm.setConfigPortalTimeout(120);
    PlaySoundBite((uint8_t *)mp3_smartphone_portal, sizeof(mp3_smartphone_portal), 50); 
    if(!wm.startConfigPortal(portalName, ""))
    {  delay(5000);
       ESP.restart();
    }
  } 

  //if you get here you have connected to the WiFi    
  Serial.println("AutoConnect() - connected...yeey :)");
  PlaySoundBite((uint8_t *)mp3_wificonnected, sizeof(mp3_wificonnected), 50); 
  
  ssid =wm.getWiFiSSID();
  password = wm.getWiFiPass();
  Serial.println(ssid);
  Serial.println(password);
  if( (strcmp(GlobeSettings.ssid, ssid.c_str()  )!=0) || (strcmp(GlobeSettings.password, password.c_str() )!=0) )
  { strcpy(GlobeSettings.ssid, ssid.c_str());
    strcpy(GlobeSettings.password, password.c_str());
    EEPROM.put(0x0, GlobeSettings);
    EEPROM.commit();
    Serial.println("AutoConnect() - SSID and password saved to EEprom.");
  } 
}

extern void killwebserver(void); 
void checkButton(bool force)
{ String ssid;
  String password;
  char portalName[32];

  sprintf(portalName,"RadioGlobe%d", GlobeSettings.serialnumber);

  // check for button press
  if(digitalRead(PORTALSWITCH_PIN) == LOW)Serial.println("checkButton() - Fysical Portal Button Pressed");
  if(force)Serial.println("checkButton() - Virtual Portal Button Pressed (force)");
  
  if ( digitalRead(PORTALSWITCH_PIN) == LOW || force==true) 
  { // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if( digitalRead(PORTALSWITCH_PIN) == LOW || force==true)
    { // still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // reset delay hold
      if( digitalRead(PORTALSWITCH_PIN) == LOW )
      { Serial.println("checkButton() - Button Held");
        Serial.println("checkButton() - Erasing Config, restarting");
        wm.resetSettings();
        ESP.restart();
      }
      
      killwebserver(); 
      // start portal w delay
      Serial.println("checkButton() - Starting config portal");
      wm.setConfigPortalTimeout(120);
      PlaySoundBite((uint8_t *)mp3_smartphone_portal, sizeof(mp3_smartphone_portal), 50); 

      
      // if (!wm.startConfigPortal("OnDemandAP","password")) {
      if (!wm.startConfigPortal(portalName, "")) 
      { Serial.println("checkButton() - failed to connect or hit timeout");
        delay(3000);
        // ESP.restart();
      } else 
      { //if you get here you have connected to the WiFi
        Serial.println("checkButton() - connected...yeey :)");
        PlaySoundBite((uint8_t *)mp3_wificonnected, sizeof(mp3_wificonnected), 50); 
        ssid =wm.getWiFiSSID();
        password = wm.getWiFiPass();
        Serial.println(ssid);
        Serial.println(password);
        if( (strcmp(GlobeSettings.ssid, ssid.c_str()  )!=0) || (strcmp(GlobeSettings.password, password.c_str() )!=0) )
        { strcpy(GlobeSettings.ssid, ssid.c_str());
          strcpy(GlobeSettings.password, password.c_str());
          EEPROM.put(0x0, GlobeSettings);
          EEPROM.commit();
          Serial.println("checkButton() - SSID and password saved to EEprom.");
          PlaySoundBite((uint8_t *)mp3_rebooting_radioglobe, sizeof(mp3_rebooting_radioglobe), 50); 
          delay(2000);
          ESP.restart();
        }         
      }
      AddToQueueForDisplay("checkButton() - Globe Closed Portal", MESSAGE_GLOBE_CLOSED_PORTAL);
    }
  }
}


String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback(){
  unsigned int ip4address[4];
  unsigned int puckmac[6];
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
  Serial.println("PARAM custommqttid = " + getParam("custommqttid"));
  Serial.println("PARAM custompuckmacid = " + getParam("custompuckmacid"));
  //memcpy(newpuckmac, getParam("custompuckmacid").c_str(), 17);

  ip4address[0] = 0; ip4address[1] = 0; ip4address[2] = 0; ip4address[3] = 0;
  sscanf(getParam("custommqttid").c_str(), "%d.%d.%d.%d", &ip4address[0], &ip4address[1], &ip4address[2], &ip4address[3]);
  Serial.printf("ip4address[0] = %d\n", ip4address[0]);
  Serial.printf("ip4address[1] = %d\n", ip4address[1]);
  Serial.printf("ip4address[2] = %d\n", ip4address[2]);
  Serial.printf("ip4address[3] = %d\n", ip4address[3]);

  //if(ip4address[0] && ip4address[1]  && ip4address[2] && ip4address[3])
  { GlobeSettings.ee_mqttserver_ip[0] = ip4address[0];
    GlobeSettings.ee_mqttserver_ip[1] = ip4address[1];
    GlobeSettings.ee_mqttserver_ip[2] = ip4address[2];
    GlobeSettings.ee_mqttserver_ip[3] = ip4address[3];
    Serial.printf("Set in EEprom space\n");
  }


  sscanf(getParam("custompuckmacid").c_str(), "%02X:%02X:%02X:%02X:%02X:%02X", &puckmac[0], &puckmac[1], &puckmac[2], &puckmac[3], &puckmac[4], &puckmac[5]);
  GlobeSettings.ee_puckmac[0] = puckmac[0];
  GlobeSettings.ee_puckmac[1] = puckmac[1];
  GlobeSettings.ee_puckmac[2] = puckmac[2];
  GlobeSettings.ee_puckmac[3] = puckmac[3];
  GlobeSettings.ee_puckmac[4] = puckmac[4];
  GlobeSettings.ee_puckmac[5] = puckmac[5];
  

  EEPROM.put(0x0, GlobeSettings);
  EEPROM.commit();
  Serial.println("Eeprom saved..");

}

void loop2() 
{
  if(wm_nonblocking) wm.process(); // avoid delays() in loop when non-blocking and other long running code  
  checkButton(false); // button on back of globe, use paperclip to activate and open wifi portal 
  // put your main code here, to run repeatedly:
}
