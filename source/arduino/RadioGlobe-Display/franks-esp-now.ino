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
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&DataFromGlobe, incomingData, sizeof(DataFromGlobe));

  // todo -> recalibration of coordinates
  DataFromDisplay.ew_cal = DataFromGlobe.ew;
  DataFromDisplay.ns_cal = DataFromGlobe.ns;

  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //Serial.println(DataFromGlobe.ns);
  //Serial.println(DataFromGlobe.ew);
  //Serial.println(DataFromGlobe.Title);
}
 
void setup_esp_now() {
  
    // Set device as a Wi-Fi Station & accespoint 
//  WiFi.begin("SSID", "PASSWORD");
  WiFi.mode(WIFI_AP_STA); // hoewel, WIFI_STA werkte ook

  strcpy(PrevDataFromGlobe.Name, "???");
  strcpy(PrevDataFromGlobe.Title, "???");
  strcpy(PrevDataFromGlobe.TimeZoneId, "???");


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
 
void loop_esp_now() {

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataFromDisplay, sizeof(DataFromDisplay));
   


  if (result == ESP_OK) 
  { //Serial.println("Sent with success");
    if(DataFromDisplay.CalibrateZeroPos == 5678)
    { Serial.print("DataFromDisplay.CalibrateZeroPos = "); 
      Serial.println(DataFromDisplay.CalibrateZeroPos);
      DataFromDisplay.CalibrateZeroPos = 0;
    }
  }
  //else {
  //  //Serial.println("Error sending the data");
  //}
  
}

