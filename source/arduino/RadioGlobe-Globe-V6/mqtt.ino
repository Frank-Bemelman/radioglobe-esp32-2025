// Code below handles communication with Home Assistant (famous home automation software)
// library -> manage libraries -> home-assistant-integration Version 2.1.0
#include <ArduinoHA.h>

unsigned long lastSentAt = millis();
HADevice device(WiFi.getHostname()); // -> something like RADIOGLOBE-54
WiFiClient HAclient; 
HAMqtt mqtt(HAclient, device);

void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length)
{ // This callback is called when message from MQTT broker is received.
  // Please note that you should always verify if the message's topic is the one you expect.
  // For example: if (strstr(topic, "myCustomTopic") == 0) { ... }
  char payloadcopy[256];
  char message[QUEUEMESSAGELENGTH]; 
  char *p;

  snprintf(payloadcopy, sizeof(payloadcopy), "%.*s", length, (const char*)payload);
  
  Serial.print("New message on topic: ");
  Serial.println(topic);
  Serial.print("Data: ");
  Serial.println(payloadcopy);
  
  unsigned int poweron = 0;
  unsigned int volume = 0;
  
  if(strstr(topic, "GlobeSpeakerSwitch" )!=NULL)
  { GlobeSettings.ee_internal_speakers = strcmp(payloadcopy, "OFF")==0 ? 0:1;
    digitalWrite(MUTE_AMPLIFIERS, strcmp(payloadcopy, "OFF")==0 ? 1:0); // mute output works as a disable
    sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
    AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
  }
  else if(strstr(topic, "GlobeBTSwitch" )!=NULL)
  { GlobeSettings.btmodule_power_on = strcmp(payloadcopy, "OFF")==0 ? 0:1;
    sprintf(message, "%d-%d-%d", GlobeSettings.btmodule_switchable, GlobeSettings.btmodule_power_on, GlobeSettings.btmodule_installed);
    AddToQueueForDisplay(message, MESSAGE_DISPLAY_BT_SWITCHABLE_STATE);
  }
  else if(strstr(topic, "GlobeVolume" )!=NULL)
  { if(!IgnoreMqqtUpdates) // ignore updates that may lag the setting done by truss touch
    { sscanf((const char*)payload, "%d", &volume);
      DataFromGlobe.G_Volume = volume;
    }  
  }
  else if((p=strstr(topic, "GlobePreset")) != NULL)
  { if(bPowerStatus)
    { sscanf(p, "GlobePreset%d", &PresetRequestFromHA);
      Serial.printf("p= %s -> PresetRequestFromHA = %d\n", p, PresetRequestFromHA);
      sprintf(message, "%d", PresetRequestFromHA);
      AddToQueueForDisplay(message, MESSAGE_STATION_PRESET);
    }  
  }

  else
  { if(strcasecmp(payloadcopy, "OFF")==0)
    { AddToQueueForDisplay("Requested by HA-MQTT", MESSAGE_POWERDOWN);
      bPowerStatus = false;
      PresetRequestFromHA = 0;
    }
    else if(strcasecmp(payloadcopy, "ON")==0)
    { AddToQueueForDisplay("Requested by HA-MQTT", MESSAGE_POWERUP);
      bPowerStatus = true;
    }
  }  
}

void onMqttConnected() 
{ bMqttActivated = 1234;
  Serial.println("Connected Succesfully to HA MQTT broker!");
  // You can subscribe to custom topic if you need
  mqtt.subscribe(WiFi.getHostname()); // only listen to RADIOGLOBE-54 or whatever the hostname of the globe is
}

void onMqttDisconnected() {
  Serial.println("Failed to connect to the broker!");
  bMqttActivated = 4567;
}

HASwitch HA_GlobePowerSwitch("GlobePowerSwitch");
HASwitch HA_GlobeBTSwitch("GlobeBTSwitch");
HANumber HA_GlobeVolume("GlobeVolume");
HASwitch HA_GlobeSpeakerSwitch("GlobeSpeakerSwitch");
HASwitch HA_GlobePreset1("GlobePreset1");
HASwitch HA_GlobePreset2("GlobePreset2");
HASwitch HA_GlobePreset3("GlobePreset3");
HASwitch HA_GlobePreset4("GlobePreset4");
HASensor HA_GlobeGpsNs("GlobeGpsNs"); 
HASensor HA_GlobeGpsEw("GlobeGpsEw"); 
HASensor HA_GlobeStationTitle("GlobeStationTitle");
HASensor HA_GlobeSongTitle("GlobeSongTitle");
HASensor HA_GlobeRadioUrl("GlobeRadioUrl");
HASensor HA_PuckBattery("PuckBattery");
HASensor HA_PuckSerialNumber("PuckSerialNumber");


void setupMQTT(void)
{ char mqtt_ip_address[64];
  mqtt.onMessage(onMqttMessage);
  mqtt.onConnected(onMqttConnected);
  mqtt.onDisconnected(onMqttDisconnected);
  

  device.enableExtendedUniqueIds();
  device.setName(WiFi.getHostname()); // -> something like RADIOGLOBE-54 with puck serial number (set in Avanced.ino)
  device.setSoftwareVersion("V1.0 by Frank");
  device.setModel("RadioGlobe V1.0");
  device.setManufacturer("Frank Techniek");

  //GlobePowerSwitch.setOptimistic(true); 
  HA_GlobePowerSwitch.setIcon("mdi:power");
  HA_GlobePowerSwitch.setName("Globe Power");

  HA_GlobeVolume.setName("Globe Volume");
  HA_GlobeVolume.setIcon("mdi:volume-high");

  HA_GlobeSpeakerSwitch.setIcon("mdi:speaker");
  HA_GlobeSpeakerSwitch.setName("Globe Speaker");

  HA_GlobeBTSwitch.setIcon("mdi:bluetooth");
  HA_GlobeBTSwitch.setName("Globe BlueTooth");

  HA_GlobePreset1.setIcon("mdi:radiobox-marked");
  HA_GlobePreset1.setName("Globe Preset 1");
  HA_GlobePreset2.setIcon("mdi:radiobox-marked");
  HA_GlobePreset2.setName("Globe Preset 2");
  HA_GlobePreset3.setIcon("mdi:radiobox-marked");
  HA_GlobePreset3.setName("Globe Preset 3");
  HA_GlobePreset4.setIcon("mdi:radiobox-marked");
  HA_GlobePreset4.setName("Globe Preset 4");
  

  // configure sensor (optional)
  // HA_GlobeGpsNs.setUnitOfMeasurement("Deg");
  // HA_GlobeGpsNs.setDeviceClass("???"); // wish there was a GPS class, non-existent class leads not non-detection of sensor (oops)
  HA_GlobeGpsNs.setIcon("mdi:crosshairs-gps");
  HA_GlobeGpsNs.setName("GPS Latitude");
  // configure sensor (optional)
  // HA_GlobeGpsEw.setUnitOfMeasurement("Deg");
  // HA_GlobeGpsEw.setDeviceClass("???"); // wish there was a GPS class, non-existent class leads not non-detection of sensor (oops)
  HA_GlobeGpsEw.setIcon("mdi:crosshairs-gps");
  HA_GlobeGpsEw.setName("GPS Longitude");
  
  HA_GlobeStationTitle.setIcon("mdi:label");
  HA_GlobeStationTitle.setName("Station: ");
  HA_GlobeSongTitle.setIcon("mdi:label");
  HA_GlobeSongTitle.setName("Song: ");
  HA_GlobeRadioUrl.setIcon("mdi:label");
  HA_GlobeRadioUrl.setName("URL: ");
  HA_PuckBattery.setDeviceClass("VOLTAGE");
  HA_PuckBattery.setUnitOfMeasurement("V");
  HA_PuckBattery.setIcon("mdi:battery");
  HA_PuckBattery.setName("Puck Battery");
  HA_PuckSerialNumber.setIcon("mdi:account-circle");
  HA_PuckSerialNumber.setName("Puck Serial");
  


  sprintf(mqtt_ip_address, "%d.%d.%d.%d", (uint16_t)GlobeSettings.ee_mqttserver_ip[0], (uint16_t)GlobeSettings.ee_mqttserver_ip[1], (uint16_t)GlobeSettings.ee_mqttserver_ip[2], (uint16_t)GlobeSettings.ee_mqttserver_ip[3]);
  
  if(GlobeSettings.ee_mqttserver_ip[0] && GlobeSettings.ee_mqttserver_ip[1] && GlobeSettings.ee_mqttserver_ip[2] && GlobeSettings.ee_mqttserver_ip[3])
  {  mqtt.begin(mqtt_ip_address, "mqtt1", "mqtt1"); // mqtt_ip_address, (user not), (password not) for the mqtt broker can be set in portal config
     Serial.printf("Connecting mqtt at ip %s\n", mqtt_ip_address);
     mqtt.loop();
     if(mqtt.isConnected())
     { AddToQueueForDisplay("ON", MESSAGE_MQTT_STATUS);
       Serial.printf("MQTT connected\n");
     }
     else 
     { AddToQueueForDisplay("OFF", MESSAGE_MQTT_STATUS);
       Serial.printf("MQTT NOT connected\n");
     }
  }
  else Serial.printf("No MQTT Server IP configured in Portal\n");
}

void loopMQTT(void) {
  static char previous_gpsns[16];
  static char previous_gpsew[16];
  static char previous_powerstatus[8];
  static uint16_t previous_volume = 9999;
  static uint16_t previous_internal_speakers = 9999;
  static uint16_t previous_btmodule_power_on = 9999;
  static unsigned int previousPresetRequestFromHA = 9999;
  char value[16];
  static char previous_station_title[256] = "";
  static char previous_song_title[256] = "";
  static char previous_radio_url[256] = "";
  static uint16_t previous_batteryvoltage = 0;
  static uint16_t previous_serialnumber = 0;


  mqtt.loop();

  // every second, update values using MQTT (for Home Assistant)
  if ((millis() - lastSentAt) >= 1000) 
  { lastSentAt = millis();
    sprintf(value, "%.6f", DataFromDisplay.D_StationGpsNS);
    if(strcmp(previous_gpsns, value)!=NULL)
    { strcpy(previous_gpsns, value);
      Serial.printf("Tell HA new D_StationGpsNS is %s\n", value);
      HA_GlobeGpsNs.setValue(value);
    }

    sprintf(value, "%.6f", DataFromDisplay.D_StationGpsEW);
    if(strcmp(previous_gpsew, value)!=NULL)
    { strcpy(previous_gpsew, value);
      Serial.printf("Tell HA new D_StationGpsEW is %s\n", value);
      HA_GlobeGpsEw.setValue(value);
    }

    if(bPowerStatus)strcpy(value, "On");
    else            strcpy(value, "Off");
    if(strcmp(previous_powerstatus, value)!=NULL)
    { strcpy(previous_powerstatus, value);
      Serial.printf("Tell HA new powerstatus is %s\n", value);
      HA_GlobePowerSwitch.setState(bPowerStatus);
    }

    //if(bPowerStatus)
    { if(previous_volume != GlobeSettings.ee_volume)
      { previous_volume = GlobeSettings.ee_volume; 
        Serial.printf("Tell HA new GlobeVolumeValue is %d\n", GlobeSettings.ee_volume);
        HA_GlobeVolume.setState(HANumeric((int16_t)GlobeSettings.ee_volume, (uint8_t)0), 0);
      }  
    }  

    //if(bPowerStatus)
    { if(previous_internal_speakers != GlobeSettings.ee_internal_speakers)
      { previous_internal_speakers = GlobeSettings.ee_internal_speakers;
        HA_GlobeSpeakerSwitch.setState((GlobeSettings.ee_internal_speakers==1));
      }
    }

    { if(previous_btmodule_power_on != GlobeSettings.btmodule_power_on)
      { previous_btmodule_power_on = GlobeSettings.btmodule_power_on;
        HA_GlobeBTSwitch.setState((GlobeSettings.btmodule_power_on==1));
      }
    }


    if(previousPresetRequestFromHA != PresetRequestFromHA)
    { previousPresetRequestFromHA = PresetRequestFromHA;
      Serial.printf("Tell HA new Preset Station is %ld\n", PresetRequestFromHA);
      for(int n=1; n<5;n++)
      { if(n == 1)
        { if(n == PresetRequestFromHA)HA_GlobePreset1.setState(true);
          else HA_GlobePreset1.setState(false);
        }
        if(n == 2)
        { if(n == PresetRequestFromHA)HA_GlobePreset2.setState(true);
          else HA_GlobePreset2.setState(false);
        }
        if(n == 3)
        { if(n == PresetRequestFromHA)HA_GlobePreset3.setState(true);
          else HA_GlobePreset3.setState(false);
        }
        if(n == 4)
        { if(n == PresetRequestFromHA)HA_GlobePreset4.setState(true);
          else HA_GlobePreset4.setState(false);
        }
      }
    }

    if(strcmp(previous_station_title, ActiveStationTitle)!=0)
    { strcpy(previous_station_title, ActiveStationTitle);
      Serial.printf("Tell HA new Station Title is %s\n", ActiveStationTitle);
      HA_GlobeStationTitle.setValue(ActiveStationTitle);
    }

    if(strcmp(previous_song_title, ActiveSongTitle)!=0)
    { strcpy(previous_song_title, ActiveSongTitle);
      Serial.printf("Tell HA new Song Title is %s\n", ActiveSongTitle);
      HA_GlobeSongTitle.setValue(ActiveSongTitle);
    }

    if(strcmp(previous_radio_url, ActiveUrl)!=0)
    { strcpy(previous_radio_url, ActiveUrl);
      Serial.printf("Tell HA new Radio URL is %s\n", ActiveUrl);
      HA_GlobeRadioUrl.setValue(ActiveUrl);
    }

    if(previous_batteryvoltage != DataFromDisplay.D_BatteryVoltage)
    { previous_batteryvoltage = DataFromDisplay.D_BatteryVoltage;
      sprintf(value, "%d.%d", DataFromDisplay.D_BatteryVoltage/10, DataFromDisplay.D_BatteryVoltage%10);
      HA_PuckBattery.setValue(value); 
    }

    if(previous_serialnumber != GlobeSettings.serialnumber)
    { previous_serialnumber = GlobeSettings.serialnumber;
      sprintf(value, "%d", GlobeSettings.serialnumber);
      Serial.printf("Tell HA Globe Serial is %s\n", value);
      HA_PuckSerialNumber.setValue(value); 
    }
  }
}
