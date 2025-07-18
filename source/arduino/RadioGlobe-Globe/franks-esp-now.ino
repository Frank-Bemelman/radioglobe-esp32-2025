/*
  Credits for this work from which I borrowed most code
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <esp_now.h>

QueueOut ToDisplay; // queue with messages for display

// REPLACE WITH THE MAC Address of display puck
// which is b4:3a:45:a5:03:10 (PUCK1)
//uint8_t broadcastAddress[] = {0xB4, 0x3A, 0x45, 0xA5, 0x03, 0x10};

// which is a0:85:e3:e1:53:28 (PUCK2)
uint8_t broadcastAddress[] = {0xA0, 0x85, 0xE3, 0xE1, 0x53, 0x28};


esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//Serial.print("\r\nLast Packet Send Status:\t");
//Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&DataFromDisplay, incomingData, sizeof(DataFromDisplay));
  
  // quickly store gps location for timezone request before they become altered
  if(PrevDataFromDisplay.D_QueueMessageType != DataFromDisplay.D_QueueMessageType)
  { PrevDataFromDisplay.D_QueueMessageType = DataFromDisplay.D_QueueMessageType;
    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE_BY_GPS)
    { D_StationGpsNS = DataFromDisplay.D_StationGpsNS;
      D_StationGpsEW = DataFromDisplay.D_StationGpsEW;
      DataFromGlobe.FindTimeZone = MESSAGE_GET_TIMEZONE_BY_GPS; // makes the CallGetTimeZone task actually do it
    }  

    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE)
    { ns_cal_received = DataFromDisplay.ns_cal;
      ew_cal_received = DataFromDisplay.ew_cal;
      DataFromGlobe.FindTimeZone = MESSAGE_GET_TIMEZONE; // makes the CallGetTimeZone task actually do it
    }  

    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION_BY_GPS)
    { D_GeoLocationGpsNS = DataFromDisplay.D_StationGpsNS;
      D_GeoLocationGpsEW = DataFromDisplay.D_StationGpsEW;
      DataFromGlobe.FindGeoLocationData = MESSAGE_GET_GEOLOCATION_BY_GPS; // makes the CallGetTimeZone task actually do it
    }  

  }  

  //Serial.print("Bytes received: ");
  //Serial.println(len);

}
 
void setup_esp_now() {
  
    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

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
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

bool Q_filling = 0;
bool Q_sending = 0;

void loop_esp_now() {
  static uint16_t acked_qserialnumber = 0;
  // we get called here from the encoder loop, every 200mS

  while(Q_filling); // wait until idle
  Q_sending = true;
  // check for queued messages to send
  // Send message via ESP-NOW
  if(1)//bUpAndRunning)
  { // first check if last prepared message was received and processed by display, we are both in sync
    if(DataFromDisplay.G_QueueSerialNumber == DataFromGlobe.G_QueueSerialNumber)
    { // yes we are in sync, get new message from queue to send out, if any

      if(acked_qserialnumber != DataFromDisplay.G_QueueSerialNumber)
      { // Serial.printf("Message %d acknowledged by display.\n", DataFromDisplay.G_QueueSerialNumber);
        acked_qserialnumber = DataFromDisplay.G_QueueSerialNumber;
      }  
      if(ToDisplay.QueueCnt>0)
      { if(ToDisplay.QueueIndexOut>=QUEUESIZE)ToDisplay.QueueIndexOut = 0;
        if(ToDisplay.QueueIndexOut != ToDisplay.QueueIndexIn)
        { DataFromGlobe.G_QueueSerialNumber++;
          strcpy(DataFromGlobe.G_QueueMessage, ToDisplay.QueueMessage[ToDisplay.QueueIndexOut]);   
          DataFromGlobe.G_QueueMessageType = ToDisplay.QueueMessageType[ToDisplay.QueueIndexOut];
          //Serial.printf("Message %d-%d sent to display = >%s<\n", DataFromGlobe.G_QueueSerialNumber, DataFromGlobe.G_QueueMessageType, DataFromGlobe.G_QueueMessage);
          ToDisplay.QueueIndexOut++;
          ToDisplay.QueueCnt--;
        }
      }
    }
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataFromGlobe, sizeof(DataFromGlobe));
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


