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
volatile bool bWasTransmitted = false;
void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{ //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  bWasTransmitted = (status == ESP_NOW_SEND_SUCCESS);
}


// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *incomingData, int len) 
{ static uint16_t MessageSerialNumberApi = -1;
  static uint16_t MessageSerialNumber = -1;
  static bool firstskipped = false; // skip the first message after (re)boot
  
  // this is just a silly check
  // Serial.printf("Received From Mac %02X:%02X:%02X:%02X:%02X:%02X\n", rx_info->src_addr[0], rx_info->src_addr[1], rx_info->src_addr[2], rx_info->src_addr[3], rx_info->src_addr[4], rx_info->src_addr[5]);  
  
  // Copy received raw data, nothing more, certainly not too much
  int safe_len = (len < sizeof(DataFromDisplay)) ? len : sizeof(DataFromDisplay);
  memcpy(&DataFromDisplay, incomingData, safe_len);
  // make sure message string is 0 terminated, just in case it is corrupt data, bitten once, never twice
  DataFromDisplay.D_QueueMessage[QUEUEMESSAGELENGTH - 1] = 0; 
  
  // quickly store gps location for timezone request before they become altered
//  if(PrevDataFromDisplay.D_QueueMessageType != DataFromDisplay.D_QueueMessageType && ((DataFromDisplay.D_QueueMessageType != MESSAGE_NOP) ))
  //if(PrevDataFromDisplay.D_QueueMessageType != DataFromDisplay.D_QueueMessageType && ((DataFromDisplay.D_QueueMessageType != MESSAGE_NOP) ))
  if((DataFromDisplay.D_QueueMessageType != MESSAGE_NOP) && (MessageSerialNumberApi != DataFromDisplay.D_QueueSerialNumberSend))
  { MessageSerialNumberApi = DataFromDisplay.D_QueueSerialNumberSend; // treat once, never twice
    DataFromGlobe.D_QueueSerialNumberReceived = MessageSerialNumberApi; // echo back
    if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_TIMEZONE_BY_GPS || DataFromDisplay.D_QueueMessageType == MESSAGE_HOME_TIMEZONE_NAME)
    { Serial.printf("PUCK SAYS -> MESSAGE_GET_TIMEZONE_BY_GPS <%s>\n", DataFromDisplay.D_QueueMessage);
      //PUCK SAYS -> MESSAGE_GET_TIMEZONE_BY_GPS <25 52.813301 6.090900>
      int16_t station;
      float nsGps;
      float ewGps;
      sscanf(DataFromDisplay.D_QueueMessage, "%d %f %f", &station, &nsGps, &ewGps);
      Serial.printf("  SSCANF-ED AS %d %f %f\n", station, nsGps, ewGps);

      CancelApiType(MESSAGE_GET_TIMEZONE_BY_GPS); // cancel previous ones
      CancelApiType(MESSAGE_HOME_TIMEZONE_NAME); // cancel previous ones

      ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexIn] =  DataFromDisplay.D_QueueMessageType;
      
      //ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_StationGpsNS;
      //ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_StationGpsEW;
      //ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_RequestedStation;
      
      ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexIn] = station;
      ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = nsGps;
      ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = ewGps;

      
      ApiCallsToDo.ApiQueueIndexIn++;
      ApiCallsToDo.ApiQueueIndexIn %= APIQUEUESIZE;
      ApiCallsToDo.ApiQueueCnt++;
      return; // don't add to queue
    }  
    else if((DataFromDisplay.D_QueueMessageType == MESSAGE_TIMEZONE_NAME))
    { Serial.printf("PUCK SAYS -> MESSAGE_GET_TIMEZONE <%s>\n", DataFromDisplay.D_QueueMessage);
      CancelApiType(MESSAGE_TIMEZONE_NAME);
      ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexIn] =  DataFromDisplay.D_QueueMessageType;

      ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.ns_cal/10.0;
      ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.ew_cal/10.0;
      ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_RequestedStation;

      ApiCallsToDo.ApiQueueIndexIn++;
      ApiCallsToDo.ApiQueueIndexIn %= APIQUEUESIZE;
      ApiCallsToDo.ApiQueueCnt++;
      return; // don't add to queue
    }  
    else if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION_BY_GPS)
    { Serial.printf("PUCK SAYS -> MESSAGE_GET_GEOLOCATION_BY_GPS <%s>\n", DataFromDisplay.D_QueueMessage);
      //PUCK SAYS -> MESSAGE_GET_GEOLOCATION_BY_GPS <25 52.813301 6.090900>
      int16_t station;
      float nsGps;
      float ewGps;
      sscanf(DataFromDisplay.D_QueueMessage, "%d %f %f", &station, &nsGps, &ewGps);
      Serial.printf("  SSCANF-ED AS %d %f %f\n", station, nsGps, ewGps);
      CancelApiType(MESSAGE_GET_GEOLOCATION_BY_GPS); 
      ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexIn] =  DataFromDisplay.D_QueueMessageType;
      
      //ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_StationGpsNS;
      //ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_StationGpsEW;
      //ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_RequestedStation;
      
      ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexIn] = station;
      ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = nsGps;
      ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = ewGps;
      
      ApiCallsToDo.ApiQueueIndexIn++;
      ApiCallsToDo.ApiQueueIndexIn %= APIQUEUESIZE;
      ApiCallsToDo.ApiQueueCnt++;
      return; // don't add to queue
    }  
    else if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_GEOLOCATION)
    { Serial.printf("PUCK SAYS -> MESSAGE_GET_GEOLOCATION <%s> (station %d )\n",  DataFromDisplay.D_QueueMessage, DataFromDisplay.D_RequestedStation);
      // PUCK SAYS -> MESSAGE_GET_GEOLOCATION <-1 33.900002 -38.299999> (station -1 )
      CancelApiType(MESSAGE_GET_GEOLOCATION);
      ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexIn] =  DataFromDisplay.D_QueueMessageType;
      ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.ns_cal/10.0;
      ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.ew_cal/10.0;
      ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.D_RequestedStation;
      ApiCallsToDo.ApiQueueIndexIn++;
      ApiCallsToDo.ApiQueueIndexIn %= APIQUEUESIZE;
      ApiCallsToDo.ApiQueueCnt++;
      return; // don't add to queue
    }  
    else if(DataFromDisplay.D_QueueMessageType == MESSAGE_GET_FLIGHT_DATA)
    { Serial.printf("PUCK SAYS -> MESSAGE_GET_FLIGHT_DATA <%s>\n", DataFromDisplay.D_QueueMessage);
      ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexIn] =  DataFromDisplay.D_QueueMessageType;
      ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.ns_cal/10.0;
      ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexIn] = DataFromDisplay.ew_cal/10.0;
      ApiCallsToDo.ApiQueueIndexIn++;
      ApiCallsToDo.ApiQueueIndexIn %= APIQUEUESIZE;
      ApiCallsToDo.ApiQueueCnt++;
      return; // don't add to queue
    }
    // not a special for api queue, continue..
  }  

  // put in receiving queue
  if((DataFromDisplay.D_QueueMessageType != MESSAGE_NOP) && (MessageSerialNumber != DataFromDisplay.D_QueueSerialNumberSend))
  { MessageSerialNumber = DataFromDisplay.D_QueueSerialNumberSend;
    DataFromGlobe.D_QueueSerialNumberReceived = MessageSerialNumber; // echo back
    // Serial.printf("DataFromDisplay.D_QueueSerialNumber = %d\n", DataFromDisplay.D_QueueSerialNumber);
    if(firstskipped)
    { memcpy(&FromDisplay.QueueMessage[FromDisplay.QueueIndexIn], DataFromDisplay.D_QueueMessage, sizeof(DataFromDisplay.D_QueueMessage));
      FromDisplay.QueueMessage[FromDisplay.QueueIndexIn][sizeof(DataFromDisplay.D_QueueMessage) - 1] = 0; // terminate for safety
      FromDisplay.QueueMessageType[FromDisplay.QueueIndexIn] =  DataFromDisplay.D_QueueMessageType;
      FromDisplay.QueueMessageSerialNumber[FromDisplay.QueueIndexIn] =  DataFromDisplay.D_QueueSerialNumberSend;
      
      Serial.printf("ESPNOW: OnDataRecv() -> DataFromDisplay.D_QueueSerialNumberSend = %d  - %s %s\n", FromDisplay.QueueMessageSerialNumber[FromDisplay.QueueIndexIn], messagetexts[FromDisplay.QueueMessageType[FromDisplay.QueueIndexIn]], FromDisplay.QueueMessage[FromDisplay.QueueIndexIn]);

      // atomic update of FromDisplay.QueueIndexIn, probably overly cautious since we run on same core
      uint16_t nextIndex = (FromDisplay.QueueIndexIn + 1) % QUEUESIZE;
      FromDisplay.QueueIndexIn = nextIndex; 
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
  // WiFi.mode(WIFI_AP_STA); // not good -> keeps an undesireable wifi AP active
  WiFi.mode(WIFI_STA); // 25 MAY 26 - just a station, not an acces point
  Serial.printf("setup_esp_now() WiFi.channel() = %d\n", WiFi.channel());

  uint8_t primaryChan = WiFi.channel();
  wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
  esp_wifi_set_channel(primaryChan, secondChan);
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));

  AddToQueueForDisplay("", MESSAGE_STATION_NAME);
  AddToQueueForDisplay("", MESSAGE_SONG_TITLE);
  AddToQueueForDisplay("", MESSAGE_TIMEZONE_NAME);

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

void loop_esp_now() 
{ static uint16_t wait = 10;
            
  // we get called here from the encoder loop, every 100mS
  
  while(Q_filling); // wait until idle
  Q_sending = true;
  // check and send all queued messages via ESP-NOW
  if(1) //bUpAndRunning)
  { if(ToDisplay.QueueCnt>0)
    { int max_to_send_attempts = 10; 
      while(ToDisplay.QueueCnt && max_to_send_attempts--) // don't let us hang in here forever, best effort here
      { if(ToDisplay.QueueIndexOut != ToDisplay.QueueIndexIn)
        { //DataFromGlobe.G_QueueSerialNumberSend++;
          //Serial.printf("ESPNOW esp_now_send() ToDisplay.QueueCnt %d -> ToDisplay.QueueIndexOut %d != ToDisplay.QueueIndexIn %d\n", ToDisplay.QueueCnt, ToDisplay.QueueIndexOut, ToDisplay.QueueIndexIn);

          DataFromGlobe.D_QueueMessageCount = ToDisplay.QueueCnt;
          // use memcpy, message can have embedded 0's - not always a traditional string 
          memcpy(DataFromGlobe.G_QueueMessage, ToDisplay.QueueMessage[ToDisplay.QueueIndexOut], QUEUEMESSAGELENGTH);   

          DataFromGlobe.G_QueueMessageType = ToDisplay.QueueMessageType[ToDisplay.QueueIndexOut];
          
          bWasTransmitted = false;
          esp_err_t result = esp_now_send(PuckMac, (uint8_t *) &DataFromGlobe, sizeof(DataFromGlobe));
          Serial.printf("ESPNOW esp_now_send() SERIALNR %d -> %s sent to display = >%s<\n", DataFromGlobe.G_QueueSerialNumberSend, messagetexts[DataFromGlobe.G_QueueMessageType]  , DataFromGlobe.G_QueueMessage);
          
          if(result==0) // && !bWasTransmitted) // all went ok
          { //Serial.printf("ESPNOW esp_now_send() succesfully queued\n");
            delay(2); // bWasTransmitted very likely true after this wait
            int wait = 25;
            while(wait-- && !bWasTransmitted ) // usually never happens, bWasTransitted already true;
            { Serial.printf("ESPNOW esp_now_send() wait for message actually send out\n");
              delay(1);
            }
            if(bWasTransmitted)
            { //Serial.printf("ESPNOW esp_now_send() package DataFromGlobe.G_QueueSerialNumber = %d -> delivered at puck\n", DataFromGlobe.G_QueueSerialNumberSend);
              Serial.printf("ESPNOW esp_now_send() DISPLAY CONFIRMED  %s -> delivered at puck\n", messagetexts[ToDisplay.QueueMessageType[ToDisplay.QueueIndexOut]]);
              ToDisplay.QueueIndexOut++;
              ToDisplay.QueueIndexOut %= QUEUESIZE;
              ToDisplay.QueueCnt--;
              DataFromGlobe.G_QueueSerialNumberSend++;
            }
            else
            { Serial.printf("ESPNOW Failed after 25mS waiting, try again..\n");
            }  
          } 
          else
          { Serial.printf("ESPNOW esp_now_send() could not queue (result=%d) message, will try again after 10 mS\n", result);
            delay(10);
          }
          delay(5);
        }
        // keep last one send
        memcpy(&PrevDataFromGlobe, &DataFromGlobe, sizeof(PrevDataFromGlobe));
        delay(5); // since we are in a wile loop, this seems to fix the odd lost packet of data
      }
    }
    else // keep sending last one, if changed with perhaps updated coordinates, rssi strength, whatever
    { if(memcmp(&PrevDataFromGlobe, &DataFromGlobe,sizeof(PrevDataFromGlobe)) != 0)
      { DataFromGlobe.G_QueueMessageType = MESSAGE_NOP; // don't want this to be picked by puck up as message with real job to do
        esp_err_t result = esp_now_send(PuckMac, (uint8_t *) &DataFromGlobe, sizeof(DataFromGlobe));
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

  if(ToDisplay.QueueCnt < (QUEUESIZE-1)) // at least one empty slot needed
  { ToDisplay.QueueIndexIn %= QUEUESIZE; // just to be sure
    //strncpy(ToDisplay.QueueMessage[ToDisplay.QueueIndexIn], message, QUEUEMESSAGELENGTH);
    memcpy(ToDisplay.QueueMessage[ToDisplay.QueueIndexIn], message, QUEUEMESSAGELENGTH);
    
    ToDisplay.QueueMessage[ToDisplay.QueueIndexIn][QUEUEMESSAGELENGTH-1] = 0; // terminate just in case of idiotic long message that could upset the puck decoding this
    ToDisplay.QueueMessageType[ToDisplay.QueueIndexIn] = queuemessagetype;
    
    Serial.printf("TELL DISPLAY (QueueIndexIn %d): %s -> %s\n", ToDisplay.QueueIndexIn, messagetexts[ToDisplay.QueueMessageType[ToDisplay.QueueIndexIn]], ToDisplay.QueueMessage[ToDisplay.QueueIndexIn]);

    ToDisplay.QueueIndexIn++;
    ToDisplay.QueueIndexIn %= QUEUESIZE;
    ToDisplay.QueueCnt++;
  }
  else Serial.println("Queue to display is full!!!");
  Q_filling = false;
}

