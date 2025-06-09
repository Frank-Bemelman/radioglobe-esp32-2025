// list of calibration prompts

struct calibration_prompts {
    int16_t ns;
    int16_t ew;
    char* prompt;
    uint8_t area_lb;
    uint8_t area_rb;
    uint8_t area_lt;
    uint8_t area_rt;
     //
};

struct area_xy {
    int16_t area_lb_ns;
    int16_t area_rb_ns;
    int16_t area_lt_ns;
    int16_t area_rt_ns;
    int16_t area_lb_ew;
    int16_t area_rb_ew;
    int16_t area_lt_ew;
    int16_t area_rt_ew;
     //
};

area_xy area_xy_table[32];

// array of prompts
// the idea is to divide the earth in 32 equal areas
// 16 of them in the northern hemisphere
// 16 of them in the southern hemisphere
// each hemisphere is divided in two rows of 8 segments 
// of which 8 are towards the poles and 8 towards the equator
// the prompts below are to instruct to user to calibrate the position of these segments
// to be used later to adjust the values from both encoders by interpolation
// coordinates are in degrees times 10
calibration_prompts cal_prompts[] PROGMEM = {
    450,     0, "Move To N45 & EW0",    0,  7,  8, 15, // 0 lb, rb, lt, rt
    450,   450, "Move To N45 & E45",    1,  0,  9,  8, // 1
    450,   900, "Move To N45 & E90",    2,  1, 10,  9, // 2
    450,  1350, "Move To N45 & E135",   3,  2, 11, 10, // 3
    450,  1800, "Move To N45 & EW180",  4,  3, 12, 11, // 4
    450, -1350, "Move To N45 & W135",   5,  4, 13, 12, // 5
    450,  -900, "Move To N45 & W90",    6,  5, 14, 13, // 6
    450,  -450, "Move To N45 & W45",    7,  6, 15, 14, // 7

      0,     0, "Move To N1 &  EW0",    8, 15, 99, 99, 
      0,   450, "Move To N1 & E45",     9,  8, 99, 99,
      0,   900, "Move To N1 & E90",    10,  9, 99, 99,
      0,  1350, "Move To N1 & E135",   11, 10, 99, 99,
      0,  1800, "Move To N1 & EW180",  12, 11, 99, 99,
      0, -1350, "Move To N1 & W135",   13, 12, 99, 99,
      0,  -900, "Move To N1 & W90",    14, 13, 99, 99,
      0,  -450, "Move To N1 & W45",    15, 14, 99, 99,

      0,     0, "Move To S1 &  EW0",   99, 99, 16, 23, 
      0,   450, "Move To S1 & E45",    99, 99, 17, 16,
      0,   900, "Move To S1 & E90",    99, 99, 18, 17,
      0,  1350, "Move To S1 & E135",   99, 99, 19, 18,
      0,  1800, "Move To S1 & EW180",  99, 99, 20, 19,
      0, -1350, "Move To S1 & W135",   99, 99, 21, 20,
      0,  -900, "Move To S1 & W90",    99, 99, 22, 21,
      0,  -450, "Move To S1 & W45",    99, 99, 23, 22,

   -450,     0, "Move To S45 & EW0",   16, 23, 24, 31,  // 0 lb, rb, lt, rt
   -450,   450, "Move To S45 & E45",   17, 16, 25, 24,
   -450,   900, "Move To S45 & E90",   18, 17, 26, 25,
   -450,  1350, "Move To S45 & E135",  19, 18, 27, 26,
   -450,  1800, "Move To S45 & EW180", 20, 19, 28, 27,
   -450, -1350, "Move To S45 & W135",  21, 20, 29, 28,
   -450,  -900, "Move To S45 & W90",   22, 21, 30, 29,
   -450,  -450, "Move To S45 & W45",   23, 22, 31, 30,
};

//EOF