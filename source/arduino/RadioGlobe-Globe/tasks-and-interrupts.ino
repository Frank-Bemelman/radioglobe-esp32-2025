// tasks

void setup_tasks(void)
{
  xTaskCreatePinnedToCore(
                    CheckUrl,   /* Task function. */
                    "CheckUrl",     /* name of task. */
                    25000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */      

  xTaskCreatePinnedToCore(
                    CallGetTimeZone,   /* Task function. */
                    "CallGetTimeZone",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
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
}



void CallGetTimeZone(void * pvParameters)
{ while(1)
  { if(DataFromGlobe.FindTimeZone == 1)
    {  GetTimeZone(DataFromDisplay.D_StationGpsNS, DataFromDisplay.D_StationGpsEW); 
       DataFromGlobe.FindTimeZone = 0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // lowered to 100, was 200
  }  
}

// Task to read the globe encoders
void ReadAS5600Encoders(void * pvParameters)
{ int16_t AverageNS[4];
  int16_t AverageEW[4];
  int8_t  AverageIdx = 0;
  bool  stable = false;
  bool  stable_changed = true;
  int16_t NSFilteredRAW;
  int16_t EWFilteredRAW;
  uint16_t stable100ms = 0;
  bool EncoderReliable = false; // 
  
  while(1)
  { ReadEncoderTicker100mS++;
    // keep 4 last readings
    if(!EncoderReliable)
    { if(ReadEncoderTicker100mS == 25)
      { EncoderReliable = true;
        DataFromGlobe.G_EncoderReliable = true;
      }
    }
    AverageIdx %= 4;
    AverageNS[AverageIdx] = as5600_0.readAngle()-2048;
    AverageEW[AverageIdx] = as5600_1.readAngle()-2048;
    AverageIdx++;

    // get the average
    if(!stable)
    { NSFilteredRAW = (AverageNS[0] + AverageNS[1] + AverageNS[2] + AverageNS[3]) / 4;
     EWFilteredRAW = (AverageEW[0] + AverageEW[1] + AverageEW[2] + AverageEW[3]) / 4;
    }  

    // check if last reading were fairly different from the average
    if((abs(NSFilteredRAW - AverageNS[(AverageIdx-1)%4])<8) && (abs(EWFilteredRAW - AverageEW[(AverageIdx-1)%4])<8))
    { if(stable100ms<10000)stable100ms++;
      if(stable100ms>5)stable = true;
    }  
    else
    { stable = false;
      stable100ms = 0;
    }
    NSDegLive10 = NSFilteredRAW * 3600 / 4096; // convert to degrees times 10
    EWDegLive10 = EWFilteredRAW * 3600 / 4096; // convert to degrees times 10

    // make available to display 
    if((NSDegLive10>=-900) && (NSDegLive10<=900)) DataFromGlobe.ns = NSDegLive10;
    if((EWDegLive10>=-1800) && (EWDegLive10<=1800)) DataFromGlobe.ew = EWDegLive10;

    if(stable_changed != stable)
    { stable_changed = stable;
      if(stable)
      { Serial.println("Stable");
        NSDeg10 = NSDegLive10;
        EWDeg10 = EWDegLive10;
        //Serial.print("readAngle NS = ");Serial.print(NSDeg10/10.0);
        //Serial.print("\tReadAGC NS = "); Serial.println(as5600_0.readAGC());
        //Serial.print("readAngle EW = ");Serial.print(EWDeg10/10.0);
        //Serial.print("\tReadAGC EW = "); Serial.println(as5600_1.readAGC());
        // todo - calibrate
        gNSCal10 = NSDeg10; 
        gEWCal10 = EWDeg10;

        // wait until really stable, after the first search has been done
        if(EncoderReliable)bEncoderNewPosition = true; // signal to main program to pick another station
      }  
    }

    if(((ReadEncoderTicker100mS % 1) == 0) && (Tuning == false))loop_esp_now();
    //Serial.println("EFree Heap B4 we continue " + String(ESP.getFreeHeap()));   
    vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS calculates as 1 and results in 50mS delay here;-)
  }
}

