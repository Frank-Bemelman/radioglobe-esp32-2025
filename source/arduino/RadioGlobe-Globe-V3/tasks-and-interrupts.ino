// tasks

void setup_tasks(void)
{
  //xTaskCreatePinnedToCore(
  //                  CheckUrl,   /* Task function. */
  //                  "CheckUrl",     /* name of task. */
  //                  25000,       /* Stack size of task */
  //                  NULL,        /* parameter of the task */
  //                  4,           /* priority of the task */
  //                  NULL,      /* Task handle to keep track of created task */
  //                  0);          /* pin task to core 0 */      

  xTaskCreatePinnedToCore(
                    CallGetTimeZone,   /* Task function. */
                    "CallGetTimeZone",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    3,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */      
  xTaskCreatePinnedToCore(
                    ReadAS5600Encoders,   /* Task function. */
                    "ReadAS5600Encoders",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */      
  xTaskCreatePinnedToCore(
                    TaskTouch,   /* Task function. */
                    "TaskTouch",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                   2,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */      


}


void CallGetTimeZone(void * pvParameters)
{ while(1)
  { if(DataFromGlobe.FindTimeZone == MESSAGE_GET_TIMEZONE)
    { GetTimeZone(ns_cal_received/10, ew_cal_received/10); 
      DataFromGlobe.FindTimeZone = 0;
    }
    else if(DataFromGlobe.FindTimeZone == MESSAGE_GET_TIMEZONE_BY_GPS)
    { GetTimeZone(D_StationGpsNS, D_StationGpsEW);
      DataFromGlobe.FindTimeZone = 0;
    }

    else if(DataFromGlobe.FindGeoLocationData == MESSAGE_GET_GEOLOCATION)
    { GetGeolocationData(ns_cal_received/10, ew_cal_received/10);
      DataFromGlobe.FindGeoLocationData = MESSAGE_GET_WEATHER_DATA; // also find weather data after this TODO -> by ns_cal_receved etc
    }
    else if(DataFromGlobe.FindGeoLocationData == MESSAGE_GET_WEATHER_DATA)
    { GetOpenWeatherData(ns_cal_received/10, ew_cal_received/10);
      DataFromGlobe.FindGeoLocationData = 0;
    }


    else if(DataFromGlobe.FindGeoLocationData == MESSAGE_GET_GEOLOCATION_BY_GPS)
    { GetGeolocationData(D_GeoLocationGpsNS, D_GeoLocationGpsEW);
      DataFromGlobe.FindGeoLocationData = MESSAGE_GET_WEATHER_DATA_BY_GPS; // also find weather data after this
    }
    else if(DataFromGlobe.FindGeoLocationData == MESSAGE_GET_WEATHER_DATA_BY_GPS)
    { GetOpenWeatherData(D_GeoLocationGpsNS, D_GeoLocationGpsEW);
      DataFromGlobe.FindGeoLocationData = 0;
    }
    else if(GetRatesNow)
    { FetchJsonExchangeRates();
      GetRatesNow = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // lowered to 100, was 200
  }  
}

// Task to read the globe encoders, roughly ten times a second
void ReadAS5600Encoders(void * pvParameters)
{ static int16_t AverageNS[4];
  static int16_t AverageEW[4];
  static int16_t CurrentNS4096;
  static int16_t CurrentEW4096;
  static int16_t OldCurrentNS4096;
  static int16_t OldCurrentEW4096;
  
  static int8_t  AverageIdx = 0;
  static bool  stable = false;
  static bool  stable_changed = true;
  static int16_t NSFilteredRAW;
  static int16_t EWFilteredRAW;
  static uint16_t stable100ms = 0;
  static bool EncoderReliable = false;  
  static uint8_t LedAnimationBrightness = 0;
  static uint16_t AngleMilliRadians = 0;
  
  while(1)
  { ReadEncoderTicker100mS++;
    if(Timer100msSerialCanBeOpened)Timer100msSerialCanBeOpened--;
    if(Timer100msSerialIsOpen)
    { Timer100msSerialIsOpen--;
      if(Timer100msSerialIsOpen==0)
      { Serial.printf("SerialPort Closed\n");

      }
    }
    
    // keep 4 last readings
    if(!EncoderReliable)
    { if(ReadEncoderTicker100mS > 25)
      { EncoderReliable = true;
        DataFromGlobe.G_EncoderReliable = true;
      }
    }
    AverageIdx %= 4;
    if(as5600_1.readAGC()<100)CurrentNS4096 = as5600_1.readAngle()-2048;
    else CurrentNS4096 = 0;
    //Serial.println(CurrentNS4096);

    if(as5600_2.readAGC()<100)CurrentEW4096 = as5600_2.readAngle()-2048;
    else CurrentEW4096 = 0;
    //Serial.println(CurrentEW4096);

    AverageNS[AverageIdx] = CurrentNS4096;
    AverageEW[AverageIdx] = CurrentEW4096;
    AverageIdx++;

    //Serial.print("ReadAGC NS = "); Serial.println(as5600_1.readAGC());
    //Serial.print("ReadAGC EW = "); Serial.println(as5600_2.readAGC());


    // get the average
    if(!stable)
    { NSFilteredRAW = (AverageNS[0] + AverageNS[1] + AverageNS[2] + AverageNS[3]) / 4;
      EWFilteredRAW = (AverageEW[0] + AverageEW[1] + AverageEW[2] + AverageEW[3]) / 4;
    }  

    // check if last reading is fairly different from the average
    if((abs(NSFilteredRAW - AverageNS[(AverageIdx-1)%4])<8) && (abs(EWFilteredRAW - AverageEW[(AverageIdx-1)%4])<8))
    { // pretty stable
      if(stable100ms<10000)stable100ms++;
      if(EncoderReliable && (stable100ms>5))
      { stable = true;
        if(LedAnimationBrightness)LedAnimationBrightness-=5; // brigthness dims down in roughly 2.5 seconds
      }
    }  
    else // movement detected of NS and/or EW
    { stable = false;
      stable100ms = 0;
      PixelUpdate(0, 0xFFFFFF, 0x000000, 2000); // solid white to light up globe                    
      LedAnimationBrightness=255;

      if(bPowerStatus && !stable && !bEncoderKillStation && bSetupCompleted)
      { PlayWhile((uint8_t *)mp3_radio_tuning, sizeof(mp3_radio_tuning), true); // play one snippet and loop around if need be
      }

    }
    
    // send NS postion to globe with small hysteresis
    if(OldCurrentNS4096 != CurrentNS4096)
    { if(abs(((OldCurrentNS4096 - CurrentNS4096))%4096)>1)
      { OldCurrentNS4096 = CurrentNS4096;
        NSDegLive10 = (CurrentNS4096) * 3600 / 4096; // convert to degrees times 10
        if((NSDegLive10>=-900) && (NSDegLive10<=900)) 
        { DataFromGlobe.ns = NSDegLive10;
          // roughly map angle (0-180) to radians to get color from color circle to use for led animation
          AngleMilliRadians = 35 * ((NSDegLive10 + 900) / 10); 
        }
      }
    }

    // send EW postion to globe with small hysteresis
    if(OldCurrentEW4096 != CurrentEW4096)
    { if(abs(((OldCurrentEW4096 - CurrentEW4096))%4096)>1)
      { OldCurrentEW4096 = CurrentEW4096;
        EWDegLive10 = (CurrentEW4096) * 3600 / 4096; // convert to degrees times 10
        if((EWDegLive10>=-1800) && (EWDegLive10<=1800)) DataFromGlobe.ew = EWDegLive10;
      }
    }


    if(EncoderReliable && (stable_changed != stable))
    { stable_changed = stable;
      if(stable)
      { if(bPowerStatus)Serial.println("Stable while bPowerStatus=true");
        else Serial.println("Stable while bPowerStatus=false");
        NSDeg10 = NSDegLive10;
        EWDeg10 = EWDegLive10;
        //Serial.print("readAngle NS = "); Serial.print(NSDeg10/10.0);
        //Serial.print("\tReadAGC NS = "); Serial.println(as5600_0.readAGC());
        //Serial.print("readAngle EW = "); Serial.print(EWDeg10/10.0);
        //Serial.print("\tReadAGC EW = "); Serial.println(as5600_1.readAGC());
        bEncoderNewPosition = true; // signal to main program to pick another station
        Serial.println("bEncoderNewPosition=true");
      }
      else
      { bEncoderKillStation = true;
      }  
    }

    if(((ReadEncoderTicker100mS % 1) == 0) && (Tuning == false))loop_esp_now();
    checkSpeakerToggleButton();
    //Serial.println("EFree Heap B4 we continue " + String(ESP.getFreeHeap()));  

    if((ReadEncoderTicker100mS % 10)==0)DataFromGlobe.G_rssi_globe = WiFi.RSSI();


    if((ReadEncoderTicker100mS % 3000)==0) // check every 5 minutes if battery needs recharge
    // if((ReadEncoderTicker100mS % 100)==0) // check every 10 seconds (testing) if battery needs recharge
    { if((DataFromDisplay.D_BatteryVoltage > 30) && (DataFromDisplay.D_BatteryVoltage < 36)) // valid reading but below 3.6 volt
      //if((DataFromDisplay.D_BatteryVoltage > 30) && (DataFromDisplay.D_BatteryVoltage < 50)) // force test
      { // Serial.printf("DataFromDisplay.D_BatteryVoltage = %d\n", DataFromDisplay.D_BatteryVoltage);
        PlaySoundBite((uint8_t *)mp3_please_recharge, sizeof(mp3_please_recharge), 0);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS calculates as 1 and results in 100mS delay here;-)
  }
}

void TaskTouch(void * pvParameters)
{ static uint32_t filtered[16];
  static uint8_t idx = 0;
  static uint32_t sum;
  static uint32_t average;
  static bool erased = false;
  static uint8_t averagestartup = 32;
  static uint16_t touchedcount = 0;
  static bool touch_status = 0;
  static bool previous_touch_status = 0;
  static uint32_t last_short_touch_mS = 0; 
  static uint16_t short_touches = 0;
  static uint16_t recent_short_touches = 0;
  static uint32_t first_short_touch_stopped_at_mS = 0; 
  static uint32_t last_touch_stopped_at_mS = 0; 
  static uint32_t touch_started_at_mS = 0; 
  static uint32_t touch_stopped_at_mS = 0; 
  static uint32_t touch_pressed_mS = 0;
  static uint32_t touch_released_mS = 0;
  static uint32_t touch_volume_off_mS = 0;
  static bool long_press_up;
  static float volume = -1;
  char message[QUEUEMESSAGELENGTH]; 
  uint32_t value;
  static uint16_t ticks;

  while(1)
  { if(IgnoreMqqtUpdates)IgnoreMqqtUpdates--;
    if(!erased) // only once after boot
    { for(int n=0;n<16;n++)filtered[n] = 0;
      sum = 0;
      erased = true;
      
    }

    if(SpeakerOffAfter25mS)
    { SpeakerOffAfter25mS--;
      if(SpeakerOffAfter25mS == 0)
      { digitalWrite(MUTE_AMPLIFIERS, 1);
        Serial.println("Speakers Timeout OFF");
      }
    }


    value = touchRead(8);

    //if(digitalRead(SPEAKER_TOGGLE_PIN) == false)
    //{ Serial.printf("Touchvalue = %ld average =%ld\n", value, average);
    //}
  
    if(averagestartup)
    { averagestartup--;
    }
    else
    { 
      
      if(value > (average + (average/20))) // started with 16, try more sensitive with 20
      { if(touch_status == false)
        { touch_started_at_mS = millis();
          if(recent_short_touches==1)long_press_up = true;
          else long_press_up = false;

          PixelUpdate(0, 0xFFFF00, 0x000000, 1000); // yellow flash 500mS

          DataFromGlobe.G_Volume = DataFromDisplay.volumevalue;
          volume = (float)DataFromGlobe.G_Volume + 0.5; // initialize value
          //Serial.printf("volume = %f\n", volume);
        }
        touch_status = true;
        IgnoreMqqtUpdates = 40;
        touch_pressed_mS = millis() - touch_started_at_mS;
        if(touch_pressed_mS>500)
        { recent_short_touches = 0;
          if(long_press_up == true)
          { //Serial.print("+");
            if(bPowerStatus == true)
            { if(DataFromGlobe.G_Volume<100)
              { volume += 0.25;
                DataFromGlobe.G_Volume = (uint16_t)volume;
              }
            }
            else
            { if(touch_pressed_mS>2000)
              { AddToQueueForDisplay("Requested by Touch Knob", MESSAGE_POWERUP);
                bPowerStatus = true;
              }
            }  
          }
          else
          { //Serial.print("-");
            if(DataFromGlobe.G_Volume)
            { volume -= 0.25;
              DataFromGlobe.G_Volume = (uint16_t)volume;
            Serial.printf("DataFromGlobe.G_Volume = %d\n", DataFromGlobe.G_Volume);
              //PixelSet(8-(((touch_pressed_mS-500)/50)%8));
            }
            else // volume reached or is 0
            { if(touch_volume_off_mS==0)touch_volume_off_mS = millis();
              if((millis() - touch_volume_off_mS)>2000)
              { if(bPowerStatus == true)
                { AddToQueueForDisplay("Requested by Touch Knob", MESSAGE_POWERDOWN);
                  bPowerStatus = false;
                }
              }
            }
          }
        }
      }

      
      else // touch release
      { touch_volume_off_mS = 0;
        
        if(touch_status == true)
        { touch_stopped_at_mS = millis();
          if(touch_pressed_mS>1000)Serial.printf("\n");
          PixelUpdate(0, 0x000000, 0x000000, 5000); // all off
        }
        touch_status = false;
        touch_released_mS = millis() - touch_stopped_at_mS;

        if(touch_pressed_mS && (touch_pressed_mS<150)) // it was a short touch
        { short_touches++;
          if(short_touches==5)Timer100msSerialCanBeOpened = 50; // 5 seconds to open serial port with speaker button
          recent_short_touches = short_touches;
          Serial.printf("Short touch press lasted %ld\n", touch_pressed_mS);
          if(short_touches==1)first_short_touch_stopped_at_mS = touch_stopped_at_mS;
        }  

        if(short_touches && ((touch_released_mS)>300))
        { 
          Serial.printf("Short Touches %d\n", short_touches);
          short_touches = 0;
        }   

        if(recent_short_touches && ((touch_released_mS)>1000))
        { recent_short_touches = 0;
        }

        //Serial.printf("This Short Touch %ld mS\n", touch_pressed_mS);
        last_touch_stopped_at_mS = touch_stopped_at_mS;
        touch_pressed_mS = 0;


      }
      


    }
    
    if(previous_touch_status != touch_status)
    { previous_touch_status = touch_status;
      if(touch_status)
      { touchedcount++;
      }
    }
  
    if(!touch_status)
    { sum -= filtered[idx%16];
      sum += value;
      filtered[idx%16] = value;
      idx++;
      average = sum / 16;
    }  

    ticks++;
    if((ticks%4)==0)PixelUpdate(99, 9, 9, 9); // update the led ring, next step in anmination
    vTaskDelay(25 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS calculates as 1 and results in xx delay here
  }
}

void PixelSet(uint8_t num)
{ 
  for(int n=0; n<8; n++)
  { if(n==num)pixels.setPixelColor(n, pixels.Color(255, 0, 255));
    else pixels.setPixelColor(n, pixels.Color(0, 0, 0));
  }
}

// led ring ideas
// ring mode 0 solid color all leds
// ring mode 1 step left
// ring mode 2 step right
// ring mode 3 auto rotate left
// ring mode 4 auto rotate right
// ring mode 5 flash 4<>4 two colors
// ring mode 6 flash 8/8 one color

// every 100mS
void PixelUpdate(uint16_t mode, uint32_t color1, uint32_t color2, uint16_t msek)
{ static int16_t sstep = 0;
  static int16_t smode = -1;
  static uint32_t scolor1 = 0;
  static uint32_t scolor2 = 0;
  static uint16_t smsek = 5000;
  static uint16_t flash = 0;
  uint16_t n;

//  if((mode!=99) && (smode!=mode))
  if(mode!=99)
  { smode=mode;
    scolor1 = color1;
    scolor2 = color2;
    smsek=msek;
    switch(smode)
    { case 0:
      case 6:
        for(n=0; n<8; n++)pixels.setPixelColor(n, scolor1);
        break;
      case 1:
      case 3:
        sstep--;
        if(sstep<0)sstep=7;
        for(n=0; n<8; n++)pixels.setPixelColor(n, 0x0);
        pixels.setPixelColor(sstep, scolor1);
        break;
      case 2:
      case 4:
        sstep++;
        if(sstep>7)sstep=0;
        for(n=0; n<8; n++)pixels.setPixelColor(n, 0x0);
        pixels.setPixelColor(sstep, scolor1);
        break; 
      case 5:
        for(n=0; n<4; n++)pixels.setPixelColor(n, scolor1);
        for(n=4; n<8; n++)pixels.setPixelColor(n, 0x0);
        break;

      default:
        break;  
    }
    flash = 0;
    pixels.show();
    return;
  }

  if(smsek)
  { smsek -= 100;
    if(smsek<=0)
    { for(n=0; n<8; n++)pixels.setPixelColor(n, 0x0);
      pixels.show();
      smode = -1;
      return;
    }
  }

  flash++;

  //if((flash%5)==0)
  { switch(smode)
    { case 3:
        sstep--;
        //Serial.printf("sstep = %d\n", sstep);
        if(sstep<0)sstep=7;
        for(n=0; n<8; n++)pixels.setPixelColor(n, 0x0);
        pixels.setPixelColor(sstep, scolor1);
        break;
      case 4:
        sstep++;
        //Serial.printf("sstep = %d\n", sstep);
        if(sstep>7)sstep=0;
        for(n=0; n<8; n++)pixels.setPixelColor(n, 0x0);
        pixels.setPixelColor(sstep, scolor1);
        break;
      case 5:
        if((flash%5)==0)
        { if((flash%10)==0)
          { for(n=0; n<4; n++)pixels.setPixelColor(n, scolor1);
            for(n=4; n<8; n++)pixels.setPixelColor(n, 0x0);
          }
          else  
          { for(n=0; n<4; n++)pixels.setPixelColor(n, 0x0);
            for(n=4; n<8; n++)pixels.setPixelColor(n, scolor2);
          }
        }
        break;  
      case 6:
        if((flash%5)==0)
        { if((flash%10)==0)
          { for(n=0; n<8; n++)pixels.setPixelColor(n, scolor1);
          }
          else  
          { for(n=0; n<8; n++)pixels.setPixelColor(n, 0x0);
          }
        }
        break;  

      default:
        break;
    } 
    pixels.show();      
  }

  
}

