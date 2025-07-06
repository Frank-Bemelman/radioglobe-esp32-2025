#include <math.h>
// calibrations using a table of intersections of latitude lines with longitude lines
// from north pole (900) to south pole (-900)
// 14 rows or ns/ew coordinates, 24 pairs or 48 values from W180 to E180
// (row 0) 900,-1800,900,-1650,900,-1500,900,-1350,900,-1200,900,[....],1500,900,1650,
// (row 1) 750,-1800,750,-1650,750,-1500,750,-1350,750,-1200,750,[....],1050,750,-900,
// (row 2) 600,-1800,600,-1650,600,-1500,600,-1350,600,-1200,600,[....],1050,600,-900,
// (row 3) 450,-1800,450,-1650,450,-1500,450,-1350,450,-1200,450,[....],1050,450,-900,
// (row 4) 300,-1800,349,-1641,349,-1488,300,-1350,300,-1200,300,[....],-1050,300,-900,
// (row 5) 150,-1800,190,-1639,190,-1487,150,-1350,150,-1200,150,[....],-1050,150,-900,
// (row 6) 0,-1800,0,-1650,0,-1500,0,-1350,0,-1200,0,-1050,0,-900,[....],0,-750,0,-600,  // two rows for equator as top/bottom halves may not align lingitudal wise
// (row 7) 0,-1800,0,-1650,0,-1500,0,-1350,0,-1200,0,-1050,0,-900,[....],0,-750,0,-600,  // two rows for equator as top/bottom halves may not align lingitudal wise
// (row 8) -150,-1800,-150,-1650,-150,-1500,-150,-1350,-150,-1200,[....],-150,-1050,-150,
// (row 9) -300,-1800,-300,-1650,-300,-1500,-300,-1350,-300,-1200,[....],-300,-1050,-300,
// (row 10) -450,-1800,-450,-1650,-450,-1500,-450,-1350,-450,-1200,[....],-450,-1050,-450,
// (row 11) -600,-1800,-600,-1650,-600,-1500,-600,-1350,-600,-1200,[....],-600,-1050,-600,
// (row 12) -750,-1800,-750,-1650,-750,-1500,-750,-1350,-750,-1200,[....],-750,-1050,-750,
// (row 13) -900,-1800,-900,-1650,-900,-1500,-900,-135,-750,-1200,[....],-750,-1050,-750,






//quadriliteral to normalized 
  ns_ew_intersection src_i16[4];
  ns_ew_intersection dst_i16[4];

int16_t remapped_ns;
int16_t remapped_ew;


bool CalibrationSpinLock;

// SET AS ZERO BUTTON
void CalibrationSetAsZero(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
  }
}


// LATITUDES BUTTON
void CalibrationLatitudes(lv_event_t *e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    CalibrationSpinLock = true;
    CalibrationModeLatLong = CALMODE_NS;
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    lv_obj_add_flag(uic_RollerEW, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(uic_RollerNS, LV_OBJ_FLAG_HIDDEN);
    lv_scr_load(ui_CalibrationScreenAdvanced);
    // early initialization of CalToGps label
    lv_event_t * e;
    RollerNSorEWChanged(e); 
    Lvgl_Loop();
  }
}

// LONGITUDES BUTTON
void CalibrationLongitudes(lv_event_t *e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    //CalibrationSpinLock = true;
    CalibrationModeLatLong = CALMODE_EW;
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    lv_obj_add_flag(uic_RollerNS, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(uic_RollerEW, LV_OBJ_FLAG_HIDDEN);
    lv_scr_load(ui_CalibrationScreenAdvanced);
    // early initialization of CalToGps label
    lv_event_t * e;
    RollerNSorEWChanged(e); 
    Lvgl_Loop();
  }  
}

// LONGITUDES BUTTON
void CalibrationAdvanced(lv_event_t * e)
{ if(isLongPressed(e)==5)
  { beepforMs(1000);
    //CalibrationSpinLock = true;
    CalibrationModeLatLong = CALMODE_NSEW;
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    lv_obj_clear_flag(uic_RollerNS, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(uic_RollerEW, LV_OBJ_FLAG_HIDDEN);
    lv_scr_load(ui_CalibrationScreenAdvanced);
    // early initialization of CalToGps label
    lv_event_t * e;
    RollerNSorEWChanged(e); 
    Lvgl_Loop();
  }  
}



void CalibratePoint(lv_event_t * e)
{ static uint16_t longpressed=0;
  char content[32];

  if(isLongPressed(e)==5)
  { if(CalibrationSpinLock)return;
    // store current coordinates in calibration array
    // CalibrationModeLatLong reflects CALMODE_NS, CALMODE_EW or both
    if(CalibrationModeLatLong !=CALMODE_NSEW)
    { // do a full latitude or longitude
      if(CalibrationModeLatLong == CALMODE_NS)
      { for(int16_t ew = 0; ew<24; ew++)
        { ns_ew_calibrations.calibrations[CalToIndexNS][ew].ns = DataFromGlobe.ns;
        }
        GetFormattedLocation(content, "G", CalibrationModeLatLong); // update label under calibration button
      }
      else if(CalibrationModeLatLong == CALMODE_EW)
      { for(int16_t ns = 0; ns<14; ns++)
        { ns_ew_calibrations.calibrations[ns][CalToIndexEW].ew = DataFromGlobe.ew;
        }
      }
    }  
    else // do a single latitude/longitude point
    { ns_ew_calibrations.calibrations[CalToIndexNS][CalToIndexEW].ns = DataFromGlobe.ns;
      ns_ew_calibrations.calibrations[CalToIndexNS][CalToIndexEW].ew = DataFromGlobe.ew;
      if(CalToIndexNS==6) // N-1, also store ns value to S-1
      { ns_ew_calibrations.calibrations[CalToIndexNS+1][CalToIndexEW].ns = DataFromGlobe.ns;
      }
      if(CalToIndexNS==7) // S-1, also store ns value to N-1
      { ns_ew_calibrations.calibrations[CalToIndexNS-1][CalToIndexEW].ns = DataFromGlobe.ns;
      }
    } 
    Serial.printf("Store ns_idx=%d ew_idx=%d ns = %d ew = %d\n", CalToIndexNS, CalToIndexEW, ns_ew_calibrations.calibrations[CalToIndexNS][CalToIndexEW].ns, ns_ew_calibrations.calibrations[CalToIndexNS][CalToIndexEW].ew);
    // update label under calibration button
    GetFormattedLocation(content, "C", CalibrationModeLatLong); // update label under calibration button
    lv_label_set_text(ui_CalToStored, content);   
    remap_ns_ew(DataFromGlobe.ns, DataFromGlobe.ew);
    SaveCalibrations();  
    // todo set label text warning, first adjust rollers for next calib 
    CalibrationSpinLock = true;
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    beepforMs(1000);
    lv_label_set_text(ui_SpinLock, "Calibration Executed"); 
  }
}

void CalibrationInit(void)
{
  // first, load array with perfect values to start with.
  // typedef struct calibrations_arraybin
  // {  ns_ew_intersection calibrations[(180/15)+2][360/15]; // 14 positions for north to south, 0-15-30-45-60-75-90
  // };
  
  // remember, coordinates are in whole tenths of a degree, so is always multiplied by 10.

  // north
  int16_t idx = 0;
  for(int16_t n=6; n>=0; n--)
  { for(int16_t ew=0; ew<24; ew++)
    { ns_ew_calibrations.calibrations[idx][ew].ns  = n*150;
      ns_ew_calibrations.calibrations[idx][ew].ew  = -1800 + (ew*150);
    }
    idx++;
  }

  // south
  for(int16_t n=0; n<7; n++)
  { for(int16_t ew=0; ew<24; ew++)
    { ns_ew_calibrations.calibrations[idx][ew].ns  = -n*150;
      ns_ew_calibrations.calibrations[idx][ew].ew  = -1800 + (ew*150);
    }
    idx++;
  }

  // keep a copy of virgin values
  memcpy(&def_cal, &ns_ew_calibrations, sizeof(def_cal)); 



  // get position of the calibratio roller
  CalToIndexEW = lv_roller_get_selected(uic_RollerEW);
  CalToIndexNS = lv_roller_get_selected(uic_RollerNS);

   // early initialization of CalToGps label
   lv_event_t * e;
   RollerNSorEWChanged(e); 
    
}

void SaveCalibrations(void)
{
   //{  ns_ew_intersection calibrations_north[(90/15)+1][360/15]; // 7 positions for north, 0-15-30-45-60-75-90
   //   ns_ew_intersection calibrations_south[(90/15)+1][360/15]; // 7 positions for south, 0-15-30-45-60-75-90
   //} ns_ew_calibrations;

   // remember, coordinates are in whole tenths of a degree, so is always multiplied by 10.

   // store as lines of txt, first the north hemispehere, followed by south hemispere
  File calfile;
  char calfilename[] = {"/calibrations.txt"};
  char *p;

  if (!SD_MMC.begin("/sdcard", true, false))return; // no card

  SD_MMC.remove(calfilename); // delete old file
  calfile = SD_MMC.open(calfilename, FILE_WRITE);
  if(calfile)
  { for(int16_t n=0; n<14; n++)
    { for(int16_t ew=0; ew<24; ew++)
      { calfile.print(ns_ew_calibrations.calibrations[n][ew].ns);
        calfile.print(",");
        calfile.print(ns_ew_calibrations.calibrations[n][ew].ew);
        calfile.print(",");
      }
      calfile.print("\n");
    }
  }
    
  calfile.close();
  SD_MMC.end();    
}


void RollerNSorEWChanged(lv_event_t * e)
{ char content[256];
  static int16_t prevCalToIndexNS;
  static int16_t prevCalToIndexEW;
  CalToIndexNS = lv_roller_get_selected(uic_RollerNS)+1; // plus 1, because roller does not begin with 90
  CalToIndexEW = lv_roller_get_selected(uic_RollerEW);

  //Serial.printf("CalToIndexNS = %d CalToIndexEW = %d CalibrationModeLatLong=%d\n", CalToIndexNS, CalToIndexEW, CalibrationModeLatLong);

  // if truly new postion chosen, enable calibration again
  if((prevCalToIndexNS!=CalToIndexNS) || (prevCalToIndexEW!=CalToIndexEW))
  { prevCalToIndexNS=CalToIndexNS;
    prevCalToIndexEW=CalToIndexEW;
    CalibrationSpinLock = false;
    lv_obj_clear_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    lv_label_set_text(ui_SpinLock, "Check Twice Before Calibrating"); 
  }  

  if(CalToIndexNS<14)
  { GetFormattedLocation(content, "C", CalibrationModeLatLong);
    lv_label_set_text(ui_CalToStored, content);                                          
  }                                          
}


// called upon receiving new position from globe
void remap_ns_ew(int16_t from_globe_ns, int16_t from_globe_ew)
{ // remap coordinates
  // find the 'square' where we are in
  int16_t square_ns;
  int16_t square_ew;
  int32_t square_area;
  int32_t coord_area;
  int16_t scenario = 1234;
  
  static uint32_t lapMillis;
  static uint32_t currentMillis;


  lapMillis = millis(); // let's check how long this all takes

  // coordinates from globe are simply what the encoders gives us.
  // since the globe is not very accurately printed, these coordinates need to remapped
  // first make a good guess in which square on the globe we are
  // square_ns 0-5 is for north hemispehere  and 7-12 for south hemispere
  // square_ns 6 is skipped for good reasons - to deal with jagged longitudes at the equator
  // as seen with some low quality globes - those squares are place holder for calibrations being different 
  // for just above and below equator
  // checking and finding the right square may consume aroud 15ms
  // recalculating the position inside the warped/skewed square on the globe is another 50ms-60ms

  // first guess in which square we probably are - this will certainly be the case when in the center of a square
  square_ns = (-from_globe_ns + 900) / 150;
  square_ew = (from_globe_ew + 1800) / 150;

  if(square_ew>=24)square_ew%=24; // watch out for the pitfall at E180W
  if(square_ns ==6)square_ns++; // skip 6

  // first snap calibration rollers to nearest lat/long lines on the globe
  if(from_globe_ns>0) // north pole is not to be calibrated, all others are
  { // sync NS and EW rollers used in the advance calibration menu
    // snap to nearest NS latitude
    lv_roller_set_selected(uic_RollerNS, ((-from_globe_ns + 900+75) / 150)-1, LV_ANIM_ON); // minus 1 because NS roller starts at N75

    // snap to nearest EW longitude
    // when coming from E165 towards E180W the snapped square is 24 which is ok because the roller has E180 and W180 positions
    // therefore, we don't need to mod EW with %24
    lv_roller_set_selected(uic_RollerEW, ((from_globe_ew + 1800+75) / 150), LV_ANIM_ON);
    Lvgl_Loop();
    lv_event_t * e;
    RollerNSorEWChanged(e); 
  }
  else
  { lv_roller_set_selected(uic_RollerNS, ((-from_globe_ns + 900+75) / 150)-1, LV_ANIM_ON); // not minus 1 because NS roller two values around equator
    lv_roller_set_selected(uic_RollerEW, ((from_globe_ew + 1800+75) / 150), LV_ANIM_ON);
    Lvgl_Loop();
    CalibrationSpinLock = true; // keep the button locked
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    lv_label_set_text(ui_SpinLock, "- North Pole Is Fixed -"); 
  }

  Serial.printf("\nMAPPING STARTED square_ns=%d square_ew=%d globe_ns=%d globe_ew=%d\n", square_ns, square_ew, from_globe_ns, from_globe_ew);

  // guessed the square, but maybe the wrong square, if just close to the border of the square
  check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);

  while(scenario>0)
  { switch(scenario)
    { case 1:  // UP - UP/RIGHT - RIGHT or ns-- ew++ ns++
        safe_ns_calc(-1, &square_ns);
        check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
        if(scenario!=0)
        { safe_ew_calc(1, &square_ew);
          check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
          if(scenario!=0)
          { safe_ns_calc(1, &square_ns);
            check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
            if(scenario!=0)
            { Serial.println("SCENARIO FAIL 11111111111111 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK IT OUT");
              square_ew++; // back to first gamble, but why
              scenario=-1;
            }
          }        
        }
        break;
      case 2: // RIGHT - DOWN/RIGHT - DOWN or ew++ ns++ ew--
        safe_ew_calc(1, &square_ew);
        check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
        if(scenario!=0)
        { safe_ns_calc(1, &square_ns);
          check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
          if(scenario!=0)
          { safe_ew_calc(-1, &square_ew);
            check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
            if(scenario!=0)
            { Serial.println("SCENARIO FAIL 22222222222222 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK IT OUT");
              square_ew++; // back to first gamble, but why
              scenario=-1;
            }
          }        
        }
        break;
      case 3: // DOWN - DOWN/LEFT - LEFT or ns++ ew-- ns--
        safe_ns_calc(1, &square_ns);
        check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
        if(scenario!=0)
        { safe_ew_calc(-1, &square_ew);
          check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
          if(scenario!=0)
          { safe_ns_calc(-1, &square_ns);
            check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
            if(scenario!=0)
            { Serial.println("SCENARIO FAIL 33333333333333 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK IT OUT");
              square_ew++; // back to first gamble, but why
              scenario=-1;
            }
          }        
        }
        break;
      case 4: // LEFT - UP/LEFT - UP or ew-- ns-- ew++
        safe_ew_calc(-1, &square_ew);
        check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
        if(scenario!=0)
        { safe_ns_calc(-1, &square_ns);
          check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
          if(scenario!=0)
          { safe_ew_calc(1, &square_ew);
            check_outside_area(square_ns, square_ew, from_globe_ns, from_globe_ew, &square_area, &coord_area, &scenario);
            if(scenario!=0)
            { Serial.println("SCENARIO FAIL 44444444444444 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK IT OUT");
              square_ew++; // back to first gamble, but why
              scenario=-1;
            }
          }        
        }
        break;
      default:
        break;  
    }
  }


  // time to remap
  // map the coordinate with a span of 150 in both directions to the actual square we landed in

  // adjust the perfect but raw position to the actual warped/skewed square we are in
  int16_t mapped_ns;
  int16_t mapped_ew;
  remap_by_square_number(square_ns, square_ew, from_globe_ns, from_globe_ew, &mapped_ns, &mapped_ew);
  DataFromDisplay.ns_cal = remapped_ns;
  DataFromDisplay.ew_cal = remapped_ew;

  Serial.printf("MAPPING FINISHED -> time elapsed = %ld\n", (currentMillis = millis()) - lapMillis);

}


void check_outside_area(int16_t square_ns, int16_t square_ew, int16_t coord_ns, int16_t coord_ew, int32_t *square_area, int32_t *coord_area, int16_t *scenario)
{ // calculate if we are outside the given area by square_ns and square_ew
  // find four corners of square, lt top-left, rt top-right, bl bottom-left and br bottm-right
  // without printf the time is less than 1ms so quite affordable to calculate all this stuff
  // when outside the square calculate which direction the fitting square is
  
  //Serial.printf("Check (scenario %d) if outside square_ns=%d square_ew=%d globe_ns=%d globe_ew=%d\n", *scenario, square_ns, square_ew, coord_ns, coord_ew);

  int32_t ltns = ns_ew_calibrations.calibrations[square_ns][square_ew].ns; // y4
  int32_t ltew = ns_ew_calibrations.calibrations[square_ns][square_ew].ew; // x4

  int32_t rtns = ns_ew_calibrations.calibrations[square_ns][(square_ew+1)%24].ns; // y3
  int32_t rtew = ns_ew_calibrations.calibrations[square_ns][(square_ew+1)%24].ew; // x3

  int32_t lbns = ns_ew_calibrations.calibrations[(square_ns+1)%12][square_ew].ns; // y1 
  int32_t lbew = ns_ew_calibrations.calibrations[(square_ns+1)%12][square_ew].ew; // x1

  int32_t rbns = ns_ew_calibrations.calibrations[(square_ns+1)%12][(square_ew+1)%24].ns; // y2
  int32_t rbew = ns_ew_calibrations.calibrations[(square_ns+1)%12][(square_ew+1)%24].ew; // x2

  if(square_ew==23) // fix for using negative coordinates from square 0
  { rtew = 1800;
    rbew = 1800;
  }

  //Serial.printf("ltns=%ld ltew=%ld\n", ltns, ltew);
  //Serial.printf("rtns=%ld rtew=%ld\n", rtns, rtew);
  //Serial.printf("lbns=%ld lbew=%ld\n", lbns, lbew);
  //Serial.printf("rbns=%ld rbew=%ld\n", rbns, rbew);

  // first step is to calculate the total area this square occupies
  // shoelace formula -> area = 0.5 * |(x1y2 + x2y3 + x3y4 + x4y1) - (y1x2 + y2x3 + y3x4 + y4x1)|
  int32_t area = abs((((lbew*rbns)+(rbew*rtns)+(rtew*ltns)+(ltew*lbns)) - ((lbns*rbew)+(rbns*rtew)+(rtns*ltew)+(ltns*lbew)))) / 2;
  *square_area = area; 
  
  
  // now calculate sum of all 4 triangles from postion to corners of square
  // if the total area of that is larger than the area of the square, we are outside the square
  int32_t area1 = area_triangle(coord_ew, coord_ns, ltew, ltns, rtew, rtns); // relative to top line of square
  //Serial.printf("area1=%ld\n", area1);
  int32_t area2 = area_triangle(coord_ew, coord_ns, rtew, rtns, rbew, rbns); // relative to right line of square
  //Serial.printf("area2=%ld\n", area2);
  int32_t area3 = area_triangle(coord_ew, coord_ns, lbew, lbns, rbew, rbns); // relative to bottom line of square
  //Serial.printf("area3=%ld\n", area3);
  int32_t area4 = area_triangle(coord_ew, coord_ns, lbew, lbns, ltew, ltns); // relative to left line of square
  //Serial.printf("area4=%ld\n", area4);
  
  int32_t totalarea = area1+area2+area3+area4;
  *coord_area = totalarea; 

  if(totalarea>area) // we are outside the square
  { // we are outside the square
    // figure out if we need to move down, right, up or left

    if(area1<area3) // true square is higher
    { if(area2<area4)*scenario = 1; 
      else *scenario = 4; 
    }
    else // true square is lower
    { if(area2<area4)*scenario = 2; 
      else *scenario = 3; 
    }
    Serial.printf("OUTSIDE SQ-NS-%d-EW%d -> total area=%ld vs square area=%ld scenario=%d\n", square_ns, square_ew, totalarea, area, *scenario);
    //Serial.printf("dist %d %d %d %d biggest = %d\n", dist[0], dist[1], dist[2], dist[3], biggest+1);
  }

  *scenario = 0;
  Serial.printf("INSIDE SQ-NS-%d-EW%d -> total area=%ld vs square area=%ld scenario=%d\n", square_ns, square_ew, totalarea, area, *scenario);
}


int32_t area_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3)
{ // int32_t area;
  float area;
  // formula area = 1/2 * |x₁(y₂ - y₃) + x₂(y₃ - y₁) + x₃(y₁ - y₂)|

  x1 += 900;
  x2 += 900;
  x3 += 900;
  y1 += 1800;
  y2 += 1800;
  y3 += 1800;

//  area = abs(   x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2)   ) / 2;
  area = abs(   (float)x1*((float)y2-(float)y3) + (float)x2*((float)y3-(float)y1) + (float)x3*((float)y1-(float)y2)   ) / (float)2;

  return (int32_t)area;
}


void remap_by_square_number(int16_t sq_ns, int16_t sq_ew, int16_t coord_ns, int16_t coord_ew, int16_t* mapped_ns, int16_t* mapped_ew)
{ //quadriliteral coordinates
  double src[4][2] = { {100, 100}, {300, 100}, {300, 300}, {100, 300} };
  double dst[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

  src[0][0] = (double) ns_ew_calibrations.calibrations[(sq_ns+1)%12][sq_ew].ew; 
  src[0][1] = (double) ns_ew_calibrations.calibrations[(sq_ns+1)%12][sq_ew].ns; 
  src[1][0] = (double) ns_ew_calibrations.calibrations[(sq_ns+1)%12][(sq_ew+1)%24].ew; 
  src[1][1] = (double) ns_ew_calibrations.calibrations[(sq_ns+1)%12][(sq_ew+1)%24].ns; 
  src[2][0] = (double) ns_ew_calibrations.calibrations[sq_ns][(sq_ew+1)%24].ew; 
  src[2][1] = (double) ns_ew_calibrations.calibrations[sq_ns][(sq_ew+1)%24].ns; 
  src[3][0] = (double) ns_ew_calibrations.calibrations[sq_ns][sq_ew].ew; 
  src[3][1] = (double) ns_ew_calibrations.calibrations[sq_ns][sq_ew].ns; 

  //Serial.printf("For square ns=%d ew=%d coord_ns=%d coord_ew=%d\n", sq_ns, sq_ew, coord_ns, coord_ew);
  //Serial.print("From: ");
  //for(int16_t from =0; from<4; from++)
  //{ Serial.printf("x %f <> y.%f , ", src[from][0], src[from][1] );  
  //}
  //Serial.println(" Done.");

  dst[0][0] = (double) def_cal.calibrations[(sq_ns+1)%12][sq_ew].ew; 
  dst[0][1] = (double) def_cal.calibrations[(sq_ns+1)%12][sq_ew].ns; 
  dst[1][0] = (double) def_cal.calibrations[(sq_ns+1)%12][(sq_ew+1)%24].ew; 
  dst[1][1] = (double) def_cal.calibrations[(sq_ns+1)%12][(sq_ew+1)%24].ns; 
  dst[2][0] = (double) def_cal.calibrations[sq_ns][(sq_ew+1)%24].ew; 
  dst[2][1] = (double) def_cal.calibrations[sq_ns][(sq_ew+1)%24].ns; 
  dst[3][0] = (double) def_cal.calibrations[sq_ns][sq_ew].ew; 
  dst[3][1] = (double) def_cal.calibrations[sq_ns][sq_ew].ns; 

  //Serial.print("To:   ");
  //for(int16_t to =0; to<4; to++)
  //{ Serial.printf("x %f <> y.%f , ", dst[to][0], dst[to][1] );  
  //}
  //Serial.println(" Done.");


  double H[3][3];
  solveHomography(src, dst, H);
  double x = 150, y = 150;
  double u, v;

  x = (double)coord_ew;
  y = (double)coord_ns;
  applyHomography(H, x, y, &u, &v);

  //printf("Mapped point: (%f, %f) -> (%f, %f)\n", x, y, u, v);

  remapped_ew = (int16_t)(u+0.5);
  remapped_ns = (int16_t)(v+0.5);

  //printf("remapped point: (%d, %d)\n", remapped_ew, remapped_ns);


  *mapped_ns = remapped_ns;
  *mapped_ns = remapped_ew;

}


void LoadCalibrations(void)
{ File calfile;
  //char calfilename[] = {"/calibrations - kopie.txt"};
  char calfilename[] = {"/calibrations.txt"};
  char c;
  int16_t valcount = 0; 
  int16_t linescount = 0; 
  char valuetext[32];
  int16_t valuetext_idx = 0; 
  
  
  if (!SD_MMC.begin("/sdcard", true, false))return; // no card

  calfile = SD_MMC.open(calfilename, FILE_READ);
  if(calfile)
  { while(calfile.available() && (linescount<12)) 
    { c = calfile.read(); 
//      Serial.write(c); 
      switch(c)
      { case 0x0d:
        case 0x0a:
          break;
        case ',':
//          Serial.println(valuetext);  
//          Serial.print(linescount);  
//          Serial.print("-");  
//          Serial.println(valcount);  
          valuetext_idx = 0;
          if((valcount%2)==0)
          { ns_ew_calibrations.calibrations[linescount][valcount/2].ns = atoi(valuetext); 
          }
          else
          { ns_ew_calibrations.calibrations[linescount][valcount/2].ew = atoi(valuetext); 
          }
          valcount++;
          if(valcount>=48)
          { valcount = 0;
            linescount++;
          }
          break;
        default:
          if(valuetext_idx<31)
          { valuetext[valuetext_idx++] = c;  
            valuetext[valuetext_idx] = 0;  
          }  
          break;
      }   
    }
  }
  Serial.println("Finished loading calibrations from SD");
  calfile.close();
  SD_MMC.end();
}


// 8 equations, 8 unknowns (h11..h32), h33 = 1
#define NEQ 8   

void solveHomography(double src[4][2], double dst[4][2], double H[3][3]) {
    double A[NEQ][NEQ + 1]; // augmented matrix for 8 equations + 1 result column

    // build system of equations
    for (int i = 0; i < 4; ++i) {
        double x = src[i][0];
        double y = src[i][1];
        double u = dst[i][0];
        double v = dst[i][1];

        int row1 = 2 * i;
        int row2 = row1 + 1;

        // Equation 1
        A[row1][0] = x;
        A[row1][1] = y;
        A[row1][2] = 1;
        A[row1][3] = 0;
        A[row1][4] = 0;
        A[row1][5] = 0;
        A[row1][6] = -x * u;
        A[row1][7] = -y * u;
        A[row1][8] = u;

        // Equation 2
        A[row2][0] = 0;
        A[row2][1] = 0;
        A[row2][2] = 0;
        A[row2][3] = x;
        A[row2][4] = y;
        A[row2][5] = 1;
        A[row2][6] = -x * v;
        A[row2][7] = -y * v;
        A[row2][8] = v;
    }

    // Solve using Gaussian elimination
    for (int i = 0; i < NEQ; ++i) {
        // Pivot
        int maxRow = i;
        for (int k = i + 1; k < NEQ; ++k)
            if (fabs(A[k][i]) > fabs(A[maxRow][i])) maxRow = k;

        // Swap
        for (int k = 0; k <= NEQ; ++k) {
            double tmp = A[i][k];
            A[i][k] = A[maxRow][k];
            A[maxRow][k] = tmp;
        }

        // Eliminate
        for (int k = i + 1; k < NEQ; ++k) {
            double c = A[k][i] / A[i][i];
            for (int j = i; j <= NEQ; ++j)
                A[k][j] -= c * A[i][j];
        }
    }

    // Back substitution
    double h[8];
    for (int i = NEQ - 1; i >= 0; --i) {
        h[i] = A[i][NEQ];
        for (int j = i + 1; j < NEQ; ++j)
            h[i] -= A[i][j] * h[j];
        h[i] /= A[i][i];
    }

    // Construct the 3x3 matrix (h33 = 1)
    H[0][0] = h[0]; H[0][1] = h[1]; H[0][2] = h[2];
    H[1][0] = h[3]; H[1][1] = h[4]; H[1][2] = h[5];
    H[2][0] = h[6]; H[2][1] = h[7]; H[2][2] = 1.0;
}

// apply the homography to a point
void applyHomography(double H[3][3], double x, double y, double *u, double *v) {
    double denom = H[2][0]*x + H[2][1]*y + H[2][2];
    *u = (H[0][0]*x + H[0][1]*y + H[0][2]) / denom;
    *v = (H[1][0]*x + H[1][1]*y + H[1][2]) / denom;
}

// to move between squares om the map without worrying about overflowing 
void safe_ns_calc(int16_t dev, int16_t *square_ns)
{ *square_ns += dev;
  if(*square_ns==6)*square_ns += dev;
  if(*square_ns<0)*square_ns+=12;
  if(*square_ns>=12)*square_ns%=12;
}

// to move between squares om the map without worrying about overflowing 
void safe_ew_calc(int16_t dev, int16_t *square_ew)
{ *square_ew += dev;
  if(*square_ew<0)*square_ew+=24;
  if(*square_ew>=24)*square_ew%=24;
}

