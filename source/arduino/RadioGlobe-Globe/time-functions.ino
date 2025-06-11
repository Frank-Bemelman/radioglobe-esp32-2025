#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "time.h" 

/*
You have to get your own key at google cloud platform
https://maps.googleapis.com/maps/api/timezone/json?location=52.21810%2C4.54510&timestamp=1747924919&key=YOURKEY
*/

const char* ntpServer = "pool.ntp.org";
const char gps_to_time_url[]   = "https://maps.googleapis.com/maps/api/timezone/json?location=";
const char gps_to_time_stamp[] = "&timestamp=";
const char gps_to_time_key[]   = "YOURKEY";
char url_timezone_google_api[250];

void GetTimeZone(int16_t gps_ns, int16_t gps_ew)
{ WiFiClientSecure client;
  time_t now;
  char   payload[256]; 
  char   content[256]; 
  int dstOffset = 0;
  int rawOffset = 0;

  struct tm timeinfo;
  bool print;
  
  print = 1;
  if(print)Serial.println("GetTimeZone");
  sprintf(content, "From Display NS-CAL = %d, EW-CAL = %d", gps_ns, gps_ew);
  Serial.println(content);

  time(&now); // UTC epoch time
  sprintf(url_timezone_google_api, "%s%ld.%ld%%2C%ld.%ld%s%ld", gps_to_time_url, gps_ns/10, abs(gps_ns%10), gps_ew/10, abs(gps_ew%10), gps_to_time_stamp, now);
  strcat(url_timezone_google_api, gps_to_time_key);
  if(print)Serial.println(url_timezone_google_api);
  if(print)Serial.println(strlen(url_timezone_google_api));
  
  client.setInsecure();

  HTTPClient https;
  if(print)Serial.println("HTTP Client starten");
  https.begin(client, url_timezone_google_api);
  if(print)Serial.println("HTTP Client gestart");
  int httpResponseCode = https.GET();
  if (httpResponseCode>0) 
  { if(print)Serial.print("HTTP Response code: ");
    if(print)Serial.println(httpResponseCode);
    strcpy(payload, https.getString().c_str());
    if(print)Serial.println(payload);
  }  
  else 
  {
    if(print)Serial.print("Error code: ");
    if(print)Serial.println(httpResponseCode);
  }
  https.end();
  client.stop(); // stop insecure client

  if (httpResponseCode>0)
  { // typical response content
    //{
    //"dstOffset" : 0,
    //"rawOffset" : 3600,
    //"status" : "OK",
    //"timeZoneId" : "Africa/Ndjamena",
    //"timeZoneName" : "West Africa Standard Time"
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if(strcmp(doc["status"],"OK") == 0)
    { dstOffset = doc["dstOffset"];
      rawOffset = doc["rawOffset"];
      
      sprintf(DataFromGlobe.TimeZoneId, "TZ - %s", (const char*)doc["timeZoneId"]); 
      sprintf(content, "TZN - %s", (const char*)doc["timeZoneName"]); 

      if(print)Serial.println(dstOffset);
      if(print)Serial.println(rawOffset);
      if(print)Serial.println(DataFromGlobe.TimeZoneId);
      if(print)Serial.println(content);
    }
    else // at sea
    { dstOffset = 0;
      rawOffset = 0;
      strcpy(DataFromGlobe.TimeZoneId, "TZ - Nautical"); //      timeZoneId = "TZ - Nautical";
      if(print)Serial.println(dstOffset);
      if(print)Serial.println(rawOffset);
    }
    // use the offsets 
    configTime(rawOffset, dstOffset, ntpServer);
    getLocalTime(&DataFromGlobe.timeinfo);
  }

  
  if(print)Serial.print("Done Google query timezone\n");
  
}



void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();

  time_t now;
  char txt[32];
  time(&now);
  sprintf(txt, "UTC = %ld\n", (long)now);
  Serial.println(txt);


}
