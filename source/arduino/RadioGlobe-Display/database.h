// useful stuff for database.ino 

// plan is to implement a monochrome bitmap of the entire world
// for every degree longitude (ew, 360) and lattitude (ns, 180) there is one bit or pixel
// pixeldata 360x180/8 = 8100 bytes

// bitmap files have this header, a 54 byte structure
#pragma once

typedef struct BMPHeader{          // Total: 54 bytes
  uint16_t  filler;                // puts all uint32_t values here on uint32_t boundaries
  uint16_t  type = 0x4d42;         // Magic identifier: 0x4d42
  uint32_t  size = 8162;           // File size in bytes 54 + 8 + 8100
  uint16_t  reserved1 = 0;         // Not used
  uint16_t  reserved2 = 0;         // Not used
  uint32_t  offset = 62;           // Offset to image data in bytes from beginning of file (54 + 8 bytes)
  uint32_t  dib_header_size = 40;  // DIB Header size in bytes (40 bytes)
  uint32_t   width_px = 360;       // Width of the image
  uint32_t   height_px = 180;      // Height of image
  uint16_t  num_planes = 1;        // Number of color planes
  uint16_t  bits_per_pixel = 1;    // Bits per pixel
  uint32_t  compression = 0;       // Compression type
  uint32_t  image_size_bytes = 0;  // Image size in bytes, dummy 0 for BI_RGB or 8640??
  int32_t   x_resolution_ppm = 2;  // Pixels per meter
  int32_t   y_resolution_ppm = 2;  // Pixels per meter
  uint32_t  num_colors = 2;        // Number of colors  
  uint32_t  important_colors = 0;  // Important colors 
};

typedef struct BMPColorTable
{
    uint8_t blue_off = 0x0;     ///< Blue component
    uint8_t green_off = 0x0;    ///< Green component
    uint8_t red_off = 0x0;      ///< Red component
    uint8_t reserved_off = 0x0; ///< Should be 0.
    uint8_t blue_on = 0xff;     ///< Blue component
    uint8_t green_on = 0xff;    ///< Green component
    uint8_t red_on = 0xff;      ///< Red component
    uint8_t reserved_on = 0x0;  ///< Should be 0.
};

typedef struct bmpfile{                // Total: 54 + 8 + 8100 bytes
  BMPHeader bmpheader;          // 54 bytes
  BMPColorTable bmpct;          // 8 bytes
  uint8_t  pixeldata[8640];     // 180 ns entries of 360/8=45 ew bytes, round up to multiple of 4 bytes -> 48 -> 180x48 -> 8460 bytes of pixeldata
}; 

#define MAX_STATIONS 150
#define MAX_FAVORITES 4

typedef struct station_name_url_gps
{ char name[32];
  char url[128];
  char town[32]; // derived from town_cc.txt filename
  char countrycode[3]; // derived from town_cc.txt filename
  char countryname[50]; // derived from town_cc.txt filename
  float gps_ns; // gps position to use for timezone retrieval
  float gps_ew; // gps position to use for timezone retrieval
};

typedef struct stations_arraybin
{ station_name_url_gps StationNUG[MAX_STATIONS+MAX_FAVORITES];
  uint16_t count; // number of found stations in this list
  int16_t  requested; // number of station requested by display to globe
  int16_t  playing; // number of station that globe connected to
  uint16_t connect_attempts; // number of times globe was asked to connect to a station from this list
};



