#include "Wireless.h"


bool WIFI_Connection = 0;
uint8_t WIFI_NUM = 0;
uint8_t BLE_NUM = 0;
bool Scan_finish = 0;
uint8_t NUM=0;
uint8_t wifi_scan()
{
  printf("/**********WiFi Test**********/\r\n");
  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  // Serial.println("Scan done");
  if (n == 0)
  {
    printf("no networks found\r\n");
  }
  else
  {
      printf("Nr | SSID                             | RSSI | CH | Encryption\r\n");
      for (int i = 0; i < 3; ++i)
      {
          // Print SSID and RSSI for each network found
          printf("%2d | ", i + 1);
          printf("%-32.32s | ", WiFi.SSID(i).c_str());
          printf("%4d | ", WiFi.RSSI(i));
          printf("%2d | ", WiFi.channel(i));
          switch (WiFi.encryptionType(i))
          {
          case WIFI_AUTH_OPEN:
              printf("open\r\n");
              break;
          case WIFI_AUTH_WEP:
              printf("WEP\r\n");
              break;
          case WIFI_AUTH_WPA_PSK:
              printf("WPA\r\n");
              break;
          case WIFI_AUTH_WPA2_PSK:
              printf("WPA2\r\n");
              break;
          case WIFI_AUTH_WPA_WPA2_PSK:
              printf("WPA+WPA2\r\n");
              break;
          case WIFI_AUTH_WPA2_ENTERPRISE:
              printf("WPA2-EAP\r\n");
              break;
          case WIFI_AUTH_WPA3_PSK:
              printf("WPA3\r\n");
              break;
          case WIFI_AUTH_WPA2_WPA3_PSK:
              printf("WPA2+WPA3\r\n");
              break;
          case WIFI_AUTH_WAPI_PSK:
              printf("WAPI\r\n");
              break;
          default:
              printf("unknown\r\n");
          }
          vTaskDelay(10);
      }
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  WiFi.disconnect(true); 
  WiFi.mode(WIFI_OFF);   
  vTaskDelay(100);           
  printf("/*******WiFi Test Over********/\r\n\r\n");
  printf("/**********BLE Test**********/\r\n"); 
}
BLEScan* pBLEScan;
uint8_t ble_scan()
{

  if(NUM == 0){
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
  }
  BLEScanResults* foundDevices = pBLEScan->start(5);
  int count = foundDevices->getCount();
  printf("Found %d devices\r\n",count);
  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    if (device.haveName()) {
      printf("Bluetooth Name      : %s\r\n",device.getName().c_str());
      printf("Device address      : %s\r\n",device.getAddress().toString().c_str());
      printf("Response Length     : %d\r\n",device.getPayloadLength());
      printf("\r\n");
      NUM++;
    }
    if(NUM == 2)
    {
      break;
    }
  }
  if(NUM == 2)
  {
    pBLEScan->stop(); 
    pBLEScan->clearResults();
    BLEDevice::deinit(true); 
    printf("/**********BLE Test Over**********/\r\n\r\n");
  }
  return NUM;
}
void Wireless_Test1(){
  uint8_t Time=0;
  pinMode(0, INPUT);                    
  while(digitalRead(0) != 0)         
  {
    vTaskDelay(10);
    Time++;
    if(Time == 100)
    {
      printf("Please press the BOOT button \r\n");
      Time=0;
    }
  }
  wifi_scan();
  uint8_t ble_NUM = ble_scan();
  while(ble_NUM != 2)
    ble_NUM = ble_scan();
    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int wifi_scan_number()
{
  printf("/**********WiFi Test**********/\r\n");
  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);                           
  WiFi.setSleep(true);     
  // WiFi.scanNetworks will return the number of networks found.
  int count = WiFi.scanNetworks();
  if (count == 0)
  {
    printf("No WIFI device was scanned\r\n");
  }
  else{
    printf("Scanned %d Wi-Fi devices\r\n",count);
  }
  
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  WiFi.disconnect(true); 
  WiFi.mode(WIFI_OFF);  
  vTaskDelay(100);         
  printf("/*******WiFi Test Over********/\r\n\r\n");
  return count;
}
int ble_scan_number()
{
  printf("/**********BLE Test**********/\r\n"); 
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  BLEScanResults* foundDevices = pBLEScan->start(5);  
  vTaskDelay(5000);       
  int count = foundDevices->getCount();
  if (count == 0)
  {
    printf("No Bluetooth device was scanned\r\n");
  }
  else{
    printf("Scanned %d Bluetooth devices\r\n",count);
  }
  pBLEScan->clearResults(); 
  pBLEScan->stop(); 
  BLEDevice::deinit(true); 
  vTaskDelay(100);         
  printf("/**********BLE Test Over**********/\r\n\r\n");
  return count;
}

void WirelessScanTask(void *parameter) {
  WIFI_NUM = wifi_scan_number();
  BLE_NUM = ble_scan_number();
  Scan_finish = 1;
  vTaskDelete(NULL);
}
void Wireless_Test2(){
  xTaskCreatePinnedToCore(
    WirelessScanTask,     
    "WirelessScanTask",   
    4096,                
    NULL,                 
    1,                    
    NULL,                
    0                    
  );
}
