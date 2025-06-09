// calibrate/recalculate coordinates from encoders
// use the calibartion table
// snap to nearest station
// 

#include "calibrations.h"


void calibrate_globe(void)
{ int c, n;
  as5600_0.setOffset(0);
  as5600_1.setOffset(0);
  GlobeSettings.Offset0 = (2048-as5600_0.rawAngle());
  Serial.print("Offset measured 0: ");
  Serial.println(GlobeSettings.Offset0);
  GlobeSettings.Offset1 = (2048-as5600_1.rawAngle());
  Serial.print("Offset measured 1: ");
  Serial.println(GlobeSettings.Offset1);
  as5600_0.setOffset(GlobeSettings.Offset0 * AS5600_RAW_TO_DEGREES);
  as5600_1.setOffset(GlobeSettings.Offset1 * AS5600_RAW_TO_DEGREES);
  EEPROM.put(0x0, GlobeSettings);
  EEPROM.commit();
  Serial.println("Calibration Of The Globe done.");
  
  // build area_xy_table
  for(c=0;c<32;c++)
  { // find lb
    area_xy_table[c].area_lb_ns = 0;
    area_xy_table[c].area_lb_ew = 0;
    area_xy_table[c].area_rb_ns = 0;
    area_xy_table[c].area_rb_ew = 0;
    area_xy_table[c].area_lt_ns = 0;
    area_xy_table[c].area_lt_ew = 0;
    area_xy_table[c].area_rt_ns = 0;
    area_xy_table[c].area_rt_ew = 0;
    if(c<8)
    { area_xy_table[c].area_lt_ns = 900;
      area_xy_table[c].area_rt_ns = 900;
    }
    if(c>23)
    { area_xy_table[c].area_lb_ns = -900;
      area_xy_table[c].area_rb_ns = -900;
    }


//    for(n=0; n<32; n++)
//    { if(c == cal_prompts[n].area_lb)
//      { area_xy_table[c].area_lb_ns = GlobeSettings[n].Offset0;
//        area_xy_table[c].area_lb_ew = GlobeSettings[n].Offset1;
//      }
//      if(c == cal_prompts[n].area_rb)
//      { area_xy_table[c].area_rb_ns = GlobeSettings[n].Offset0;
//        area_xy_table[c].area_rb_ew = GlobeSettings[n].Offset1;
//      }
//      if(c == cal_prompts[n].area_lt)
//      { area_xy_table[c].area_lt_ns = GlobeSettings[n].Offset0;
//        area_xy_table[c].area_lt_ew = GlobeSettings[n].Offset1;
//      }
//      if(c == cal_prompts[n].area_rt)
//      { area_xy_table[c].area_rt_ns = GlobeSettings[n].Offset0;
//        area_xy_table[c].area_rt_ew = GlobeSettings[n].Offset1;
//     }
//    }

  }
}