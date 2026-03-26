#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h" 


/*
https://maps.googleapis.com/maps/api/timezone/json?location=52.21810%2C4.54510&timestamp=1747924919&key=YOUR-API-KEY-FROM-GOOGLE
https://maps.googleapis.com/maps/api/geocode/json?latlng=53,5&key=YOUR-API-KEY-FROM-GOOGLE
https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key}
*/


const char* ntpServer = "pool.ntp.org";
//const char* ntpServer = "time.google.com";

const char gps_to_time_url[] = "https://maps.googleapis.com/maps/api/timezone/json?location=";
const char gps_to_geocoding[] = "https://maps.googleapis.com/maps/api/geocode/json?latlng=";

// as defined in ..\secrets.h
// const char google_api_key[] = "YOUR-GOOGLE-API-KEY";

char api_url[250];
char exchangepayload[8192];

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
  sprintf(api_url, "%s%.6f%%2C%.6f&timestamp=%lld&key=%s", gps_to_time_url, StationGpsNS, StationGpsEW, now, GlobeSettings.google_api_key);
  
  if(print)Serial.println(api_url);
  client.setInsecure();

  HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, api_url);
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
  char   payload[512]; // 256->512
  char   shortname[128]; // 64->128
  //char   longname[64];
  char   countrycode[64]="XX"; 
  char   town[128]="???"; // 64->128
  uint16_t searches;
  bool found = false;
  bool townfound = false;
  bool countryfound = false;
  
  bool print;
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   
  print = 0;
  
  //if(print)
  Serial.printf("GetGeoLocationData Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

  // examples
  // https://maps.googleapis.com/maps/api/geocode/json?latlng=42.464699,21.466900&key=YOUR-API-KEY-FROM-GOOGLE
  // https://maps.googleapis.com/maps/api/geocode/json?latlng=19.286900,-81.367401&key=YOUR-API-KEY-FROM-GOOGLE


  sprintf(api_url, "%s%.6f%%2C%.6f&key=%s", gps_to_geocoding, StationGpsNS, StationGpsEW, GlobeSettings.google_api_key);

  // get ocean name??
  // https://maps.googleapis.com/maps/api/geocode/json?latlng=19.286900,-81.367401&result_type=natural_feature&key=YOUR-API-KEY-FROM-GOOGLE
  // sprintf(api_url, "%s%.6f%%2C%.6f&result_type=natural_feature&key=%s", gps_to_geocoding, StationGpsNS, StationGpsEW, GlobeSettings.google_api_key); // does not give name of sea

  // to investigate, getting the name of sea or ocean
  //  api.geonames.org/oceanJSON?lat=40.78343&lng=-43.96625&username=demo
  //  as seen here https://www.geonames.org/export/web-services.html#ocean
  
  if(print)Serial.println(api_url);
  client.setInsecure();

  HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, api_url);
  //if(print)Serial.println("HTTP Client gestart");
  int httpResponseCode = https.GET();
  if (httpResponseCode>0) 
  { client.setTimeout(100);
    char *p;
    while(client.available() && !found)
    { uint16_t cnt = client.readBytesUntil('\n', payload, sizeof(payload)); // or until client.setTimeout(100)
      payload[cnt]=0;
      if(print)Serial.printf("cnt=%d ->%s\n", cnt, payload);
      if((p = strrchr(payload, '\"')) != NULL)*p=0; // get rid of the last ",
      if((p = strstr(payload, "\"short_name\" : \"")) != NULL)
      { p+=16;
        //Serial.println(p);
        strcpy(shortname, p); // contains town name or countrycode
      }
//      if((p = strstr(payload, "\"long_name\" : \"")) != NULL)
//      { p+=15;
//        //Serial.println(p);
//        strcpy(longname, p);
//      }
//      if((p = strstr(payload, "\"administrative_area_level_1")) != NULL)
//      { strcpy(town, longname); // that's not the town, more like an area name
//        searches--;
//      }
      if((p = strstr(payload, "\"locality")) != NULL)
      { strcpy(town, shortname); // in some countries, this is can be weird unicode-character text like thai จ.นครสวรรค์
        townfound = true;
      }
      if((p = strstr(payload, "\"country")) != NULL)
      { strcpy(countrycode, shortname);
        countrycode[2] = 0; // shorten to 2 characters just in case
        countryfound = true;
      }
      if(townfound && countryfound)found=true;
    }
    sprintf(payload, "%s,%s", countrycode, town);
    Serial.printf("GetGeolocationData() result -> >%s<\n", payload);
  }  
  else 
  { sprintf(payload, "%s,%s", countrycode, town);
    Serial.printf("GetGeolocationData() -> Error code: %d\n", httpResponseCode);
  }

  https.end();
  client.stop(); // stop insecure client

  // use this country code if we go play music from SD
  strcpy(CountryCodeSelectorSD, countrycode);
  if(DataFromGlobe.FindGeoLocationData == MESSAGE_GET_GEOLOCATION_BY_GPS)AddToQueueForDisplay(payload, MESSAGE_GET_GEOLOCATION_BY_GPS);
  else if(DataFromGlobe.FindGeoLocationData == MESSAGE_GET_GEOLOCATION)AddToQueueForDisplay(payload, MESSAGE_GET_GEOLOCATION);

}

void GetOpenWeatherData(float StationGpsNS, float StationGpsEW)
{ WiFiClientSecure client;
  char   payload[1024]; 
  char   countrycode[3]=""; 
  char   content[256]; 

  
  bool print;
  
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   

  print = 0;
  
  if(print)Serial.printf("GetOpenWeatherData Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

  sprintf(api_url, "https://api.openweathermap.org/data/2.5/weather?lat=%.4f&lon=%.4f&appid=%s&units=metric", StationGpsNS, StationGpsEW, GlobeSettings.open_weather_map_api_key);
  
  if(print)Serial.println(api_url);
  client.setInsecure();

  HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, api_url);
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
  { // typical response content - see https://openweathermap.org/current
    // "weather": [
    //  {
    //     "id": 501,
    //     "main": "Rain",
    //     "description": "moderate rain",
    //     "icon": "10d"
    //  }
    //],
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if(!error)
    { float temperature = doc["main"]["temp"];
		  int humidity = doc["main"]["humidity"];
			String icon = doc["weather"][0]["icon"];
      sprintf(content, "Temp %.1f Rh %d Icon %s", temperature, humidity, icon); 
      AddToQueueForDisplay(content, MESSAGE_STATION_WEATHER_DATA);
    }
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


void FetchJsonExchangeRates(void)
{ WiFiClientSecure client;
  int country_index;

  if((country_index=IsCountryCodeValid(GlobeSettings.HomeCountryCode))<0)
  { Serial.printf("Can't get exchange rates for unknown country %s\n", GlobeSettings.HomeCountryCode);
    return;
  }

  if(strlen(CountryList[country_index].valutacode)<3)
  { Serial.printf("Can't get exchange rates for valuta -> %s\n", CountryList[country_index].valutacode);
    return;
  }

  bool print;
  print = 1;
  
  if(print)Serial.printf("GetExchangeRates for %s\n", CountryList[country_index].valutacode);

  sprintf(api_url, "https://open.er-api.com/v6/latest/%s", CountryList[country_index].valutacode);
  
  if(print)Serial.println(api_url);
  client.setInsecure();

  HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, api_url);
  //if(print)Serial.println("HTTP Client gestart");
  int httpResponseCode = https.GET();
  if (httpResponseCode>0) 
  { if(print)Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    strcpy(exchangepayload, https.getString().c_str());
    if(print)Serial.println(exchangepayload);
    if(print)Serial.printf("length exchangepayload = %d\n", strlen(exchangepayload));
  }  
  else 
  { exchangepayload[0]=0;
    if(print)Serial.printf("Error code: %d\n", httpResponseCode);
  }
  https.end();
  client.stop(); // stop insecure client
  DecodeExchangeJson();
}

void DecodeExchangeJson(void)
{ if(strlen(exchangepayload)>0)
  { Serial.println("Deserialization Start");
    StaticJsonDocument<8000> doc;
    DeserializationError error = deserializeJson(doc, exchangepayload);
    if(!error)
    { float rate = 3.1415;    
      for(int n=0; n<(sizeof(CountryList) / sizeof(country_info)); n++)
      { //Serial.printf("check index %d (cc-code=%s) of countrylist ->%s\n", n, CountryList[n].code, CountryList[n].valutacode);
        //Serial.printf("check index %d (cc-code=%s) of countrylist ->%s\n", n, countrycode, valutacode);
        if(strlen(CountryList[n].valutacode)>0)
        { rate = doc["rates"][CountryList[n].valutacode];
          // Serial.printf("Found rate for %s = %f\n", CountryList[n].valutacode, rate);
          CountryList[n].exchangerate = rate;
        }
      }
    }
    else 
    { Serial.println("DeserializationError");
    }
  }
  Serial.println("Deserialization Finished");
}

int IsCountryCodeValid(char *countrycode)
{ uint16_t n=0;
  static char prevvalidcountrycode[32] = "";
  static int index = -1;
  
  if(strcmp(prevvalidcountrycode, countrycode)==NULL)
  { return index;
  }
  while(n<(sizeof(CountryList) / sizeof(country_info)))
  { //Serial.printf("CountryList[%d].name = %s countrycode %s\n", n, CountryList[n].name, CountryList[n].code);
    if(strncmp(countrycode, CountryList[n].code, 2)==NULL)
    { strcpy(prevvalidcountrycode, CountryList[n].code);
      index = n;
      return index;
    }
    n++;
  }
  return index; // which is -1 if we get here
}


/*
// exchange rates
// https://open.er-api.com/v6/latest/EUR

{
  "result": "success",
  "provider": "https://www.exchangerate-api.com",
  "documentation": "https://www.exchangerate-api.com/docs/free",
  "terms_of_use": "https://www.exchangerate-api.com/terms",
  "time_last_update_unix": 1755043351,
  "time_last_update_utc": "Wed, 13 Aug 2025 00:02:31 +0000",
  "time_next_update_unix": 1755130861,
  "time_next_update_utc": "Thu, 14 Aug 2025 00:21:01 +0000",
  "time_eol_unix": 0,
  "base_code": "EUR",
  "rates": {
    "EUR": 1,
    "AED": 4.280732,
    "AFN": 79.446329,
    "ALL": 97.162267,
    "AMD": 445.596635,
    "ANG": 2.086456,
    "AOA": 1090.020061,
    "ARS": 1540.364134,
    "AUD": 1.788229,
    "AWG": 2.086456,
    "AZN": 1.973532,
    "BAM": 1.95583,
    "BBD": 2.331236,
    "BDT": 141.745726,
    "BGN": 1.95583,
    "BHD": 0.438272,
    "BIF": 3469.615621,
    "BMD": 1.165618,
    "BND": 1.496842,
    "BOB": 8.055136,
    "BRL": 6.312013,
    "BSD": 1.165618,
    "BTN": 102.00973,
    "BWP": 16.061623,
    "BYN": 3.673219,
    "BZD": 2.331236,
    "CAD": 1.605838,
    "CDF": 3381.971429,
    "CHF": 0.941886,
    "CLP": 1124.195363,
    "CNY": 8.371953,
    "COP": 4674.783553,
    "CRC": 587.362371,
    "CUP": 27.974831,
    "CVE": 110.265,
    "CZK": 24.469897,
    "DJF": 207.154789,
    "DKK": 7.460898,
    "DOP": 71.038289,
    "DZD": 150.938991,
    "EGP": 56.315037,
    "ERN": 17.484269,
    "ETB": 159.54894,
    "FJD": 2.619911,
    "FKP": 0.864364,
    "FOK": 7.460898,
    "GBP": 0.864384,
    "GEL": 3.127189,
    "GGP": 0.864364,
    "GHS": 13.044827,
    "GIP": 0.864364,
    "GMD": 84.735373,
    "GNF": 10122.108036,
    "GTQ": 8.910058,
    "GYD": 243.057495,
    "HKD": 9.153426,
    "HNL": 30.471624,
    "HRK": 7.5345,
    "HTG": 152.341055,
    "HUF": 395.654974,
    "IDR": 18943.269169,
    "ILS": 3.983228,
    "IMP": 0.864364,
    "INR": 102.010156,
    "IQD": 1524.064378,
    "IRR": 49305.34087,
    "ISK": 143.005239,
    "JEP": 0.864364,
    "JMD": 185.684258,
    "JOD": 0.826423,
    "JPY": 172.479733,
    "KES": 150.167132,
    "KGS": 101.454893,
    "KHR": 4672.460526,
    "KID": 1.788228,
    "KMF": 491.96775,
    "KRW": 1615.60357,
    "KWD": 0.35539,
    "KYD": 0.971348,
    "KZT": 630.989034,
    "LAK": 25176.468168,
    "LBP": 104322.807385,
    "LKR": 350.145733,
    "LRD": 232.856516,
    "LSL": 20.536901,
    "LYD": 6.299574,
    "MAD": 10.531529,
    "MDL": 19.464781,
    "MGA": 5136.805318,
    "MKD": 61.5448,
    "MMK": 2440.095575,
    "MNT": 4152.554634,
    "MOP": 9.427652,
    "MRU": 46.540891,
    "MUR": 52.893812,
    "MVR": 17.927989,
    "MWK": 2030.337828,
    "MXN": 21.694621,
    "MYR": 4.922692,
    "MZN": 74.379392,
    "NAD": 20.536901,
    "NGN": 1781.314725,
    "NIO": 42.74314,
    "NOK": 11.920663,
    "NPR": 163.215569,
    "NZD": 1.960331,
    "OMR": 0.448177,
    "PAB": 1.165618,
    "PEN": 4.10544,
    "PGK": 4.840581,
    "PHP": 66.362305,
    "PKR": 330.793694,
    "PLN": 4.257876,
    "PYG": 8690.468125,
    "QAR": 4.242849,
    "RON": 5.064366,
    "RSD": 117.143576,
    "RUB": 92.815741,
    "RWF": 1691.304793,
    "SAR": 4.371067,
    "SBD": 9.564084,
    "SCR": 17.174574,
    "SDG": 519.162281,
    "SEK": 11.15592,
    "SGD": 1.49658,
    "SHP": 0.864364,
    "SLE": 26.880867,
    "SLL": 26874.658391,
    "SOS": 663.751402,
    "SRD": 43.518015,
    "SSP": 5496.122398,
    "STN": 24.5,
    "SYP": 14980.565738,
    "SZL": 20.536901,
    "THB": 37.740425,
    "TJS": 10.890955,
    "TMT": 4.063004,
    "TND": 3.359117,
    "TOP": 2.748167,
    "TRY": 47.518292,
    "TTD": 8.127734,
    "TVD": 1.788228,
    "TWD": 34.876336,
    "TZS": 2929.848599,
    "UAH": 48.137422,
    "UGX": 4122.127754,
    "USD": 1.16591,
    "UYU": 46.525997,
    "UZS": 14598.653067,
    "VES": 155.759649,
    "VND": 30516.183371,
    "VUV": 138.488047,
    "WST": 3.11952,
    "XAF": 655.957,
    "XCD": 3.147168,
    "XCG": 2.086456,
    "XDR": 0.850374,
    "XOF": 655.957,
    "XPF": 119.332,
    "YER": 278.882504,
    "ZAR": 20.532125,
    "ZMW": 27.18222,
    "ZWL": 31.156
  }
}

*/


