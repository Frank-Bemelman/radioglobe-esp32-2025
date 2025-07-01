#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// prototypes
bool DnsCheck(char *url);

// task
void CheckUrl(void * pvParameters)
{ static int16_t http_result;
  while(1)
  { if(DataFromGlobe.Unraveling == 1)
    {  // we use (global) RequestedStationUrl
       Serial.println("CheckUrl() Start");
       Serial.println("MFree Heap at Start " + String(ESP.getFreeHeap()));   
       //if(!DnsCheck(RequestedUrl))
       if(0)
       { DataFromGlobe.UnravelingResult = 0;
       }
       else // dns valid
       { int n=3;
         Serial.println("Start redirect checks");

         while(n) // check max 3 times if we are redirected, redirected, redirected..
         { Serial.println("MFree Heap B4 we continue " + String(ESP.getFreeHeap()));   
           Serial.printf("DataFromGlobe.UnravelingResult1 -> %d\n", DataFromGlobe.UnravelingResult);
           if(UnraveledUrl[4]=='s')
           { if(!CheckForRedirectsHttps())break;
           }
           else
           { if(!CheckForRedirectsHttp())break;
           }
           n--; // try again
         }
         Serial.printf("DataFromGlobe.UnravelingResult1A -> %d\n", DataFromGlobe.UnravelingResult);
           
        

         if(n>0) // possible redirected bur still valid url
         { if(UnraveledUrl[4]=='s')DataFromGlobe.UnravelingResult = UnravelHttpsUrl();
           else DataFromGlobe.UnravelingResult = UnravelHttpUrl();
           Serial.printf("DataFromGlobe.UnravelingResult2 -> %d\n", DataFromGlobe.UnravelingResult);
         }  
         else DataFromGlobe.UnravelingResult = 0;


         // final check http code responce
//         { if(UnraveledUrl[4]=='s')
//           { http_result = HttpsUrlCodeReturn();
//             if(http_result!=200)DataFromGlobe.UnravelingResult = 0;
//           }
//           else 
//           { http_result = HttpUrlCodeReturn();
//             if(http_result!=200)DataFromGlobe.UnravelingResult = 0;
//           }
//         }  

         // final dns check for final url
         if(DataFromGlobe.UnravelingResult)
         { if(!DnsCheck(UnraveledUrl))
           { DataFromGlobe.UnravelingResult = 0;
           }
         }  
       }
       // finished
       Serial.println("MFree Heap Check() finished1 " + String(ESP.getFreeHeap()));   
       DataFromGlobe.Unraveling = 0;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS); // lowered to 100, was 200
  }  
}


bool DnsCheck(char *url)
{ IPAddress aResult;
  int dns_result = 0;  
  int dns_result2 = 0;  
    
  char host[128];
  char *p1;
  char *p2;
  char *p3;
  
  //  https://blabla.nl:99/blabla

  lapMillis = millis(); 
  if((p1 = strstr(url,"//"))!=NULL)
  { p1+=2;
    if((p2 = strstr(p1,"/"))!=NULL) 
    {  p3 = host;
       while(p1<p2)
       { *p3++ = *p1++; 
       }
       *p3 = 0;
       Serial.println(host);
    }

    if((p3 = strstr(host,":"))!=NULL)
    { *p3=0; // port number termination
    }
    Serial.println(host);
    dns_result = WiFi.hostByName(host, aResult);
    Serial.println(aResult); 

  }
  else
  { Serial.printf("Not an url -> dns_result -> 0\n"); 
  }

  Serial.printf("WiFi.hostByName finished -> time elapsed = %ld\n", (currentMillis = millis()) - lapMillis);
  if(dns_result>0) return true;
  return false;

}


uint16_t HttpUrlCodeReturn(char *url)
{ //WiFiClientSecure client;
  WiFiClient client;
  HTTPClient http;
  int httpResponseCode;
  char *p;
  
  bool print;
  print = 1;
    
  if(print)Serial.printf("HttpUrlCodeReturn %s\n", url);

//   end stream point?
//   if((p=strstr(url, ".mp3"))!=NULL)
//   { if(print)Serial.printf("HttpUrlCodeReturn self appointed 200 for this mp3 link\n");
//     return 200;
//   }
 
  if(print)Serial.println("HttpUrlCodeReturn HTTP Client starten");

  http.begin(client, url);
  if(print)Serial.println("HttpUrlCodeReturn HTTP Client gestart");
  httpResponseCode = http.GET();
  if (httpResponseCode>0) 
  { if(print)Serial.printf("HttpUrlCodeReturn HTTP Response code: %d\n", httpResponseCode);
  }  
  else 
  { if(print)Serial.printf("HttpUrlCodeReturn Error code: %d\n", httpResponseCode);
  }

  http.end();
  client.stop(); // stop insecure client


  return httpResponseCode;
}


uint16_t HttpsUrlCodeReturn(char *url)
{ //WiFiClientSecure client;
  WiFiClientSecure client;
  HTTPClient https;
  int httpsResponseCode;
  char *p;

  bool print;
  print = 1;

  client.setInsecure();

  if(print)Serial.printf("HttpsUrlCodeReturn %s\n", url);

  // end stream point?
  // if((p=strstr(url, ".mp3"))!=NULL)
  // { if(print)Serial.printf("HttpsUrlCodeReturn self appointed 200 for this mp3 link\n");
  //   return 200;
  // }
 
  if(print)Serial.println("HttpsUrlCodeReturn HTTP Client starten");

  https.begin(client, url);
  if(print)Serial.println("HttpsUrlCodeReturn HTTP Client gestart");
  httpsResponseCode = https.GET();
  if (httpsResponseCode>0) 
  { if(print)Serial.printf("HttpsUrlCodeReturn HTTP Response code: %d\n", httpsResponseCode);
  }  
  else 
  { if(print)Serial.printf("HttpsUrlCodeReturn Error code: %d\n", httpsResponseCode);
  }

  https.end();
  client.stop(); // stop insecure client

  return httpsResponseCode;
}



