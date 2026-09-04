#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h" 

/*

// API for timezone, google, api key needed
// https://maps.googleapis.com/maps/api/timezone/json?location=52.21810%2C4.54510&timestamp=1747924919&key=YOUR-API-KEY-FROM-GOOGLE

// API for geo decoding, google, api key needed
// https://maps.googleapis.com/maps/api/geocode/json?latlng=53,5&key=YOUR-API-KEY-FROM-GOOGLE

// API for weather condition, api key needed
//https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key}

// API for exchange rates 
// https://open.er-api.com/v6/latest/EUR

// API for flightradar data, free, no api key needed
// https://opendata.adsb.fi/api/v3/lat/53.0000/lon/11.0000/dist/25

// API for timezone, not google, not tested
// https://timeapi.io/api/v1/time/current/coordinate?latitude=38.9&longitude=-77.03
*/


const char* ntpServer = "pool.ntp.org"; // seems faster overall
//const char* ntpServer = "time.google.com";

const char gps_to_time_url[] = "https://maps.googleapis.com/maps/api/timezone/json?location=";
const char gps_to_geocoding[] = "https://maps.googleapis.com/maps/api/geocode/json?latlng=";


// as defined in ..\secrets.h
// const char google_api_key[] = "YOUR-GOOGLE-API-KEY";

char api_url[256];



// asynchronous fetching of timezone, geolocation country and town, weather data and currency exchange rates 
// plan is to queue these 
void Queued_Api_Calls(void * pvParameters)
{ while(1)
  { if(RefreshRatesCountDownTimer==0)
    { FetchJsonExchangeRates(); // resets RefreshRatesCountDownTimer
    }

    // process messages arrived in ApiQueue
    if(ApiCallsToDo.ApiQueueIndexIn != ApiCallsToDo.ApiQueueIndexOut && FromDisplay.QueueCnt==0) // we have to catch up with new messages, one at the time
    { // process ApiCallsToDo.ApiQueueMessageType[ApiCallsToDo.ApiQueueIndexOut];
      uint16_t ApiType = ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexOut]; // atomic local copy

      Serial.printf("ASYNC PROCESS: PUCK-SERIAL %d %s %d\n", ApiCallsToDo.ApiQueueMessageSerialNumber[ApiCallsToDo.ApiQueueIndexOut], messagetexts[ApiType], ApiType);

      switch(ApiType)
      { case MESSAGE_GET_TIMEZONE_BY_GPS:
        case MESSAGE_TIMEZONE_NAME:
        case MESSAGE_HOME_TIMEZONE_NAME:
          //GetTimeZone(ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiPuckRequest[ApiCallsToDo.ApiQueueIndexOut]);
          GetTimeZoneFromGeoNames(ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiType[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiPuckRequest[ApiCallsToDo.ApiQueueIndexOut]);
          GetOpenWeatherData(ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiPuckRequest[ApiCallsToDo.ApiQueueIndexOut]);
          break;


        case MESSAGE_GET_GEOLOCATION_BY_GPS:
          GetGeolocationData(ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiPuckRequest[ApiCallsToDo.ApiQueueIndexOut]);
          break;

        case MESSAGE_GET_GEOLOCATION:
          GetGeolocationData(ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiRequestedStation[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiPuckRequest[ApiCallsToDo.ApiQueueIndexOut]);
          break;

        case MESSAGE_GET_FLIGHT_DATA:
          // degrees are full degrees
          GetAdsbFiFlightData(ApiCallsToDo.ApiParameterNS[ApiCallsToDo.ApiQueueIndexOut], ApiCallsToDo.ApiParameterEW[ApiCallsToDo.ApiQueueIndexOut]); 
          break;  
          
        default:
          break;  
      }
      ApiCallsToDo.ApiQueueIndexOut++;
      ApiCallsToDo.ApiQueueIndexOut %= APIQUEUESIZE;
      ApiCallsToDo.ApiQueueCnt--;
    }
    vTaskDelay(250 / portTICK_PERIOD_MS); // lowered to 100, was 200
  }  
}

void CancelApiType(uint16_t ApiType)
{ for(uint16_t n = 0; n<APIQUEUESIZE; n++)
  { if(ApiCallsToDo.ApiType[n]==ApiType)
    { ApiCallsToDo.ApiType[n]=MESSAGE_CANCELLED_APITYPE;
    }
 }
}

void GetTimeZone(float StationGpsNS, float StationGpsEW, uint16_t ApiType, uint16_t PuckRequest)
{ WiFiClientSecure client;
  time_t now;
  char   payload[256]; 
  char   content[256]; 
  int32_t dstOffset = 0;
  int32_t rawOffset = 0;
  bool print = 0;
  
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   

  lapMillis = millis(); 
  
  if(print) Serial.printf("GetTimeZone Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

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
      char posix[32];
      int32_t total_offset = -(rawOffset+dstOffset);
      snprintf(posix, sizeof(posix), "CUSTOM%ld:%02ld:%02ld", (total_offset/3600), labs((total_offset%3600)/60), labs(total_offset%60));
      // is something like "CUSTOM-2:00:00
      Serial.printf("TZ posix = %s\n", posix);
      setenv("TZ", posix, 1);
      tzset();
      if(ApiType==MESSAGE_HOME_TIMEZONE_NAME)AddToQueueForDisplay(posix, MESSAGE_HOME_TIMEZONE_POSIX);
      AddToQueueForDisplay(posix, MESSAGE_TIMEZONE_POSIX);

      uint16_t retrys = 0;
      while(!getLocalTime(&DataFromGlobe.timeinfo) && retrys < 50) 
      { delay(100);
        retrys++;
      }
      DataFromGlobe.G_now = time(NULL); // UTC for puck

      //Serial.printf("TEST -> hour = %d\n", DataFromGlobe.timeinfo.tm_hour);
      //Serial.printf("TEST -> min = %d\n", DataFromGlobe.timeinfo.tm_min);
  
      //sprintf(content, "TZN - %s", (const char*)doc["timeZoneName"]); 
      sprintf(content, "TZ - %s", (const char*)doc["timeZoneId"]); 
      AddToQueueForDisplay(content, ApiType);
      if(print)Serial.println(content);
      if(print)printLocalTime();      
    }
    else // at sea
    { dstOffset = 0;
      rawOffset = 0;
      // use these offsets 
      setenv("TZ", "CUSTOM0:00:00", 1);
      tzset(); 
      AddToQueueForDisplay("CUSTOM0:00:00", MESSAGE_TIMEZONE_POSIX);
      uint16_t retrys = 0;
      while(!getLocalTime(&DataFromGlobe.timeinfo) && retrys < 50) 
      { delay(100);
        retrys++;
      }
      DataFromGlobe.G_now = time(NULL); // UTC for puck
      AddToQueueForDisplay("TZ - Nautical", MESSAGE_TIMEZONE_NAME);
      

      CancelApiType(MESSAGE_GET_GEOLOCATION); // does not make sense anymore, TZ nautical means we are at sea
      strcpy(CountryCodeSelectorSD, "XX");
      DataFromGlobe.D_ApisFetchedForStation = -1;
      sprintf(content, "XX,???,%d", PuckRequest); 
      AddToQueueForDisplay(content, MESSAGE_GET_GEOLOCATION);

      //printLocalTime();
    }
  }
  
  if(print)Serial.print("Done Google query timezone\n");
  // Serial.println("MFree Heap at End GetTimeZone()" + String(ESP.getFreeHeap()));   
  Serial.printf("GetTimeZone http response (%d) took %ld mSec\n", httpResponseCode, millis() - lapMillis);
}

// fetch country and town
void GetGeolocationData(float StationGpsNS, float StationGpsEW, int16_t AskingForStation, uint16_t PuckRequest)
{ static WiFiClientSecure client;
  char   payload[512]; // 256->512
  char   shortname[128]; // 64->128
  // char   longname[128]; // 64->128
  char   countrycode[64]="XX"; 
  char   town[128]="???"; // 64->128
  uint16_t searches;
  bool found = false;
  bool townfound = false;
  bool countryfound = false;
  unsigned long startMs;
  
  bool print;
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   
  print = 0;
  
  if(print)Serial.printf("GetGeoLocationData Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

  // examples
  // https://maps.googleapis.com/maps/api/geocode/json?latlng=42.464699,21.466900&key=YOUR-API-KEY-FROM-GOOGLE
  // https://maps.googleapis.com/maps/api/geocode/json?latlng=19.286900,-81.367401&key=YOUR-API-KEY-FROM-GOOGLE

  //size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  //Serial.printf("235 Geodata - largest_block internal ram %d bytes\n", largest_block);

  sprintf(api_url, "%s%.6f%%2C%.6f&key=%s", gps_to_geocoding, StationGpsNS, StationGpsEW, GlobeSettings.google_api_key);

  // get ocean name??
  // https://maps.googleapis.com/maps/api/geocode/json?latlng=19.286900,-81.367401&result_type=natural_feature&key=YOUR-API-KEY-FROM-GOOGLE
  // sprintf(api_url, "%s%.6f%%2C%.6f&result_type=natural_feature&key=%s", gps_to_geocoding, StationGpsNS, StationGpsEW, GlobeSettings.google_api_key); // does not give name of sea

  // to investigate, getting the name of sea or ocean
  //  api.geonames.org/oceanJSON?lat=40.78343&lng=-43.96625&username=demo
  //  as seen here https://www.geonames.org/export/web-services.html#ocean
  
  if(print)
  { Serial.println("START MET:");
    Serial.println(api_url);
  }

  startMs = millis();
  client.setInsecure();

  static HTTPClient https;
  //if(print)Serial.println("HTTP Client starten");
  https.begin(client, api_url);
  //if(print)Serial.println("HTTP Client gestart");
  int httpResponseCode = https.GET();
  //Serial.printf("GetGeolocationData https.GET took %ldmS\n", (millis()-startMs)); // typical 821mS
  if (httpResponseCode>0) 
  { client.setTimeout(100);
    char *p;
    while(client.available() && !found)
    { uint16_t cnt = client.readBytesUntil('\n', payload, sizeof(payload)); // or until client.setTimeout(100)
      payload[cnt]=0;
      if(print)Serial.printf("cnt=%d ->%s\n", cnt, payload);
      if((p = strstr(payload, "API key is invalid")) != NULL)
      { strcpy(GlobeSettings.google_api_key, google_api_key);
        EEPROM.put(0x0, GlobeSettings);
        EEPROM.commit();
        Serial.printf("Google Api resetted to one stored in firmware -> %s\n", GlobeSettings.google_api_key);
      }

      if((p = strrchr(payload, '\"')) != NULL)*p=0; // get rid of the last ",
      if((p = strstr(payload, "\"short_name\" : \"")) != NULL)
      { p+=16;
        //Serial.println(p);
        strcpy(shortname, p); // contains town name or countrycode
      }
      // if((p = strstr(payload, "\"long_name\" : \"")) != NULL)
      // { p+=15;
      //  Serial.println(p);
      //  strcpy(longname, p);
      // }
      // if((p = strstr(payload, "\"administrative_area_level_1")) != NULL)
      // { strcpy(town, longname); // that's not the town, more like an area name
      //   searches--;
      // }
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
    sprintf(payload, "%s,%s,%d", countrycode, town, PuckRequest);
    //sprintf(payload, "%s,%s,%d", countrycode, town, D_RequestedStation); // don't like it yet
    DataFromGlobe.D_ApisFetchedForStation = AskingForStation; 
    Serial.printf("GetGeolocationData() result -> >%s<\n", payload);



  }  
  else 
  { sprintf(payload, "%s,%s,%d", countrycode, town, PuckRequest);
    //sprintf(payload, "%s,%s,%d", countrycode, town, D_RequestedStation); // don't like it yet
    DataFromGlobe.D_ApisFetchedForStation = AskingForStation; 
    Serial.printf("GetGeolocationData() -> Error code: %d\n", httpResponseCode);
  }

  https.end();
  client.stop(); // stop insecure client
  Serial.printf("GetGeolocationData fetch took %ldmS found=%d\n", (millis()-startMs), found);

  if (httpResponseCode<=0) return;

  // if no countrycode was decoded, we are presumably at sea, which we treat as country XX
  // that will be used to load music files from GLOBEMUSE/XX folder for sea sound
  
  if(strcmp(countrycode, "XX")==0)strcpy(CountryCodeSelectorSD, countrycode);

  if(AskingForStation>=0)AddToQueueForDisplay(payload, MESSAGE_GET_GEOLOCATION_BY_GPS);
  else AddToQueueForDisplay(payload, MESSAGE_GET_GEOLOCATION);

}

void GetOpenWeatherData(float StationGpsNS, float StationGpsEW, uint16_t PuckRequest)
{ WiFiClientSecure client;
  char   payload[1024]; 
  char   countrycode[3]=""; 
  char   content[256]; 
 
  bool print = 0;
  
  //Serial.println("MFree Heap at Start  GetTimeZone()" + String(ESP.getFreeHeap()));   

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

void printLocalTime()
{ struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  { Serial.println("Failed to obtain time");
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

void FetchJsonExchangeRates(void)
{ WiFiClientSecure client;
  int country_index;
  String payload;

  if((country_index=IsCountryCodeValid(GlobeSettings.HomeCountryCode))<0)
  { Serial.printf("Can't get exchange rates for unknown home country %s\n", GlobeSettings.HomeCountryCode);
    RefreshRatesCountDownTimer = 3600; // do it again after 1 hours
    return;
  }

  if(strlen(CountryList[country_index].valutacode)<3)
  { Serial.printf("Can't get exchange rates for valuta -> %s\n", CountryList[country_index].valutacode);
    RefreshRatesCountDownTimer = 3600; // do it again after 1 hours
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
    // strcpy(exchangepayload, https.getString().c_str());
    //    if(print)Serial.println(exchangepayload);
    //    if(print)Serial.printf("length exchangepayload = %d\n", strlen(exchangepayload));
    payload = https.getString();
    if(print)Serial.println(payload);
    if(print)Serial.printf("length exchange payload = %d\n", strlen(payload.c_str()) );
  }
  else 
  { payload = "";
    if(print)Serial.printf("Error code: %d\n", httpResponseCode);
  }
  https.end();
  client.stop(); // stop insecure client
  //DecodeExchangeJson(exchangepayload);
  DecodeExchangeJson(payload.c_str());
  RefreshRatesCountDownTimer = 3600 * 24; // do it again after 24 hours
  
}

void DecodeExchangeJson(const char *exchangepayload)
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


#include <HTTPClient.h>
#include <ArduinoJson.h>

// De compacte struct voor ESP-NOW

struct FlightBlip {
    int16_t rel_x;
    int16_t rel_y;
    uint16_t heading;
    char callsign[8];
    char type[5];
    uint16_t altitude_ft; // We slaan direct het Flight Level op (bijv. 310)
} __attribute__((packed));

struct RadarDataPacket {
    uint8_t packet_type = 0xAA; // Uniek ID voor de Puck receptie
    uint8_t num_flights = 0;
    FlightBlip flights[5];
} __attribute__((packed));

void GetAdsbFiFlightData(float lat, float lon)
{ WiFiClientSecure client;  
          
  client.setInsecure();

  // search in a radius of 25 nautic miles (~46 km) at given gps location
  String url = "https://opendata.adsb.fi/api/v3/lat/" + String(lat, 4) + "/lon/" + String(lon, 4) + "/dist/25";
  // example https://opendata.adsb.fi/api/v3/lat/44.0000/lon/10.0000/dist/25
  // Serial.println(url);

  HTTPClient http;
  http.begin(url);
    
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) 
  { String payload = http.getString();
    // Serial.println(payload);
    // adsb.fi can give large JSON results. make enough room for that!
    DynamicJsonDocument doc(16384); 
    DeserializationError error = deserializeJson(doc, payload);
        
    if (!error) 
    { JsonArray aircrafts = doc["ac"];
      RadarDataPacket responsePacket;
      responsePacket.packet_type = 0xAA; 
      responsePacket.num_flights = 0;

      for (JsonObject ac : aircrafts) 
      { if (responsePacket.num_flights >= 5) break; // max 5 on radar
        if (!ac.containsKey("lat") || !ac.containsKey("lon")) continue;

        float f_lat = ac["lat"].as<float>();
        float f_lon = ac["lon"].as<float>();
        float f_track = ac["track"].as<float>();
                
        String cleanFlight = ac["flight"].as<String>();
        cleanFlight.trim(); // remove spaces
        if (cleanFlight.length() == 0) cleanFlight = "UNK";
        String cleanType = ac["t"].as<String>();
        cleanType.trim();
        if (cleanType.length() == 0) cleanType = "AIRC";
       
        int alt_baro = 0;
        if (ac["alt_baro"].is<int>()) 
        { alt_baro = ac["alt_baro"].as<int>();
        }


        float distance_from_center = CalculateDistanceInKm(lat, lon, f_lat, f_lon);

        const float R = 6371.0; 
        float mid_lat_rad = ((lat + f_lat) / 2.0) * DEG_TO_RAD;
        float distance_x_km = (f_lon - lon) * DEG_TO_RAD * cos(mid_lat_rad) * R;
        float distance_y_km = (f_lat - lat) * DEG_TO_RAD * R;

        // scale km to 200-pixel radar radius
        // 200 pixels / 46.3 km = 4.31966
        int16_t calc_x = (int16_t)(distance_x_km * 4.31966);
        int16_t calc_y = (int16_t)(distance_y_km * 4.31966);

        int32_t squared_distance = ((int32_t)calc_x * calc_x) + ((int32_t)calc_y * calc_y);
        // treshold: stay within 400x400 cirle -> radius = 200px -> squared = 40000
        // Serial.printf("x=%d y=%d px^2-distance=%d distance-float=%f kM callsign <%s>\n", calc_x, calc_y, squared_distance, distance_from_center, responsePacket.flights[responsePacket.num_flights].callsign);
               
        if(squared_distance > 40000) 
        {  // Serial.printf("Off Radar   x=%d y=%d px^2-distance=%d distance-float=%f kM callsign <%s>\n", calc_x, calc_y, squared_distance, distance_from_center, responsePacket.flights[responsePacket.num_flights].callsign);
           // aircarft is off the radar, skip it!
           continue; 
        }

        responsePacket.flights[responsePacket.num_flights].rel_x = calc_x;
        responsePacket.flights[responsePacket.num_flights].rel_y = calc_y;

        // test to see if this renders the airplane at the center of the puck display - yes it does
        // if(responsePacket.num_flights == 0)
        // { responsePacket.flights[responsePacket.num_flights].rel_x = 0;
        //   responsePacket.flights[responsePacket.num_flights].rel_y = 0;
        // }

        responsePacket.flights[responsePacket.num_flights].heading = (uint16_t)f_track;


        memset(responsePacket.flights[responsePacket.num_flights].callsign, 0, 8);
        strncpy(responsePacket.flights[responsePacket.num_flights].callsign, cleanFlight.c_str(), 7);

        memset(responsePacket.flights[responsePacket.num_flights].type, 0, 5);
        strncpy(responsePacket.flights[responsePacket.num_flights].type, cleanType.c_str(), 4);

        //Serial.printf("callsign <%s> type <%s>\n", responsePacket.flights[responsePacket.num_flights].callsign, responsePacket.flights[responsePacket.num_flights].type);

        // set height
        responsePacket.flights[responsePacket.num_flights].altitude_ft = (uint16_t)(alt_baro);

        responsePacket.num_flights++;
      } // end for

      // send struct back to puck
           
      // for(int n=0; n<responsePacket.num_flights; n++)
      // { Serial.printf("callsign <%s> type <%s>\n", responsePacket.flights[n].callsign, responsePacket.flights[n].type);
      // }
      Serial.printf("Radar: %d flights send to puck\n", responsePacket.num_flights);
      AddToQueueForDisplay((char *)&responsePacket, MESSAGE_GET_FLIGHT_DATA);
    }
  } 
  else 
  {  Serial.printf("Radar API Error: HTTP code %d\n", httpCode);
  }
  http.end();
}

float CalculateDistanceInKm(float lat1, float lon1, float lat2, float lon2) {
    // radius earth in km
    const float R = 6371.0; 

    // convert degrees to radians (degrees * pi / 180)
    float lat1_rad = lat1 * M_PI / 180.0;
    float lat2_rad = lat2 * M_PI / 180.0;
    float delta_lat_rad = (lat2 - lat1) * M_PI / 180.0;
    float delta_lon_rad = (lon2 - lon1) * M_PI / 180.0;

    // haversine formula
    float a = sin(delta_lat_rad / 2.0) * sin(delta_lat_rad / 2.0) +
              cos(lat1_rad) * cos(lat2_rad) *
              sin(delta_lon_rad / 2.0) * sin(delta_lon_rad / 2.0);
              
    float c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    // total distance in km
    return R * c; 
}


void GetTimeZoneFromGeoNames(float StationGpsNS, float StationGpsEW, uint16_t ApiType, uint16_t PuckRequest)
{ 
  WiFiClient client; 
  time_t now;
  char   payload[512]; 
  char   content[256]; 
  int32_t dstOffset = 0;
  int32_t rawOffset = 0;
  bool print = 0;
  
  lapMillis = millis(); 
  
  if(print) Serial.printf("GetTimeZone Requested By Display -> Position NS = %f, EW = %f\n", StationGpsNS, StationGpsEW);

  time(&now); // UTC epoch time
  
  // geoNames URL for http
  // http://api.geonames.org/timezoneJSON?lat=47.01&lng=10.2&username=demo
  sprintf(api_url, "http://api.geonames.org/timezoneJSON?lat=%f&lng=%f&username=%s", StationGpsNS, StationGpsEW, geonames_username);
  
  if(print) Serial.println(api_url);

  HTTPClient http;
  http.begin(client, api_url);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) 
  { 
    if(print) Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    strcpy(payload, http.getString().c_str());
    if(print) Serial.println(payload);
  }  
  else 
  {
    if(print) Serial.printf("Error code: %d\n", httpResponseCode);
  }
  http.end();
  
  if (httpResponseCode > 0)
  {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) 
    { Serial.printf("JSON parsing error: %s\n", error.c_str());
      Serial.printf("GetTimeZone http response (%d) took %ld mSec\n", httpResponseCode, millis() - lapMillis);
      return;
    } 
    
    Serial.println(payload);
    // if all goes well. geonames gives no doc["status"] 
    // at sea, it does not give a meaningful timezone 
    if (!doc.containsKey("status") && doc.containsKey("timezoneId"))
    { 
      float rawOffsetFloat = doc["rawOffset"]; // geogames uses 'rawOffset'
      float dstOffsetFloat = doc["dstOffset"]; // geogames uses 'dstOffset'
      
      // geonames gives offsets in hours, like 1.0 or 5.5 so multiply with 3600 to get it in seconds
      rawOffset = (int32_t)(rawOffsetFloat * 3600.0f);
      dstOffset = (int32_t)((dstOffsetFloat - rawOffsetFloat) * 3600.0f); // GeoNames dstOffset bevat vaak al de rawOffset, we isoleren hier het verschil

      // use offsets for POSIX string
      char posix[32];
      int32_t total_offset = -(rawOffset + dstOffset);
      snprintf(posix, sizeof(posix), "CUSTOM%ld:%02ld:%02ld", (total_offset/3600), labs((total_offset%3600)/60), labs(total_offset%60));
      
      Serial.printf("TZ posix = %s\n", posix);
      setenv("TZ", posix, 1);
      tzset();
      
      if(ApiType == MESSAGE_HOME_TIMEZONE_NAME) AddToQueueForDisplay(posix, MESSAGE_HOME_TIMEZONE_POSIX);
      AddToQueueForDisplay(posix, MESSAGE_TIMEZONE_POSIX);

      uint16_t retrys = 0;
      while(!getLocalTime(&DataFromGlobe.timeinfo) && retrys < 50) 
      { 
        delay(100);
        retrys++;
      }
      DataFromGlobe.G_now = time(NULL); // UTC voor puck
  
      sprintf(content, "TZ - %s", (const char*)doc["timezoneId"]); 
      AddToQueueForDisplay(content, ApiType);
      if(print) Serial.println(content);
      if(print) printLocalTime();      
    }
    else 
    { // at sea, typical respons is {"lng":-31.1,"gmtOffset":-2,"rawOffset":-2,"dstOffset":0,"lat":44.299999}
      // but we use nautical time instead
      dstOffset = 0;
      rawOffset = 0;
      
      setenv("TZ", "CUSTOM0:00:00", 1);
      tzset(); 
      AddToQueueForDisplay("CUSTOM0:00:00", MESSAGE_TIMEZONE_POSIX);
      
      uint16_t retrys = 0;
      while(!getLocalTime(&DataFromGlobe.timeinfo) && retrys < 50) 
      { 
        delay(100);
        retrys++;
      }
      DataFromGlobe.G_now = time(NULL); // UTC for puck
      AddToQueueForDisplay("TZ - Nautical", MESSAGE_TIMEZONE_NAME);
      
      CancelApiType(MESSAGE_GET_GEOLOCATION); // // does not make sense anymore, TZ nautical means we are at sea
      strcpy(CountryCodeSelectorSD, "XX");
      DataFromGlobe.D_ApisFetchedForStation = -1;
      sprintf(content, "XX,???,%d", PuckRequest); 
      AddToQueueForDisplay(content, MESSAGE_GET_GEOLOCATION);
    }
  }
  if(print) Serial.print("Done GeoNames query timezone\n");
  Serial.printf("GetTimeZone http response (%d) took %ld mSec\n", httpResponseCode, millis() - lapMillis);
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

