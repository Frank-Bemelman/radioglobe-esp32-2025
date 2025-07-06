/*
  Credits for this work from which I borrowed most code
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <esp_now.h>
#include "franks-esp-now.h"

QueueOut ToGlobe; // queue with messages for display

// REPLACE WITH THE MAC Address of radio esp
// which is 94:b5:55:2b:2a:d4 (ka[pot, diode])
//uint8_t broadcastAddress[] = {0x94, 0xB5, 0x55, 0x2B, 0x2A, 0xD4};
// 0c:b8:15:c3:e4:10 // kapot
// e0:5a:1b:e2:f0:68 wroom zonder dubbele antenne
uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xE2, 0xF0, 0x68};

// Variable to store if sending data was successful
char success[128];

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    strcpy(success, "Delivery Success :");
  }
  else{
    strcpy(success, "Delivery Fail :(");
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{ static int16_t old_ns = -1;
  static int16_t old_ew = -1;
  
  memcpy(&DataFromGlobe, incomingData, sizeof(DataFromGlobe));

  // coordinates are received outside the queue for speed
  // adjust these coordinates, if changed, according to calibration table
  if(bUpAndRunning && DataFromGlobe.G_EncoderReliable)
  { if((old_ns != DataFromGlobe.ns) || (old_ew != DataFromGlobe.ew))
    { old_ns = DataFromGlobe.ns;
      old_ew = DataFromGlobe.ew;
    }
  }
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //Serial.println(DataFromGlobe.ns);
  //Serial.println(DataFromGlobe.ew);
  //Serial.println(DataFromGlobe.Title);
}
 
void setup_esp_now(void) 
{ // Set device as a Wi-Fi Station & accespoint 
  //  WiFi.begin("SSID", "PASSWORD");
  WiFi.mode(WIFI_AP_STA); // hoewel, WIFI_STA werkte ook

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
  // we get called here from the main loop, every 200mS

  while(Q_filling); // wait until idle
  Q_sending = true;

  // check for queued messages to send
  // Send message via ESP-NOW

  if(bUpAndRunning) 
  { // first check if last prepared message was received and processed by display, we are both in sync
    if(DataFromGlobe.D_QueueSerialNumber == DataFromDisplay.D_QueueSerialNumber)
    { // yes we are in sync, get new message from queue to send out, if any

      if(acked_qserialnumber != DataFromGlobe.D_QueueSerialNumber)
      { // Serial.printf("Message %d acknowledged by globe.\n", DataFromGlobe.D_QueueSerialNumber);
        acked_qserialnumber = DataFromGlobe.D_QueueSerialNumber;
      }  

      // Serial.printf("ToGlobe.QueueCnt = %d\n", ToGlobe.QueueCnt);
      if(ToGlobe.QueueCnt>0)
      { if(ToGlobe.QueueIndexOut>=QUEUESIZE)ToGlobe.QueueIndexOut = 0;
        //Serial.printf("Need to send if ToGlobe.QueueIndexin %d != ToGlobe.QueueIndexOut %d\n", ToGlobe.QueueIndexIn, ToGlobe.QueueIndexOut);
        if(ToGlobe.QueueIndexOut != ToGlobe.QueueIndexIn)
        { DataFromDisplay.D_QueueSerialNumber++;
          strcpy(DataFromDisplay.D_QueueMessage, ToGlobe.QueueMessage[ToGlobe.QueueIndexOut]);
          DataFromDisplay.D_QueueMessageType = ToGlobe.QueueMessageType[ToGlobe.QueueIndexOut];
          Serial.printf("Messagetype %d sent to display = >%s<\n", DataFromDisplay.D_QueueMessageType, DataFromDisplay.D_QueueMessage);
          ToGlobe.QueueIndexOut++;
          ToGlobe.QueueCnt--;
        }
      }
    }
 
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataFromDisplay, sizeof(DataFromDisplay));
  } 


  //else {
  //  //Serial.println("Error sending the data");
  //}
  Q_sending = false;
}

void AddToQueueForGlobe(const char* message, uint16_t queuemessagetype) // one empty slot to avoid IndexIn becomes equal to IndexOut
{ 
  while(Q_sending); // wait until idle
  Q_filling = true;
  
  if(ToGlobe.QueueCnt < QUEUESIZE-1)
  { if(ToGlobe.QueueIndexIn>=QUEUESIZE)ToGlobe.QueueIndexIn = 0;
    strncpy(ToGlobe.QueueMessage[ToGlobe.QueueIndexIn], message, QUEUEMESSAGELENGTH);
    ToGlobe.QueueMessage[ToGlobe.QueueIndexIn][QUEUEMESSAGELENGTH-1] = 0; // terminate just in case of idiotic long message
    ToGlobe.QueueMessageType[ToGlobe.QueueIndexIn] = queuemessagetype;
    //Serial.printf("Message type %d Queued in %d  = >%s<\n", queuemessagetype, ToGlobe.QueueIndexIn, ToGlobe.QueueMessage[ToGlobe.QueueIndexIn]);

    ToGlobe.QueueIndexIn++;
    ToGlobe.QueueCnt++;
  }
  else Serial.println("Queue to globe is full!!!");
  Q_filling = false;
}

