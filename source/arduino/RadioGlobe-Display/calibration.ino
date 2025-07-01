#include <math.h>
// calibrations using a table of intersections of lattitude lines with longitudat lines
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


// LATTITUDES BUTTON
void CalibrationLattitudes(lv_event_t *e)
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
    { // do a full lattitude or longitude
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
    else // do a single lattitude/longitude point
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
    remap_ns_ew();
    SaveCalibrations();  
    // todo set label text warning, first adjust rollers for next calib 
    CalibrationSpinLock = true;
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    bUpdateCoordinates = true;
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
void remap_ns_ew(void)
{ // remap coordinates
  // find the 'square' where we are in
  static int16_t old_square_ns;
  static int16_t old_square_ew;

  int16_t square_ns;
  int16_t square_ew;
  int16_t move_dir;
  
  static uint32_t lapMillis;
  static uint32_t currentMillis;


  lapMillis = millis();

  // coordinates from globe are simply what the encoders gives us.
  // since the globe is not very accurately printed, these coordinates need to remapped
  //
  //Serial.printf("DataFromGlobe.ns=%d DataFromGlobe.ew=%d\n", DataFromGlobe.ns, DataFromGlobe.ew);

  // first make a good guess in which square on the globe we are
  // square_ns 0-5 is for north hemispehere  and 7-12 for south hemispere
  // square_ns 6 does not exist for good reasons...  

  square_ns = (-DataFromGlobe.ns + 900+75) / 150;
  square_ew = (DataFromGlobe.ew + 1800+75) / 150;
  if(DataFromGlobe.ns<0) // below equator, move down one line in the table
  { square_ns++;
  }  

  if(square_ns>0) // north pole is not to be calibrated, all others are
  { // sync NS and EW rollers used in the advance calibration menu
    lv_roller_set_selected(uic_RollerNS, square_ns-1, LV_ANIM_ON); // minus 1 because NS roller starts at N75
    lv_roller_set_selected(uic_RollerEW, square_ew, LV_ANIM_ON);
    Lvgl_Loop();
    // early initialization of CalToGps label
     lv_event_t * e;
     RollerNSorEWChanged(e); 
  }
  else
  { lv_roller_set_selected(uic_RollerNS, square_ns, LV_ANIM_ON); // minus 1 because NS roller starts at N75
    lv_roller_set_selected(uic_RollerEW, square_ew, LV_ANIM_ON);
    Lvgl_Loop();
    CalibrationSpinLock = true;
    lv_obj_add_state(uic_CalibrationPointText, LV_STATE_DISABLED);
    lv_label_set_text(ui_SpinLock, "- North Pole Is Fixed -"); 
  }



  Serial.printf("\nsquare_ns=%d square_ew=%d globe_ns=%d globe_ew=%d\n", square_ns, square_ew, DataFromGlobe.ns, DataFromGlobe.ew);

  // found the square, but maybe the wrong square, if just close to the border of the square
  move_dir = check_outside_area(square_ns, square_ew, DataFromGlobe.ns, DataFromGlobe.ew);

  int16_t n = 8; // max 4 tries
  while(move_dir && n--)
  { Serial.printf("Move -> %d\n", move_dir);
    
    // todo - handling around equator - DOUBLE SKIP

    if(move_dir==1)
     { square_ns++; // move down a row
       if(square_ns==6) square_ns++; // skip the 6
     }

    if(move_dir==2)square_ew--;

    if(move_dir==3)
     { square_ns--; // move up a row
       if(square_ns==6) square_ns--; // skip the 6
     }

     if(move_dir==4)square_ew++;
     // play safe
     if(square_ns<0)square_ns+=12;
     if(square_ew<0)square_ew+=24;
     if(square_ns>=12)square_ns%=12;
     if(square_ew>=24)square_ew%=24;
    
     move_dir = check_outside_area(square_ns, square_ew, DataFromGlobe.ns, DataFromGlobe.ew);
  }

  if(move_dir)Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK IT OUT");

  Serial.printf("Mapping finished -> time elapsed = %ld\n", (currentMillis = millis()) - lapMillis);

  // time to remap
  // map the coordinate with a span of 150 in both directions to the actual square we landed in

  // 
  int16_t mapped_ns;
  int16_t mapped_ew;
  
//  map_to_square(square_ns, square_ew, DataFromGlobe.ns, DataFromGlobe.ew, &mapped_ns, &mapped_ew);


  remap_by_square_number(square_ns, square_ew, DataFromGlobe.ns, DataFromGlobe.ew, &mapped_ns, &mapped_ew);

  //Serial.printf("Hee mapped_ns=%d mapped_ew=%d\n", remapped_ns, remapped_ew);

  DataFromDisplay.ns_cal = remapped_ns;
  DataFromDisplay.ew_cal = remapped_ew;

}


int16_t check_outside_area(int16_t square_ns, int16_t square_ew, int16_t coord_ns, int16_t coord_ew)
{ // calculate if we are outside the given area by square_ns and square_ew
  // find four corners of square, lt top-left, rt top-right, bl bottom-left and br bottm-right
  // without printf the time is less than 1ms so quite affordable to calculate all this stuff
  
  //Serial.printf("check if outside square_ns=%d square_ew=%d\n", square_ns, square_ew);
  
  int32_t ltns = ns_ew_calibrations.calibrations[square_ns][square_ew].ns; // y4
  int32_t ltew = ns_ew_calibrations.calibrations[square_ns][square_ew].ew; // x4

  int32_t rtns = ns_ew_calibrations.calibrations[square_ns][(square_ew+1)%24].ns; // y3
  int32_t rtew = ns_ew_calibrations.calibrations[square_ns][(square_ew+1)%24].ew; // x3

  int32_t lbns = ns_ew_calibrations.calibrations[(square_ns+1)%12][square_ew].ns; // y1 
  int32_t lbew = ns_ew_calibrations.calibrations[(square_ns+1)%12][square_ew].ew; // x1

  int32_t rbns = ns_ew_calibrations.calibrations[(square_ns+1)%12][(square_ew+1)%24].ns; // y2
  int32_t rbew = ns_ew_calibrations.calibrations[(square_ns+1)%12][(square_ew+1)%24].ew; // x2

  //Serial.printf("ltns=%ld ltew=%ld\n", ltns, ltew);
  //Serial.printf("rtns=%ld rtew=%ld\n", rtns, rtew);
  //Serial.printf("lbns=%ld lbew=%ld\n", lbns, lbew);
  //Serial.printf("rbns=%ld rbew=%ld\n", rbns, rbew);

  // first step is to calculate the total area this square occupies
  // shoelace formula -> area = 0.5 * |(x1y2 + x2y3 + x3y4 + x4y1) - (y1x2 + y2x3 + y3x4 + y4x1)|
  int32_t area = abs((((lbew*rbns)+(rbew*rtns)+(rtew*ltns)+(ltew*lbns)) - ((lbns*rbew)+(rbns*rtew)+(rtns*ltew)+(ltns*lbew)))) / 2;
  //Serial.printf("area=%ld\n", area);
  
  
  // now calculate sum of all 4 triangles from postion to corners of square
  // if the total area of that is larger than the area of the square, we are outside the square
  int32_t area1 = area_triangle(coord_ew, coord_ns, ltew, ltns, rtew, rtns);
  //Serial.printf("area1=%ld\n", area1);
  int32_t area2 = area_triangle(coord_ew, coord_ns, rtew, rtns, rbew, rbns);
  //Serial.printf("area2=%ld\n", area2);
  int32_t area3 = area_triangle(coord_ew, coord_ns, lbew, lbns, rbew, rbns);
  //Serial.printf("area3=%ld\n", area3);
  int32_t area4 = area_triangle(coord_ew, coord_ns, lbew, lbns, ltew, ltns);
  //Serial.printf("area3=%ld\n", area3);
  
  int32_t totalarea = area1+area2+area3+area4;
  //Serial.printf("total area=%ld vs sqaure area=%ld\n", totalarea, area);

  if(totalarea>area) // we are outside the square
  { // we are outside the square
    // figure out if we need to move down, right, up or left, biggest distance (cursor to top, right, bottomt, left, ells us
    int16_t dist[4];
    dist[0] = abs( ((ltns+rtns)/2) - coord_ns); // distance to top
    dist[1] = abs( ((rtew+rbew)/2) - coord_ew); // distance to right
    dist[2] = abs( ((lbns+rbns)/2) - coord_ns); // distance to bottom
    dist[3] = abs( ((ltew+lbew)/2) - coord_ew); // distance left
    // find biggest distance
    int16_t biggest = 0;
    for(int16_t n=1; n<4; n++)
    { if(dist[n]>dist[biggest])biggest = n;
    }

    //Serial.printf("dist %d %d %d %d biggest = %d\n", dist[0], dist[1], dist[2], dist[3], biggest+1);
    return biggest+1;
  }

  return 0;

}


int32_t area_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3)
{ int32_t area;
  // formula area = 1/2 * |x₁(y₂ - y₃) + x₂(y₃ - y₁) + x₃(y₁ - y₂)|
  area = abs(   x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2)                ) / 2;
  return area;
}

// replaced by remap_by_square_number() not used anymore
void map_to_square(int16_t square_ns, int16_t square_ew, int16_t coord_ns, int16_t coord_ew, int16_t* mapped_ns, int16_t* mapped_ew)
{ // map(value, fromLow, fromHigh, toLow, toHigh)
  // def_cal
  // int32_t ltns = ns_ew_calibrations.calibrations[square_ns][square_ew].ns; // y4
  // int32_t ltew = ns_ew_calibrations.calibrations[square_ns][square_ew].ew; // x4

  // should also interpolate 2 dimensional

  int16_t fromlow = ns_ew_calibrations.calibrations[square_ns][square_ew].ns; 
  int16_t fromhigh = ns_ew_calibrations.calibrations[(square_ns+1)%12][square_ew].ns;
  int16_t tolow = def_cal.calibrations[square_ns][square_ew].ns;
  int16_t tohigh = def_cal.calibrations[(square_ns+1)%12][square_ew].ns;

  //Serial.printf("MAP NScoord %d (square_ns=%d)-> %d  %d  %d  %d\n", coord_ns,square_ns, fromlow, fromhigh, tolow, tohigh);

  *mapped_ns = map(coord_ns, fromlow, fromhigh, tolow, tohigh);

  fromlow = ns_ew_calibrations.calibrations[square_ns][square_ew].ew; 
  fromhigh = ns_ew_calibrations.calibrations[square_ns][(square_ew+1)%24].ew;
  tolow = def_cal.calibrations[square_ns][square_ew].ew;
  tohigh = def_cal.calibrations[square_ns][(square_ew+1)%24].ew;

  //Serial.printf("MAP EWcoord %d (square_ew=%d) -> %d  %d  %d  %d\n", coord_ew, square_ew, fromlow, fromhigh, tolow, tohigh);

  *mapped_ew = map(coord_ew, fromlow, fromhigh, tolow, tohigh);



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
  // }
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
    double A[NEQ][NEQ + 1]; // Augmented matrix for 8 equations + 1 result column

    // Build system of equations
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

// Apply the homography to a point
void applyHomography(double H[3][3], double x, double y, double *u, double *v) {
    double denom = H[2][0]*x + H[2][1]*y + H[2][2];
    *u = (H[0][0]*x + H[0][1]*y + H[0][2]) / denom;
    *v = (H[1][0]*x + H[1][1]*y + H[1][2]) / denom;
}





/*

#include <stdio.h>
#include <stdlib.h>

#define N 8   // 8 equations, 8 unknowns (h11..h32), h33 = 1

void solveHomography(double src[4][2], double dst[4][2], double H[3][3]) {
    double A[N][N + 1]; // Augmented matrix for 8 equations + 1 result column

    // Build system of equations
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
    for (int i = 0; i < N; ++i) {
        // Pivot
        int maxRow = i;
        for (int k = i + 1; k < N; ++k)
            if (fabs(A[k][i]) > fabs(A[maxRow][i])) maxRow = k;

        // Swap
        for (int k = 0; k <= N; ++k) {
            double tmp = A[i][k];
            A[i][k] = A[maxRow][k];
            A[maxRow][k] = tmp;
        }

        // Eliminate
        for (int k = i + 1; k < N; ++k) {
            double c = A[k][i] / A[i][i];
            for (int j = i; j <= N; ++j)
                A[k][j] -= c * A[i][j];
        }
    }

    // Back substitution
    double h[8];
    for (int i = N - 1; i >= 0; --i) {
        h[i] = A[i][N];
        for (int j = i + 1; j < N; ++j)
            h[i] -= A[i][j] * h[j];
        h[i] /= A[i][i];
    }

    // Construct the 3x3 matrix (h33 = 1)
    H[0][0] = h[0]; H[0][1] = h[1]; H[0][2] = h[2];
    H[1][0] = h[3]; H[1][1] = h[4]; H[1][2] = h[5];
    H[2][0] = h[6]; H[2][1] = h[7]; H[2][2] = 1.0;
}

// Apply the homography to a point
void applyHomography(double H[3][3], double x, double y, double *u, double *v) {
    double denom = H[2][0]*x + H[2][1]*y + H[2][2];
    *u = (H[0][0]*x + H[0][1]*y + H[0][2]) / denom;
    *v = (H[1][0]*x + H[1][1]*y + H[1][2]) / denom;
}


int main() {
    double src[4][2] = { {100, 100}, {300, 100}, {300, 300}, {100, 300} };
    double dst[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

    double H[3][3];
    solveHomography(src, dst, H);

    double x = 150, y = 150;
    double u, v;
    applyHomography(H, x, y, &u, &v);

    printf("Mapped point: (%f, %f) -> (%f, %f)\n", x, y, u, v);

    return 0;
}



---------------------------------------




















voor int16_t


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define N 8 // 8 equations for 8 unknowns

void solveHomographyInt16(int16_t src_i16[4][2], int16_t dst_i16[4][2], double H[3][3]) {
    double src[4][2], dst[4][2];

    // Convert int16_t to double for computation
    for (int i = 0; i < 4; ++i) {
        src[i][0] = (double)src_i16[i][0];
        src[i][1] = (double)src_i16[i][1];
        dst[i][0] = (double)dst_i16[i][0];
        dst[i][1] = (double)dst_i16[i][1];
    }

    double A[N][N + 1];

    for (int i = 0; i < 4; ++i) {
        double x = src[i][0];
        double y = src[i][1];
        double u = dst[i][0];
        double v = dst[i][1];

        int r1 = 2 * i;
        int r2 = r1 + 1;

        // First equation
        A[r1][0] = x;
        A[r1][1] = y;
        A[r1][2] = 1;
        A[r1][3] = 0;
        A[r1][4] = 0;
        A[r1][5] = 0;
        A[r1][6] = -x * u;
        A[r1][7] = -y * u;
        A[r1][8] = u;

        // Second equation
        A[r2][0] = 0;
        A[r2][1] = 0;
        A[r2][2] = 0;
        A[r2][3] = x;
        A[r2][4] = y;
        A[r2][5] = 1;
        A[r2][6] = -x * v;
        A[r2][7] = -y * v;
        A[r2][8] = v;
    }

    // Gaussian elimination
    for (int i = 0; i < N; ++i) {
        int maxRow = i;
        for (int k = i + 1; k < N; ++k)
            if (fabs(A[k][i]) > fabs(A[maxRow][i]))
                maxRow = k;

        // Swap rows
        for (int k = 0; k <= N; ++k) {
            double tmp = A[i][k];
            A[i][k] = A[maxRow][k];
            A[maxRow][k] = tmp;
        }

        // Eliminate
        for (int k = i + 1; k < N; ++k) {
            double c = A[k][i] / A[i][i];
            for (int j = i; j <= N; ++j)
                A[k][j] -= c * A[i][j];
        }
    }

    // Back-substitution
    double h[8];
    for (int i = N - 1; i >= 0; --i) {
        h[i] = A[i][N];
        for (int j = i + 1; j < N; ++j)
            h[i] -= A[i][j] * h[j];
        h[i] /= A[i][i];
    }

    // Final 3x3 homography matrix
    H[0][0] = h[0]; H[0][1] = h[1]; H[0][2] = h[2];
    H[1][0] = h[3]; H[1][1] = h[4]; H[1][2] = h[5];
    H[2][0] = h[6]; H[2][1] = h[7]; H[2][2] = 1.0;
}

// Apply the homography to int16 input coordinates
void applyHomographyToInt16(double H[3][3], int16_t x, int16_t y, double *u, double *v) {
    double dx = (double)x;
    double dy = (double)y;
    double denom = H[2][0]*dx + H[2][1]*dy + H[2][2];

    *u = (H[0][0]*dx + H[0][1]*dy + H[0][2]) / denom;
    *v = (H[1][0]*dx + H[1][1]*dy + H[1][2]) / denom;
}



int main() {
    int16_t src[4][2] = { {100, 100}, {200, 100}, {200, 200}, {100, 200} };
    int16_t dst[4][2] = { {0, 0}, {100, 0}, {100, 100}, {0, 100} };

    double H[3][3];
    solveHomographyInt16(src, dst, H);

    int16_t x = 150, y = 150;
    double u, v;
    applyHomographyToInt16(H, x, y, &u, &v);

    printf("Mapped (%d, %d) -> (%f, %f)\n", x, y, u, v);

    return 0;
}





*/

