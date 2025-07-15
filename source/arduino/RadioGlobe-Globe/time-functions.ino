#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h" 

/*
https://maps.googleapis.com/maps/api/timezone/json?location=52.21810%2C4.54510&timestamp=1747924919&key=YOUR-API-KEY-FROM-GOOGLE
https://maps.googleapis.com/maps/api/geocode/json?latlng=53,5&key=YOUR-API-KEY-FROM-GOOGLE
*/


const char* ntpServer = "pool.ntp.org";
//const char* ntpServer = "time.google.com";

const char gps_to_time_url[] = "https://maps.googleapis.com/maps/api/timezone/json?location=";
const char gps_to_geocoding[] = "https://maps.googleapis.com/maps/api/geocode/json?latlng=";

// as defined in ..\secrets.h
// const char google_api_key[] = "YOUR-GOOGLE-API-KEY";

char url_google_api[250];

void GetTimeZone(float StationGpsNS, float StationGpsEW)
{ WiFiClientSecure client;
  time_t now;
  char   payload[256]; 
  char   content[256]; 
  int dstOffset = 0;
  int rawOffset = 0;
  
  struct tm timeinfo;
  bool print;
  
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   

  print = 0;
  
  //if(print)
  Serial.printf("GetTimeZone Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

  time(&now); // UTC epoch time
  sprintf(url_google_api, "%s%.6f%%2C%.6f&timestamp=%lld&key=%s", gps_to_time_url, StationGpsNS, StationGpsEW, now, google_api_key);
  
  if(print)Serial.println(url_google_api);
  client.setInsecure();

  HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, url_google_api);
  //if(print)Serial.println("HTTP Client gestart");
  int httpResponseCode = https.GET();
  if (httpResponseCode>0) 
  { if(print)Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    strcpy(payload, https.getString().c_str());
    if(print)Serial.println(payload);
  }  
  else 
  {
    if(print)Serial.printf("Error code: %d\n", httpResponseCode);
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
      // use these offsets 
      configTime(rawOffset, dstOffset, ntpServer);
      getLocalTime(&DataFromGlobe.timeinfo);
      //sprintf(content, "TZN - %s", (const char*)doc["timeZoneName"]); 
      sprintf(content, "TZ - %s", (const char*)doc["timeZoneId"]); 
      AddToQueueForDisplay(content, MESSAGE_TIMEZONE_ID);
      if(print)Serial.println(content);
      if(print)printLocalTime();      
    }
    else // at sea
    { dstOffset = 0;
      rawOffset = 0;
      // use these offsets 
      configTime(rawOffset, dstOffset, ntpServer);
      getLocalTime(&DataFromGlobe.timeinfo);
      AddToQueueForDisplay("TZ - Nautical", MESSAGE_TIMEZONE_ID);
      //printLocalTime();
    }
  }

  
  if(print)Serial.print("Done Google query timezone\n");
  // Serial.println("MFree Heap at End GetTimeZone()" + String(ESP.getFreeHeap()));   
  
}

void GetGeolocationData(float StationGpsNS, float StationGpsEW)
{ WiFiClientSecure client;
  char   payload[256]; 
  char   countrycode[3]=""; 
  
  bool print;
  
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   

  print = 0;
  
  if(print)Serial.printf("GetGeoLocationData Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

  sprintf(url_google_api, "%s%.6f%%2C%.6f&key=%s", gps_to_geocoding, StationGpsNS, StationGpsEW, google_api_key);
  
  if(print)Serial.println(url_google_api);
  client.setInsecure();

  HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, url_google_api);
  //if(print)Serial.println("HTTP Client gestart");
  int httpResponseCode = https.GET();
  if (httpResponseCode>0) 
  { client.setTimeout(100);
    char *p;
    while(client.available())
    { uint16_t cnt = client.readBytesUntil('\n', payload, sizeof(payload)); // or until client.setTimeout(100)
      payload[cnt]=0;
      //Serial.printf("cnt=%d ->%s\n", cnt, payload);
      if((p = strrchr(payload, '\"')) != NULL)*p=0; // get rid of the last "
      if((p = strstr(payload, "\"short_name\" : \"")) != NULL)
      { p+=16;
        //Serial.println(p);
        if(strlen(p)==2)strcpy(countrycode, p);
      }
      if((p = strstr(payload, "\"country")) != NULL)break;
    }
  }  
  else 
  {
    if(print)Serial.printf("Error code: %d\n", httpResponseCode);
  }
  if(print)Serial.printf("GetGeolocationData (%s)done.", countrycode);
  https.end();
  client.stop(); // stop insecure client

  if (httpResponseCode>0)
  { AddToQueueForDisplay(countrycode, MESSAGE_GET_GEOLOCATION_BY_GPS);
  }
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

// Http tested as ok: http://stream3.broadcast-associes.com:8405/Radio-Orient.m3u  -> http://stream3.broadcast-associes.com:8405/Radio-Orient (url retrieved from .m3u )
// dit is een 302 redirector http://netradio.radio4.dk/radio4

bool CheckForRedirectsHttp(void)
{ WiFiClient client;
  HTTPClient http;
  String redirectURL;
  bool redirected = false;
  const char *headerKeys[] = {"Location"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
 
  http.begin(UnraveledUrl);
  http.collectHeaders(headerKeys, headerKeysCount);

  int statusCode = http.GET();
  Serial.printf("REDIRCHECK HTTP Response code: %d\n", statusCode);
  if (statusCode == 301 || statusCode == 302 || statusCode == 307 || statusCode == 308)
  { String redirectURL = http.header("Location");
    if(redirectURL.length()) 
    {  Serial.print("REDIRCHECK URL= ");Serial.println(redirectURL);
       Serial.print("Redirecting to: ");
       Serial.println(redirectURL);
       strcpy(UnraveledUrl, redirectURL.c_str());
       Serial.println(UnraveledUrl);
       Serial.println("Dat is m");
       redirected = true;
    }  
  }
  return redirected;
}

bool CheckForRedirectsHttps(void)
{ WiFiClientSecure client;
  HTTPClient https;
  String redirectURL;
  bool redirected = false;
  const char *headerKeys[] = {"Location"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
  char *p;

  client.setInsecure();
  Serial.printf("REDIRCHECK HTTPS Start - headerkeys ophalen\n");
  https.begin(UnraveledUrl);
  https.collectHeaders(headerKeys, headerKeysCount);

  int statusCode = https.GET();
  Serial.printf("REDIRCHECK HTTPS Response code: %d\n", statusCode);
  if (statusCode == 301 || statusCode == 302 || statusCode == 307 || statusCode == 308) 
  { String redirectURL = https.header("Location");
    if(redirectURL.length()) 
    {  Serial.print("REDIRCHECK HTTPS URL -> ");Serial.println(redirectURL);
       // wtf REDIRCHECK URL= https://emf-seg001267.cdnstream1.com/seg001267-i64.aac?platform=web&city=haarlem&country=netherlands&geo_api=abstract_ip&latitude=52.381&longitude=4.6275&opawg_device_category=computer&opawg_device_name=windows-computer&region=north-holland&segment=001267&t=1&k=m-OLaEh-ndJBWEHUEGxV7A35CuEDgHDioZA5nZIrVW_7V9vwXDD8bMy0xJOTDL8EcJtXGH6g4pT9hQPZmRcnsXhGEt1nHIKwha06pWOjnT-ttNz3tYQm6UlvnpiYfS3LnlGs0Q
       strcpy(UnraveledUrl, redirectURL.c_str());
       Serial.println(UnraveledUrl);
       redirected = true;
    }  
  }
  return redirected;
}

uint16_t UnravelHttpUrl(void)
{ //WiFiClientSecure client;
  WiFiClient client;
  HTTPClient https;
  int httpResponseCode;
  char *url = UnraveledUrl;

  bool print;
  char *pu;
  char *pl;
  uint16_t result = 0;  
  uint16_t n;  
  char line[128] = "nada";
  char* lp;
  print = 1;


  strcpy(UnraveledUrl, RequestedUrl); 
  if(print)Serial.printf("UNRAVEL %s\n", url);

  
  //client.setInsecure();

  if(1) //( (p = strstr(url, ".m3u")) != NULL || (p = strstr(url, ".pls")) != NULL)
  { // urls packed in text file

    if(print)Serial.println("UNRAVEL HTTP Client starten");
    https.begin(client, url);
    if(print)Serial.println("UNRAVEL HTTP Client gestart");
    httpResponseCode = https.GET();
    if (httpResponseCode>0) 
    { if(print)Serial.printf("UNRAVEL HTTP Response code: %d\n", httpResponseCode);
    }  
    else 
    { if(print)Serial.printf("UNRAVEL Error code: %d\n", httpResponseCode);
      return 0;
    }

    if (httpResponseCode==200)
    {  Serial.print("client.available() returns:");Serial.println(client.available());
      // todo 302, 307 redirect
      // 400   
      
      // typical response content for http://icecast.omroep.nl/3fm-bb-mp3.m3u is a single url:
      // http://icecast.omroep.nl:80/3fm-bb-mp3
      //
      // typical response content for http://playerservices.streamtheworld.com/pls/RADIO10.pls is a list of urls
      // [playlist]
      // File1=http://25353.live.streamtheworld.com:80/RADIO10_SC
      // File2=http://25353.live.streamtheworld.com:3690/RADIO10_SC
      // File3=http://29053.live.streamtheworld.com:80/RADIO10_SC
      // File4=http://29053.live.streamtheworld.com:3690/RADIO10_SC
      // ....often much more, even a few hundred is normal
      // File166=http://27863.live.streamtheworld.com:3690/RADIO10_SC
      // File167=http://29023.live.streamtheworld.com:80/RADIO10_SC
      // File168=http://29023.live.streamtheworld.com:3690/RADIO10_SC
      // Title1=RADIO10_SC
      // Title2=RADIO10_SC-Bak
      // NumberOfEntries=168

      if((pu = strstr(url, ".m3u")) != NULL) 
      { while(client.available())
        { pl = line;
          n = 0;
          while(client.available() && n++< (sizeof(line)-1)) 
          { *pl = client.read();
            if(*pl == '\n')
            { *pl =0;
               break;
            }
            else *++pl=0;
          }
          // full line of text loaded   
          if((pl = strstr(line, "http://")) != NULL)
          { strcpy(url, pl);
            result = 1;
            break;
          }
          if((pl = strstr(pl, "https://")) != NULL)
          { strcpy(url, line);
            result = 1;
           break;
          }
       }
        Serial.printf("m3u->%s<\n", url);
      }
      else if((pu = strstr(url, ".pls")) != NULL)
      { while(client.available())
        { pl = line;
          n = 0;
          while(client.available() && n++< (sizeof(line)-1)) 
          { *pl = client.read();
            if(*pl == '\n')
            { *pl =0;
               break;
            }
            else *++pl=0;
          }
          // full line of text loaded   
          // example -> File1=http://27903.live.streamtheworld.com:80/RADIO10_SC
          if((pl = strstr(line, "http://")) != NULL)
          { strcpy(url, pl);
            result = 1;
            break;
          }
          if((pl = strstr(line, "https://")) != NULL)
          { strcpy(url, pl);
            result = 1;
           break;
          }
       }
        Serial.printf("pls->%s<\n", url);
      }
      else
      { // mp3, oog, aac, nothing, assume ok for now 
        result = 1;  
      }
    }
    else 
    { if(print)Serial.printf("UNRAVEL HTTP code error : %d\n", httpResponseCode);
      // seen a 301 for https://lyd.nrk.no/nrk_radio_mp3_mp3_h
    }    
    https.end();
    client.stop(); // stop insecure client
  }




//  if( (pu = strstr(url, ".m3u8")) != NULL) result = 0; // voorlopig weg ermee

  // misschien een connect check dan verslikt url,begin zich niet later?
 
  if(print)Serial.printf("UNRAVEL Done with result=%d\n", result);
  return result;
  
}

uint16_t UnravelHttpsUrl(void)
{ WiFiClientSecure client;
  //WiFiClient client;
  HTTPClient https;
  int httpResponseCode;
  char *url = UnraveledUrl;

  bool print;
  char *p;
  uint16_t result = 0;  
  char line[128];
  char* lp;
  print = 1;

  strcpy(UnraveledUrl, RequestedUrl); 
  if(print)Serial.printf("UNRAVEL@ %s\n", url);

  
  client.setInsecure();

  if(1) //( (p = strstr(url, ".m3u")) != NULL || (p = strstr(url, ".pls")) != NULL)
  { // urls packed in text file

    if(print)Serial.println("UNRAVEL@ HTTP Client starten");
    https.begin(client, url);
    if(print)Serial.println("UNRAVEL@ HTTP Client gestart");
    httpResponseCode = https.GET();
    if (httpResponseCode>0) 
    { if(print)Serial.printf("UNRAVEL@ HTTP Response code: %d\n", httpResponseCode);
    }  
    else 
    { if(print)Serial.printf("UNRAVEL@ Error code: %d\n", httpResponseCode);
      return 0;
    }

    if (httpResponseCode==200)
    { // todo 302, 307 redirect
      // 400   
      
      // typical response content for http://icecast.omroep.nl/3fm-bb-mp3.m3u is a single url:
      // http://icecast.omroep.nl:80/3fm-bb-mp3
      //
      // typical response content for http://playerservices.streamtheworld.com/pls/RADIO10.pls is a list of urls
      // [playlist]
      // File1=http://25353.live.streamtheworld.com:80/RADIO10_SC
      // File2=http://25353.live.streamtheworld.com:3690/RADIO10_SC
      // File3=http://29053.live.streamtheworld.com:80/RADIO10_SC
      // File4=http://29053.live.streamtheworld.com:3690/RADIO10_SC
      // ....often much more, even a few hundred is normal
      // File166=http://27863.live.streamtheworld.com:3690/RADIO10_SC
      // File167=http://29023.live.streamtheworld.com:80/RADIO10_SC
      // File168=http://29023.live.streamtheworld.com:3690/RADIO10_SC
      // Title1=RADIO10_SC
      // Title2=RADIO10_SC-Bak
      // NumberOfEntries=168

      if((p = strstr(url, ".m3u")) != NULL)
      { // *p=0; // get rid of extension
        int n = 0;
        while(client.available()) 
        { line[n] = client.read();
          Serial.print(line[n]);
          if(line[n] == '\n')
          { line[n] = 0;
//            Serial.printf("m3u@->%s<\n", line);
            if((p = strstr(line, "http://")) != NULL)
            { strcpy(url, p);
              result = 1;
              break;
            }
            else if((p = strstr(line, "https://")) != NULL)
            { strcpy(url, p);
              result = 1;
              break;
            }
            n=0;
          }
          n++;
          delay(5);
        }
        result = 1;
      }
      else if((p = strstr(url, ".pls")) != NULL)
      { *p=0; // get rid of extension
        int n = 0;
        while(client.available()) 
        { line[n] = client.read();
          if(line[n] == '\n')
          { line[n] = 0;
            Serial.printf("pls@->%s<\n", line);
            if((p = strstr(line, "http://")) != NULL)
            { strcpy(url, p);
              Serial.printf("pls->%s<\n", url);
              result = 1;
              break;
            }
            else if((p = strstr(line, "https://")) != NULL)
            { strcpy(url, p);
              Serial.printf("pls->%s<\n", url);
              result = 1;
              break;
            }
            n=0;
          }
          n++;
          delay(5);
        }  
      }
      else
      { // mp3, oog, aac, nothing, assume ok
        result = 1;  
      }
    }
    else 
    { if(print)Serial.printf("UNRAVEL@ HTTP code error : %d\n", httpResponseCode);
      if (httpResponseCode==302) // dirty fix for https://stream.zeno.fm/dahlxvtaz1guv that keeps returning redirects
      { result =1;
      }
      // seen a 301 for https://lyd.nrk.no/nrk_radio_mp3_mp3_h
    }    
    https.end();
    client.stop(); // stop insecure client
  }

  if( (p = strstr(url, ".m3u8")) != NULL) result = 0; // weg ermee

 
  if(print)Serial.print("UNRAVEL@ Done\n");
  return result;
  
}



