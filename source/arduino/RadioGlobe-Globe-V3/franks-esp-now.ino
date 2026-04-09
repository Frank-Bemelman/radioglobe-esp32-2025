/*
  Credits for this work from which I borrowed most code
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <esp_now.h>

esp_now_peer_info_t peerInfo;

//void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status);
void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *incomingData, int len);

// Callback when data is sent
void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status){
//void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//Serial.print("\r\nLast Packet Send Status:\t");
//Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *incomingData, int len) 
{ static uint16_t MessageSerialNumber = -1;
  static bool firstskipped = false; // skip the first message after (re)boot
  
  //Serial.printf("Received From Mac %02X:%02X:%02X:%02X:%02X:%02X\n", rx_info->src_addr[0], rx_info->src_addr[1], rx_info->src_addr[2], rx_info->src_addr[3], rx_info->src_addr[4], rx_info->src_addr[5]);  
  
  memcpy(&DataFromDisplay, incomingData, sizeof(DataFromDisplay));
  
  // quickly store gps location for timezone request before they become altered
  if(PrevDataFromDisplay.D_QueueMessageType != DataFromDisplay.D_QueueMessageType)
  { PrevDataFromDisplay.D_QueueMessageType = DataFromDisplay.D_QueueMessageType;
    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE_BY_GPS)
    { D_StationGpsNS = DataFromDisplay.D_StationGpsNS;
      D_StationGpsEW = DataFromDisplay.D_StationGpsEW;
      DataFromGlobe.FindTimeZone = MESSAGE_GET_TIMEZONE_BY_GPS; // makes the CallGetTimeZone task actually do it
      return; // don't add to queue
    }  

    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE)
    { ns_cal_received = DataFromDisplay.ns_cal;
      ew_cal_received = DataFromDisplay.ew_cal;
      DataFromGlobe.FindTimeZone = MESSAGE_GET_TIMEZONE; // makes the CallGetTimeZone task actually do it
      return; // don't add to queue
    }  

    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION_BY_GPS)
    { D_GeoLocationGpsNS = DataFromDisplay.D_StationGpsNS;
      D_GeoLocationGpsEW = DataFromDisplay.D_StationGpsEW;
      DataFromGlobe.FindGeoLocationData = MESSAGE_GET_GEOLOCATION_BY_GPS; // makes the CallGetTimeZone task actually do it
      return; // don't add to queue
    }  

    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION)
    { ns_cal_received = DataFromDisplay.ns_cal;
      ew_cal_received = DataFromDisplay.ew_cal;
      DataFromGlobe.FindGeoLocationData = MESSAGE_GET_GEOLOCATION; // makes the CallGetTimeZone task actually do it
      return; // don't add to queue
    }  


  }  

  if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE_BY_GPS)return;
  if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE)return;
  if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION_BY_GPS)return;
  if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION)return;

  // put in receiving queue
  if(MessageSerialNumber != DataFromDisplay.D_QueueSerialNumber)
  { MessageSerialNumber = DataFromDisplay.D_QueueSerialNumber;
    // Serial.printf("DataFromDisplay.D_QueueSerialNumber = %d\n", DataFromDisplay.D_QueueSerialNumber);
    if(firstskipped)
    { memcpy(&FromDisplay.QueueMessage[FromDisplay.QueueIndexIn], DataFromDisplay.D_QueueMessage, sizeof(DataFromDisplay.D_QueueMessage));
      FromDisplay.QueueMessageType[FromDisplay.QueueIndexIn] =  DataFromDisplay.D_QueueMessageType;
      FromDisplay.QueueIndexIn++;
      FromDisplay.QueueIndexIn %= QUEUESIZE;
      FromDisplay.QueueCnt++;
    }
    firstskipped = true;
  }  

  //Serial.print("Bytes received: ");
  //Serial.println(len);

}
 
void setup_esp_now() 
{ ToDisplay.QueueIndexIn = 0; // queue with messages from puck to globe
  ToDisplay.QueueIndexOut = 0;
  ToDisplay.QueueCnt = 0;
  FromDisplay.QueueIndexIn = 0; // queue with messages from globe for puck
  FromDisplay.QueueIndexOut = 0;
  FromDisplay.QueueCnt = 0;

    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  Serial.printf("setup_esp_now() WiFi.channel() = %d\n", WiFi.channel());

  uint8_t primaryChan = WiFi.channel();
  wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
  esp_wifi_set_channel(primaryChan, secondChan);
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));

  AddToQueueForDisplay("", MESSAGE_STATION_NAME);
  AddToQueueForDisplay("", MESSAGE_SONG_TITLE);
  AddToQueueForDisplay("", MESSAGE_TIMEZONE_ID);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  
  if(PuckMac[0]==0)return; 
  // Register peer
  
  memcpy(peerInfo.peer_addr, PuckMac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  esp_now_del_peer(peerInfo.peer_addr);
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  Serial.printf("Added Puck Peer %02X:%02X:%02X:%02X:%02X:%02X\n", (uint16_t)PuckMac[0], (uint16_t)PuckMac[1], (uint16_t)PuckMac[2], (uint16_t)PuckMac[3], (uint16_t)PuckMac[4], (uint16_t)PuckMac[5]);
  
}

bool Q_filling = 0;
bool Q_sending = 0;

void loop_esp_now() {
  // we get called here from the encoder loop, every 200mS
  if(PuckMac[0]==0)return; 

  while(Q_filling); // wait until idle
  Q_sending = true;
  // check and send all queued messages via ESP-NOW
  if(1) //bUpAndRunning)
  { if(ToDisplay.QueueCnt>0)
    { while(ToDisplay.QueueCnt)
      { if(ToDisplay.QueueIndexOut != ToDisplay.QueueIndexIn)
        { DataFromGlobe.G_QueueSerialNumber++;
          DataFromGlobe.D_QueueMessageCount = ToDisplay.QueueCnt;
          strcpy(DataFromGlobe.G_QueueMessage, ToDisplay.QueueMessage[ToDisplay.QueueIndexOut]);   
          DataFromGlobe.G_QueueMessageType = ToDisplay.QueueMessageType[ToDisplay.QueueIndexOut];
          //Serial.printf("Message %d-%d sent to display = >%s<\n", DataFromGlobe.G_QueueSerialNumber, DataFromGlobe.G_QueueMessageType, DataFromGlobe.G_QueueMessage);
          ToDisplay.QueueIndexOut++;
          if(ToDisplay.QueueIndexOut>=QUEUESIZE)ToDisplay.QueueIndexOut = 0;
        }
        ToDisplay.QueueCnt--;
        esp_err_t result = esp_now_send(PuckMac, (uint8_t *) &DataFromGlobe, sizeof(DataFromGlobe));
        // memcpy(&PrevDataFromGlobe, &DataFromGlobe, sizeof(PrevDataFromGlobe));
      }
    }
    else // keep sending last one, with perhaps updated coordinates
    { if(memcmp(&PrevDataFromGlobe, &DataFromGlobe,sizeof(PrevDataFromGlobe)) != 0)
      { esp_err_t result = esp_now_send(PuckMac, (uint8_t *) &DataFromGlobe, sizeof(DataFromGlobe));
        // Serial.printf("Stuff (usually coordinates or rssi changed) send\n");
        memcpy(&PrevDataFromGlobe, &DataFromGlobe, sizeof(PrevDataFromGlobe));
      }  
    }
  }
  Q_sending = false;
}

//typedef struct {               
//  uint8_t  QueueMessages[50][128];  // 50 strings of 128 each (6400 bytes)
//  uint_16  QueueIndexIn;
//  uint_16  QueueIndexOut;
//  uint_16  QueueCnt;
//} Queue;

void AddToQueueForDisplay(const char* message, uint16_t queuemessagetype)
{ 
  while(Q_sending); // wait until idle
  Q_filling = true;

  if(queuemessagetype>=MESSAGE_MAX) // should never happen as this message is not defined yet
  { Serial.printf("AddToQueueForDisplay -> Message %d not defined!\n", queuemessagetype);
    return;
  }

  if(ToDisplay.QueueCnt < (QUEUESIZE-1)) // one empty slot to avoid IndexIn becomes equal to IndexOut
  { if(ToDisplay.QueueIndexIn>=QUEUESIZE)ToDisplay.QueueIndexIn = 0;
    strncpy(ToDisplay.QueueMessage[ToDisplay.QueueIndexIn], message, QUEUEMESSAGELENGTH);
    ToDisplay.QueueMessage[ToDisplay.QueueIndexIn][QUEUEMESSAGELENGTH] = 0; // terminate just in case of idiotic long message
    ToDisplay.QueueMessageType[ToDisplay.QueueIndexIn] = queuemessagetype;
    
    Serial.printf("TELL DISPLAY: %s -> %s\n", messagetexts[ToDisplay.QueueMessageType[ToDisplay.QueueIndexIn]], ToDisplay.QueueMessage[ToDisplay.QueueIndexIn]);

    ToDisplay.QueueIndexIn++;
    ToDisplay.QueueCnt++;
  }
  else Serial.println("Queue to display is full!!!");
  Q_filling = false;
}


