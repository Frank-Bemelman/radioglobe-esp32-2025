// library -> manage libraries -> home-assistant-integration Version 2.1.0
#include <ArduinoHA.h>


#define BROKER_ADDR IPAddress(192,168,1,121)
unsigned long lastSentAt = millis();
HADevice device(WiFi.getHostname()); // -> something like RADIOGLOBE-B3EC16A398 full mac address
//HADevice device("RadioGlobeTwo"); // -> something like RADIOGLOBE-EC16A398 partial mac address
WiFiClient HAclient; 
HAMqtt mqtt(HAclient, device);

void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length)
{ // This callback is called when message from MQTT broker is received.
  // Please note that you should always verify if the message's topic is the one you expect.
  // For example: if (memcmp(topic, "myCustomTopic") == 0) { ... }
  char payloadcopy[256];
  char message[QUEUEMESSAGELENGTH]; 
  char *p;

  strncpy(payloadcopy, (const char*)payload, length);
  payloadcopy[length]=0;
  Serial.print("New message on topic: ");
  Serial.println(topic);
  Serial.print("Data: ");
  Serial.println(payloadcopy);
  Serial.println(topic);
  Serial.println(payloadcopy);

  unsigned int poweron = 0;
  unsigned int volume = 0;
  
  

  if(strstr(topic, "GlobeSpeakerSwitch" )!=NULL)
  { GlobeSettings.ee_internal_speakers = strcmp(payloadcopy, "OFF")==0 ? 0:1;
    digitalWrite(MUTE_AMPLIFIERS, strcmp(payloadcopy, "OFF")==0 ? 1:0); // mute output works as a disable
    sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
    AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE);
    //AddToQueueForDisplay("?", MESSAGE_FINDNEWSTATION); // test
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
  { //sscanf((const char*)payloadcopy, "%d", &poweron);
    //if(strcmp(payloadcopy, "Off")==0)GlobePowerDown();
    if(strcmp(payloadcopy, "OFF")==0)
    { // volume down
      //sprintf(message, "%d %d %d %d", 0, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
      //AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE); 
       

      AddToQueueForDisplay("Requested by HA-MQTT", MESSAGE_POWERDOWN);
      bPowerStatus = false;
      PresetRequestFromHA = 0;
    }
    //if(strcmp(payloadcopy, "On")==0)GlobePowerUp();
    if(strcmp(payloadcopy, "ON")==0)
    { //sprintf(message, "%d %d %d %d", GlobeSettings.ee_volume, GlobeSettings.ee_bass, GlobeSettings.ee_treble, GlobeSettings.ee_internal_speakers);
      //AddToQueueForDisplay(message, MESSAGE_VOLUME_AND_TONE); 
      AddToQueueForDisplay("Requested by HA-MQTT", MESSAGE_POWERUP);
      bPowerStatus = true;
    }
  }  
}

void onMqttConnected() 
{ bMqttActivated = 1234;
  Serial.println("Connected Succesfully to HA MQTT broker!");
  // You can subscribe to custom topic if you need
  mqtt.subscribe(WiFi.getHostname()); // only listen to RADIOGLOBE-30EDA0B72910 or whatever the hostname of the globe is
}

void onMqttDisconnected() {
  Serial.println("Failed to connect to the broker!");
  bMqttActivated = 4567;
}

HASwitch GlobePowerSwitch("GlobePowerSwitch");
HANumber GlobeVolume("GlobeVolume");
HASwitch GlobeSpeakerSwitch("GlobeSpeakerSwitch");
HASwitch GlobePreset1("GlobePreset1");
HASwitch GlobePreset2("GlobePreset2");
HASwitch GlobePreset3("GlobePreset3");
HASwitch GlobePreset4("GlobePreset4");
HASensor GlobeGpsNs("GlobeGpsNs"); // " " is unique ID of the sensor. You should define your own ID.
HASensor GlobeGpsEw("GlobeGpsEw"); // " " is unique ID of the sensor. You should define your own ID.
HASensor GlobeStationTitle("GlobeStationTitle");
HASensor GlobeSongTitle("GlobeSongTitle");
HASensor GlobeRadioUrl("GlobeRadioUrl");
HASensor GlobePuckBattery("GlobePuckBattery");
HASensor DisplaySerialNumber("DisplaySerialNumber");


void setupMQTT(void)
{ char mqtt_ip_address[64];
  mqtt.onMessage(onMqttMessage);
  mqtt.onConnected(onMqttConnected);
  mqtt.onDisconnected(onMqttDisconnected);
  

  device.enableExtendedUniqueIds();
  device.setName(WiFi.getHostname()); // -> something like RADIOGLOBE-98A316EC27C4-54 with mac address and puck serial number (set in Avanced.ino)
  device.setSoftwareVersion("V1.0 by Frank");
  device.setModel("RadioGlobe V1.0");
  device.setManufacturer("Frank Techniek");

  //GlobePowerSwitch.setOptimistic(true); 
  GlobePowerSwitch.setIcon("mdi:power");
  GlobePowerSwitch.setName("Globe Power");

  GlobeVolume.setName("Globe Volume");
  GlobeVolume.setIcon("mdi:volume-high");

  GlobeSpeakerSwitch.setIcon("mdi:speaker");
  GlobeSpeakerSwitch.setName("Globe Speaker");
 
  GlobePreset1.setIcon("mdi:radiobox-marked");
  GlobePreset1.setName("Globe Preset 1");
  GlobePreset2.setIcon("mdi:radiobox-marked");
  GlobePreset2.setName("Globe Preset 2");
  GlobePreset3.setIcon("mdi:radiobox-marked");
  GlobePreset3.setName("Globe Preset 3");
  GlobePreset4.setIcon("mdi:radiobox-marked");
  GlobePreset4.setName("Globe Preset 4");
  

  // configure sensor (optional)
  // GlobeGpsNs.setUnitOfMeasurement("Deg");
  // GlobeGpsNs.setDeviceClass("???"); // wish there was a GPS class, non-existent class leads not non-detection of sensor
  GlobeGpsNs.setIcon("mdi:crosshairs-gps");
  GlobeGpsNs.setName("GPS Latitude");
  // configure sensor (optional)
  // GlobeGpsEw.setUnitOfMeasurement("Deg");
  // GlobeGpsEw.setDeviceClass("???"); // wish there was a GPS class, non-existent class leads not non-detection of sensor 
  GlobeGpsEw.setIcon("mdi:crosshairs-gps");
  GlobeGpsEw.setName("GPS Longitude");
  
  GlobeStationTitle.setIcon("mdi:label");
  GlobeStationTitle.setName("Station: ");
  GlobeSongTitle.setIcon("mdi:label");
  GlobeSongTitle.setName("Song: ");
  GlobeRadioUrl.setIcon("mdi:label");
  GlobeRadioUrl.setName("URL: ");
  GlobePuckBattery.setDeviceClass("VOLTAGE");
  GlobePuckBattery.setUnitOfMeasurement("V");
  GlobePuckBattery.setIcon("mdi:battery");
  GlobePuckBattery.setName("Puck Battery");
  GlobeSongTitle.setIcon("mdi:label");
  DisplaySerialNumber.setName("Puck Serial");


  sprintf(mqtt_ip_address, "%d.%d.%d.%d", (uint16_t)GlobeSettings.ee_mqttserver_ip[0], (uint16_t)GlobeSettings.ee_mqttserver_ip[1], (uint16_t)GlobeSettings.ee_mqttserver_ip[2], (uint16_t)GlobeSettings.ee_mqttserver_ip[3]);
  
  if(GlobeSettings.ee_mqttserver_ip[0] && GlobeSettings.ee_mqttserver_ip[1] && GlobeSettings.ee_mqttserver_ip[2] && GlobeSettings.ee_mqttserver_ip[3])
  {  //mqtt.begin(BROKER_ADDR, "mqtt1", "mqtt1"); // user and password for the mqtt broker should be a portal config
     mqtt.begin(mqtt_ip_address, "mqtt1", "mqtt1"); // user and password for the mqtt broker is now a portal config
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
    sprintf(value, "%.6f", D_StationGpsNS);
    if(strcmp(previous_gpsns, value)!=NULL)
    { strcpy(previous_gpsns, value);
      Serial.printf("Tell HA new D_StationGpsNS is %s\n", value);
      GlobeGpsNs.setValue(value);
    }

    sprintf(value, "%.6f", D_StationGpsEW);
    if(strcmp(previous_gpsew, value)!=NULL)
    { strcpy(previous_gpsew, value);
      Serial.printf("Tell HA new D_StationGpsEW is %s\n", value);
      GlobeGpsEw.setValue(value);
    }

    if(bPowerStatus)strcpy(value, "On");
    else            strcpy(value, "Off");
    if(strcmp(previous_powerstatus, value)!=NULL)
    { strcpy(previous_powerstatus, value);
//      GlobePowerState.setValue(value);
      Serial.printf("Tell HA new powerstatus is %s\n", value);
      GlobePowerSwitch.setState(bPowerStatus);
    }

    //if(bPowerStatus)
    { if(previous_volume != GlobeSettings.ee_volume)
      { previous_volume = GlobeSettings.ee_volume; 
        Serial.printf("Tell HA new GlobeVolumeValue is %d\n", GlobeSettings.ee_volume);
        GlobeVolume.setState(HANumeric((int16_t)GlobeSettings.ee_volume, (uint8_t)0), 0);
      }  
    }  

    //if(bPowerStatus)
    { if(previous_internal_speakers != GlobeSettings.ee_internal_speakers)
      { previous_internal_speakers = GlobeSettings.ee_internal_speakers;
        GlobeSpeakerSwitch.setState((GlobeSettings.ee_internal_speakers==1));
      }
    }

    if(previousPresetRequestFromHA != PresetRequestFromHA)
    { previousPresetRequestFromHA = PresetRequestFromHA;
      Serial.printf("Tell HA new Preset Station is %ld\n", PresetRequestFromHA);
      for(int n=1; n<5;n++)
      { if(n == 1)
        { if(n == PresetRequestFromHA)GlobePreset1.setState(true);
          else GlobePreset1.setState(false);
        }
        if(n == 2)
        { if(n == PresetRequestFromHA)GlobePreset2.setState(true);
          else GlobePreset2.setState(false);
        }
        if(n == 3)
        { if(n == PresetRequestFromHA)GlobePreset3.setState(true);
          else GlobePreset3.setState(false);
        }
        if(n == 4)
        { if(n == PresetRequestFromHA)GlobePreset4.setState(true);
          else GlobePreset4.setState(false);
        }
      }
    }

    if(strcmp(previous_station_title, ActiveStationTitle)!=0)
    { strcpy(previous_station_title, ActiveStationTitle);
      Serial.printf("Tell HA new Station Title is %s\n", ActiveStationTitle);
      GlobeStationTitle.setValue(ActiveStationTitle);
    }

    if(strcmp(previous_song_title, ActiveSongTitle)!=0)
    { strcpy(previous_song_title, ActiveSongTitle);
      Serial.printf("Tell HA new Song Title is %s\n", ActiveSongTitle);
      GlobeSongTitle.setValue(ActiveSongTitle);
    }

    if(strcmp(previous_radio_url, ActiveUrl)!=0)
    { strcpy(previous_radio_url, ActiveUrl);
      Serial.printf("Tell HA new Radio URL is %s\n", ActiveUrl);
      GlobeRadioUrl.setValue(ActiveUrl);
    }

    if(previous_batteryvoltage != DataFromDisplay.D_BatteryVoltage)
    { previous_batteryvoltage = DataFromDisplay.D_BatteryVoltage;
      sprintf(value, "%d.%d", DataFromDisplay.D_BatteryVoltage/10, DataFromDisplay.D_BatteryVoltage%10);
      Serial.printf("Tell HA Puck Battery level is %s\n", value);
      GlobePuckBattery.setValue(value); 
    }

    if(previous_serialnumber != GlobeSettings.serialnumber)
    { previous_serialnumber = GlobeSettings.serialnumber;
      sprintf(value, "%d", GlobeSettings.serialnumber);
      Serial.printf("Tell HA Globe Serial is %s\n", value);
      DisplaySerialNumber.setValue(value); 
    }
  }
}
