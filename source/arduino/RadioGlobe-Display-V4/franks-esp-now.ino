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


// REPLACE WITH THE MAC Address of globe
// which is 94:b5:55:2b:2a:d4 (ka[pot, diode])
//uint8_t broadcastAddress[] = {0x94, 0xB5, 0x55, 0x2B, 0x2A, 0xD4};
// 0c:b8:15:c3:e4:10 // kapot
// e0:5a:1b:e2:f0:68 wroom zonder dubbele antenne = {0xE0, 0x5A, 0x1B, 0xE2, 0xF0, 0x68};
// 30:ed:a0:b8:c6:4c esp32-s3-N16R8 = {0x30, 0xED, 0xA0, 0xB8, 0xC6, 0x4C}; // first test, pins soldered upwards
// 30:ED:A0:B7:29:10 esp32-s3-N16R8 = {0x30, 0xED, 0xA0, 0xB7, 0x29, 0x10}; // 2nd protofirst test, pins soldered downwards
//uint8_t broadcastAddress[] = {0x30, 0xED, 0xA0, 0xB8, 0xC6, 0x4C};
//uint8_t broadcastAddress[] = {0x30, 0xED, 0xA0, 0xB7, 0x29, 0x10};
//uint8_t broadcastGlobeMac[] = {0x98, 0xA3, 0x16, 0xE6, 0x9A, 0x1C}; 
//uint8_t GlobeMac[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}; 

// Variable to store if sending data was successful
char success[128];

esp_now_peer_info_t peerInfo = {};
const esp_now_peer_info_t *masterNode = &peerInfo;
int dataSent = 0;
int dataReceived = 0;

int ms_sleep = 0;
esp_err_t sendResult;

#define HIGHEST_CHANNEL	13    //Set according to country
#define LOWEST_CHANNEL  1

void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status);
void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *incomingData, int len);


// inspired by https://esp32.com/viewtopic.php?f=19&t=12992#p51338
bool setup_esp_now(void) 
{ bool bFirstPeer = true;
  char progress[64];
  uint8_t primaryChan;
  uint16_t revertChan;
  
  // first reset queues
  ToGlobe.QueueIndexIn = 0; // queue with messages from puck to globe
  ToGlobe.QueueIndexOut = 0;
  ToGlobe.QueueCnt = 0;
  FromGlobe.QueueIndexIn = 0; // queue with messages from globe for puck
  FromGlobe.QueueIndexOut = 0;
  FromGlobe.QueueCnt = 0;

  btStop(); // BT off, as recommended by Cellie
  WiFi.persistent(false);
//  WiFi.disconnect();
//  WiFi.mode(WIFI_OFF);
//  WiFi.mode(WIFI_OFF);
  
  delay(100);
  
   
  
  revertChan = DisplaySettings.wifichannel;
  if(revertChan==0)revertChan = 11;
  // first try with stored wifi channel to see if that connects with globe or gives a response from globe
  // when first channel fails, loop through all channels
  loopchannels:
  for (primaryChan = LOWEST_CHANNEL; primaryChan <= HIGHEST_CHANNEL; primaryChan++) 
  { if((DisplaySettings.wifichannel > 0) &&(DisplaySettings.wifichannel <= HIGHEST_CHANNEL))primaryChan = DisplaySettings.wifichannel;

    // test if((DisplaySettings.wifichannel > 0) &&(DisplaySettings.wifichannel <= HIGHEST_CHANNEL))primaryChan = 5;

    // show on main screen
    //sprintf(progress, "Connecting Wifi Channel %d", (uint16_t)primaryChan);
    //lv_label_set_text(ui_Station_Title, progress);
    //Lvgl_Loop();

    
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA); // WIFI_STA it is, as Puck is a real slave here
    if(!PuckMac[0] && !PuckMac[1] && !PuckMac[2] && !PuckMac[3] && PuckMac[4] && PuckMac[5])readMacAddress();
    Serial.printf("setup_esp_now() actual WiFi.channel() = %d\n", WiFi.channel());


    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
    ESP_ERROR_CHECK(esp_wifi_set_channel(primaryChan, secondChan));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));
    
    WiFi.printDiag(Serial);
    WiFi.disconnect();
    Serial.print("Try Wifi channel: "); Serial.println(WiFi.channel());
  
    // Init ESP-NOW

    if (esp_now_init() == ESP_OK) 
    {     // Register for a callback function that will be called when data is received
      esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
      esp_now_register_send_cb(OnDataSent);
      Serial.println("ESP-NOW INIT OK");
    }
    else 
    { Serial.println("ESP-NOW INIT FAILED....");
      return false;
    }

    // Register peer
    memcpy(peerInfo.peer_addr, DisplaySettings.globemac, 6);
    peerInfo.channel = primaryChan;  
    peerInfo.encrypt = false;
    peerInfo.ifidx = (wifi_interface_t) ESP_IF_WIFI_STA;

    if (bFirstPeer) 
    { if(esp_now_add_peer(masterNode) == ESP_OK) 
      { Serial.println("Added Peer!");
        bFirstPeer = false;
      }
    } 
    else 
    { if(esp_now_mod_peer(masterNode) == ESP_OK) 
      { Serial.println("Modified Peer!");
      }
    }

    

    dataSent = 0;
    dataReceived = 0;
    beepforMs(50);
    //Send test data
    if(TestSend())
    { //Wait for data sent confirmed
//      for (int i = 0; i < 1000; i++, ms_sleep--) 
      for (int i = 0; i < 100; i++) 
      { delay(10);
        yield();
        if((dataSent != 0 ) || dataReceived != 0)
        {  break; // data sent, stop waiting								
        }
      }
      
      if (dataSent == 1 || dataReceived)
      { Serial.printf("Found Master on channel: %d\n", WiFi.channel());
        beepforMs(50);
        if(DisplaySettings.wifichannel != WiFi.channel())
        { DisplaySettings.wifichannel = WiFi.channel();
          SaveDisplaySettingsToEeprom();
        }  
        break;
      }
      else // no luck
      { if(DisplaySettings.wifichannel==primaryChan)
        { DisplaySettings.wifichannel = 0; // stored channel didn't work
          goto loopchannels; // start all over
        }
      }  
    }
    delay(500);
  } // for loop

  if(primaryChan==14)
  { Serial.printf("No Master Found - revert to previously stored channel %d\n", revertChan);
    DisplaySettings.wifichannel = revertChan;
    
    primaryChan = DisplaySettings.wifichannel;
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  	wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
    ESP_ERROR_CHECK(esp_wifi_set_channel(primaryChan, secondChan));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));

    WiFi.printDiag(Serial);
    WiFi.disconnect();
    Serial.print("Try Wifi channel: "); Serial.println(WiFi.channel());
    // Register peer
    memcpy(peerInfo.peer_addr, DisplaySettings.globemac, 6);
    peerInfo.channel = primaryChan;  
    peerInfo.encrypt = false;
    if(esp_now_mod_peer(masterNode) == ESP_OK) 
    { Serial.printf("Modified Peer to channel %d\n", revertChan);
    }

  }
  //lv_label_set_text(ui_Station_Title, "");
  AddToQueueForGlobe("Puck Booted", MESSAGE_PUCK_ESP_NOW_INIT);
  return true;
}

bool setup_esp_add_peer(void) 
{ 
  
  if(DisplaySettings.globemac[0]==0)return false; 
  
  // Register peer
  memcpy(peerInfo.peer_addr, DisplaySettings.globemac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  esp_now_del_peer(peerInfo.peer_addr);
   
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return false;
  }

  Serial.printf("Added peer %02X:%02X:%02X:%02X:%02X:%02X\n", (uint16_t)DisplaySettings.globemac[0],(uint16_t)DisplaySettings.globemac[1],(uint16_t)DisplaySettings.globemac[2],(uint16_t)DisplaySettings.globemac[3],(uint16_t)DisplaySettings.globemac[4],(uint16_t)DisplaySettings.globemac[5]);
  
  return true;
}



bool Q_filling = 0;
bool Q_sending = 0;

void loop_esp_now() {
  // we get called here from the main loop

  if(DisplaySettings.globemac[0]==0)return; 

  while(Q_filling); // wait until idle
  Q_sending = true;

  // check for queued messages to send
  // Send message via ESP-NOW

  if(bUpAndRunning) 
  { //Serial.printf("ToGlobe.QueueCnt = %d\n", ToGlobe.QueueCnt);
    if(ToGlobe.QueueCnt>0)
    { while(ToGlobe.QueueCnt)
      { if(ToGlobe.QueueIndexOut != ToGlobe.QueueIndexIn)
        { DataFromDisplay.D_QueueSerialNumber++;
          strcpy(DataFromDisplay.D_QueueMessage, ToGlobe.QueueMessage[ToGlobe.QueueIndexOut]);
          DataFromDisplay.D_QueueMessageType = ToGlobe.QueueMessageType[ToGlobe.QueueIndexOut];
          Serial.printf("Message %d-%d sent to globe = >%s<\n", DataFromDisplay.D_QueueSerialNumber, DataFromDisplay.D_QueueMessageType, DataFromDisplay.D_QueueMessage);
          ToGlobe.QueueIndexOut++;
          //if(ToGlobe.QueueIndexOut>=QUEUESIZE)ToGlobe.QueueIndexOut = 0;
          ToGlobe.QueueIndexOut %= QUEUESIZE;
          ToGlobe.QueueCnt--;
          esp_err_t result = esp_now_send(DisplaySettings.globemac, (uint8_t *) &DataFromDisplay, sizeof(DataFromDisplay)); 
          // Serial.printf("Stuff (message) send\n");
        }
        // keep last one send
        memcpy(&PrevDataFromDisplay, &DataFromDisplay, sizeof(PrevDataFromDisplay));
      }
    }
    else
    { if(memcmp(&PrevDataFromDisplay, &DataFromDisplay,sizeof(PrevDataFromDisplay)) != 0)
      { esp_err_t result = esp_now_send(DisplaySettings.globemac, (uint8_t *) &DataFromDisplay, sizeof(DataFromDisplay)); 
        // Serial.printf("Stuff (usually volume or battery voltage) send\n");
        memcpy(&PrevDataFromDisplay, &DataFromDisplay, sizeof(PrevDataFromDisplay));
      }  
    }
    //Serial.printf("Send To peer %02X:%02X:%02X:%02X:%02X:%02X\n", (uint16_t)DisplaySettings.globemac[0],(uint16_t)DisplaySettings.globemac[1],(uint16_t)DisplaySettings.globemac[2],(uint16_t)DisplaySettings.globemac[3],(uint16_t)DisplaySettings.globemac[4],(uint16_t)DisplaySettings.globemac[5]);
  
  } 
  Q_sending = false;
}

void AddToQueueForGlobe(const char* message, uint16_t queuemessagetype) // one empty slot to avoid IndexIn becomes equal to IndexOut
{ 
  while(Q_sending); // wait until idle
  Q_filling = true;
  
  if(ToGlobe.QueueCnt < QUEUESIZE-1)
  { //if(ToGlobe.QueueIndexIn>=QUEUESIZE)ToGlobe.QueueIndexIn = 0;
    ToGlobe.QueueIndexIn %= QUEUESIZE;
    strncpy(ToGlobe.QueueMessage[ToGlobe.QueueIndexIn], message, QUEUEMESSAGELENGTH);
    ToGlobe.QueueMessage[ToGlobe.QueueIndexIn][QUEUEMESSAGELENGTH-1] = 0; // terminate just in case of idiotic long message
    ToGlobe.QueueMessageType[ToGlobe.QueueIndexIn] = queuemessagetype;
    Serial.printf("TELL GLOBE: %s -> %s\n", messagetexts[ToGlobe.QueueMessageType[ToGlobe.QueueIndexIn]], ToGlobe.QueueMessage[ToGlobe.QueueIndexIn]);

    ToGlobe.QueueIndexIn++;
    ToGlobe.QueueCnt++;
  }
  else Serial.println("Queue to globe is full!!!");
  Q_filling = false;
}

int TestSend(void) 
{ strcpy(DataFromDisplay.D_QueueMessage, ToGlobe.QueueMessage[ToGlobe.QueueIndexOut]);
  DataFromDisplay.D_QueueMessageType = ToGlobe.QueueMessageType[ToGlobe.QueueIndexOut];
  esp_err_t sendResult = esp_now_send(DisplaySettings.globemac, (uint8_t *) &DataFromDisplay, sizeof(DataFromDisplay)); 
	if (sendResult == ESP_OK) {
		Serial.println("TestSend success");
		return 1;
	} else if (sendResult == ESP_ERR_ESPNOW_NOT_INIT) {
		// How did we get so far!!
		Serial.println("ESPNOW not Init.");
	} else if (sendResult == ESP_ERR_ESPNOW_ARG) {
		Serial.println("Invalid Argument");
	} else if (sendResult == ESP_ERR_ESPNOW_INTERNAL) {
		Serial.println("Internal Error");
	} else if (sendResult == ESP_ERR_ESPNOW_NO_MEM) {
		Serial.println("ESP_ERR_ESPNOW_NO_MEM");
	} else if (sendResult == ESP_ERR_ESPNOW_NOT_FOUND) {
		Serial.println("Peer not found.");
	} 
	else if (sendResult == ESP_ERR_ESPNOW_IF) {
		Serial.println("Interface Error.");
	}   else {
		Serial.printf("\r\nNot sure what happened\t%d", sendResult);
	}
  return 0;
}



// Callback when data is sent
void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status){
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    strcpy(success, "Delivery Success :");
  }
  else{
    strcpy(success, "Delivery Fail :(");
  }
  dataSent = (status == ESP_NOW_SEND_SUCCESS ? 1 : -1);
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *incomingData, int len) 
{ static uint16_t MessageSerialNumber = -1;
  
  dataReceived = 1; // used for trial/error finding globe

  if(memcmp(PuckMac, rx_info->des_addr, sizeof(PuckMac)) == 0)
  { // globe delivering spot on
    //Serial.printf("OnDataRecv OK from Mac %02X:%02X:%02X:%02X:%02X:%02X\n", rx_info->src_addr[0], rx_info->src_addr[1], rx_info->src_addr[2], rx_info->src_addr[3], rx_info->src_addr[4], rx_info->src_addr[5]);  
    //Serial.printf("OnDataRecv OK to Mac %02X:%02X:%02X:%02X:%02X:%02X\n", rx_info->des_addr[0], rx_info->des_addr[1], rx_info->des_addr[2], rx_info->des_addr[3], rx_info->des_addr[4], rx_info->des_addr[5]);  
    // perhaps store globe mac, if different from stored one

  }
  else // not for us, should not happen
  { Serial.printf("OnDataRecv ?? from Mac %02X:%02X:%02X:%02X:%02X:%02X\n", rx_info->src_addr[0], rx_info->src_addr[1], rx_info->src_addr[2], rx_info->src_addr[3], rx_info->src_addr[4], rx_info->src_addr[5]);  
    Serial.printf("OnDataRecv ?? to Mac %02X:%02X:%02X:%02X:%02X:%02X\n", rx_info->des_addr[0], rx_info->des_addr[1], rx_info->des_addr[2], rx_info->des_addr[3], rx_info->des_addr[4], rx_info->des_addr[5]);  
    return;
  }

  memcpy(&DataFromGlobe, incomingData, sizeof(DataFromGlobe));
  // if new message, put in receiving queue
  if(MessageSerialNumber != DataFromGlobe.G_QueueSerialNumber)
  { MessageSerialNumber = DataFromGlobe.G_QueueSerialNumber;
    Serial.printf("DataFromGlobe.G_QueueSerialNumber = %d\n", DataFromGlobe.G_QueueSerialNumber);
    memcpy(&FromGlobe.QueueMessage[FromGlobe.QueueIndexIn], DataFromGlobe.G_QueueMessage, sizeof(DataFromGlobe.G_QueueMessage));
    FromGlobe.QueueMessageType[FromGlobe.QueueIndexIn] =  DataFromGlobe.G_QueueMessageType;
    FromGlobe.QueueIndexIn++;
    FromGlobe.QueueIndexIn %= QUEUESIZE;
    FromGlobe.QueueCnt++;
  }  

  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //Serial.println(sizeof(DataFromGlobe));
  //Serial.println(DataFromGlobe.ns);
  //Serial.println(DataFromGlobe.ew);
  //Serial.println(DataFromGlobe.Title);
}


