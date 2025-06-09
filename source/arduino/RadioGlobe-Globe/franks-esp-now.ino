/*
  Credits for this work from which I borrowed most code
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <esp_now.h>

// REPLACE WITH THE MAC Address of radio display
// which is b4:3a:45:a5:03:10
uint8_t broadcastAddress[] = {0xB4, 0x3A, 0x45, 0xA5, 0x03, 0x10};

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
  memcpy(&DataFromDisplay, incomingData, sizeof(DataFromDisplay));
  
  if(PrevDataFromDisplay.CalibrateZeroPos != DataFromDisplay.CalibrateZeroPos)
  { PrevDataFromDisplay.CalibrateZeroPos = DataFromDisplay.CalibrateZeroPos;
    if(DataFromDisplay.CalibrateZeroPos == 1234)
    { CalibrateZeroPos = 1234; // will be honored in loop()
    }
  }

//  if(strcmp(PrevDataFromDisplay.RadioUrlRequest, DataFromDisplay.RadioUrlRequest) != 0)
//  { strcpy(PrevDataFromDisplay.RadioUrlRequest, DataFromDisplay.RadioUrlRequest);
//  }

  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //Serial.print("Volume: ");
  //Serial.println(DataSendToRadio.volumevalue);


}
 
void setup_esp_now() {
  
    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  strcpy(DataFromGlobe.Name, "");
  strcpy(DataFromGlobe.Title, "");
  strcpy(DataFromGlobe.TimeZoneId, "");

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
  // we get called here from the main loop, every 200mS

  // Send message via ESP-NOW
  if(bUpAndRunning)
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataFromGlobe, sizeof(DataFromGlobe));
   
//  if (result == ESP_OK) {
    // Serial.println("In loop_esp_now() Sent with success");
//  }
  //else {
    //Serial.println("In loop_esp_now() Error sending the data");
  //}
  
}

