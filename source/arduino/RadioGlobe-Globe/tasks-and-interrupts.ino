
void setup_interrupts(void)
{
  // timer for led strip timing en magneet timing
  //timer = timerBegin(1000000); // prescaler 1MHz
  //timerAttachInterrupt(timer, &onTimer);
  //timerAlarm(timer, 200000, true, 0); // elke 100mS test 200ms

  xTaskCreatePinnedToCore(
                    ReadAS5600Encoders,   /* Task function. */
                    "ReadAS5600Encoders",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */      

  xTaskCreatePinnedToCore(
                    CallGetTimeZone,   /* Task function. */
                    "CallGetTimeZone",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */      



}

void CallGetTimeZone(void * pvParameters)
{ while(1)
  { if(DataFromGlobe.FindTimeZone == 1)
    {  GetTimeZone(DataFromDisplay.ns_cal, DataFromDisplay.ew_cal);
       DataFromGlobe.FindTimeZone = 0;
       PrevDataFromDisplay.ns_cal = DataFromDisplay.ns_cal;
       PrevDataFromDisplay.ew_cal = DataFromDisplay.ew_cal;
      
    }
    vTaskDelay(200 / portTICK_PERIOD_MS); 
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
  
  while(1)
  { ReadEncoderTicker100mS++;
    // keep 4 last readings
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
    if((abs(NSFilteredRAW - AverageNS[(AverageIdx-1)%4])<8) && (abs(EWFilteredRAW - AverageEW[(AverageIdx-1)%4])<8)) stable = true;
    else stable = false;

    NSDegLive10 = NSFilteredRAW * 3600 / 4096; // convert to degrees times 10
    EWDegLive10 = EWFilteredRAW * 3600 / 4096; // convert to degrees times 10

    DataFromGlobe.ns = NSDegLive10;
    DataFromGlobe.ew = EWDegLive10;

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

        bEncoderNewPosition = true; // signal to main program to pick another station
      }  
    }

    if((ReadEncoderTicker100mS % 2) == 0)loop_esp_now();

    if(stable)
    { if((PrevDataFromDisplay.ns_cal != DataFromDisplay.ns_cal) || (PrevDataFromDisplay.ew_cal != DataFromDisplay.ew_cal))
      { DataFromGlobe.FindTimeZone = 1;
      }
    }  

    vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS calculates as 1 and results in 50mS delay here;-)
  }
}

