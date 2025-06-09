#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "time.h" 
const char* ntpServer = "pool.ntp.org";
time_t now;
int dstOffset = 0;
int rawOffset = 0;
char timeZoneId[64] = "?";
char timeZoneName[64] = "?";


/*
https://maps.googleapis.com/maps/api/timezone/json?location=52.21810%2C4.54510&timestamp=1747924919&key=AIzaSyCMxt7wLf_qSib7tMZVqKqUuiiS-ohCRpg
*/
const char gps_to_time_server[]   = "maps.googleapis.com";
const char gps_to_time_url[]   = "https://maps.googleapis.com/maps/api/timezone/json?location=";
const char gps_to_time_stamp[] = "&timestamp=";
const char gps_to_time_key[]   = "&key=AIzaSyCMxt7wLf_qSib7tMZVqKqUuiiS-ohCRpg";
char url_timezone_google_api[250]; // FB was 150 verhoogd naar 250 vanwwege vastlopen

void timetest(int16_t gps_ns, int16_t gps_ew)
{ WiFiClientSecure client;
  //String payload; 
  char   payload[256]; 
  String timeZoneId;
  String timeZoneName;
  String status;
  struct tm timeinfo;
  char content[128];
  bool print;
  
  print = 0;
  if(print)Serial.println("timestuff");

  time(&now); // UTC epoch time
  sprintf(url_timezone_google_api, "%s%ld.%ld%%2C%ld.%ld%s%ld", gps_to_time_url, gps_ns/10, abs(gps_ns%10), gps_ew/10, abs(gps_ew%10), gps_to_time_stamp, now);
  strcat(url_timezone_google_api, gps_to_time_key);
//  strcat(url_timezone_google_api, " HTTP/1.0");
  if(print)Serial.println(url_timezone_google_api);
  if(print)Serial.println(strlen(url_timezone_google_api));
  
  client.setInsecure();
//  if (!client.connect(gps_to_time_server, 443))
//    Serial.println("Connection failed!");
//  else 
//  { Serial.println("Connected to server!");
//    // Make a HTTP request:
// }

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
    status = doc["status"].as<String>();
    if(status == "OK")
    { dstOffset = doc["dstOffset"];
      rawOffset = doc["rawOffset"];
      timeZoneId = doc["timeZoneId"].as<String>();
      timeZoneName = doc["timeZoneName"].as<String>();
      if(print)Serial.println(dstOffset);
      if(print)Serial.println(rawOffset);
      if(print)Serial.println(timeZoneId);
      if(print)Serial.println(timeZoneName);
      sprintf(content, "TZ - %s", timeZoneId.c_str());
      lv_label_set_text(ui_Time_Zone, content);
    }
    else // at sea
    { dstOffset = 0;
      rawOffset = 0;
      timeZoneId = "TZ - Nautical";
      timeZoneName = "At Sea";
      if(print)Serial.println(dstOffset);
      if(print)Serial.println(rawOffset);
      if(print)Serial.println(timeZoneId);
      if(print)Serial.println(timeZoneName);
      lv_label_set_text(ui_Time_Zone, timeZoneId.c_str());
    }

  }

  // ignore areas at sea, only land locations have effect on the clock
  //if(status == "OK")
  { configTime(rawOffset, dstOffset, ntpServer);
    if(getLocalTime(&timeinfo))
    { // succes
      datetime.year = timeinfo.tm_year;
      datetime.month = timeinfo.tm_mon;
      datetime.day = timeinfo.tm_mday;
      datetime.dotw = timeinfo.tm_wday;
      datetime.hour = timeinfo.tm_hour; // + (timeinfo.tm_isdst>0)?1:0;
      datetime.minute = timeinfo.tm_min;
      datetime.second = timeinfo.tm_sec;
      //PCF85063_Set_Time(datetime);
      //PCF85063_Set_Date(datetime);
      PCF85063_Set_All(datetime);
          
//      printLocalTime(); 
    }
  }
  
  client.stop(); // stop insecure client
  if(print)Serial.print("Done Google query timezone");
  
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
