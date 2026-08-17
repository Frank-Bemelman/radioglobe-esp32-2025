#include "lvgl.h"

#define MAXPLANES 5
#define MAXLINES 3
// global pointers for radarscreen widgets
lv_obj_t * ui_RadarScreen = NULL;
lv_obj_t * uic_RadarScreen = NULL;
lv_obj_t * ui_radar_grid  = NULL;
lv_obj_t * ui_radar_grid1 = NULL;
lv_obj_t * ui_radar_grid2 = NULL;
lv_obj_t * ui_radar_grid3 = NULL;
lv_obj_t * ui_RadarLineVert = NULL;
static lv_point_t vert_points[2];
lv_obj_t * ui_RadarLineHor = NULL;
static lv_point_t hor_points[2];
lv_obj_t * ui_RadarSweep = NULL;
lv_obj_t * uic_RadarSweep = NULL;
static lv_point_t private_points[2];


lv_obj_t * ui_FlightBlips[MAXPLANES] = {0}; // array for 5 airplanes
lv_obj_t * uic_FlightBlips[MAXPLANES] = {0}; 
lv_obj_t * ui_FlightLabels[MAXPLANES][MAXLINES] = {0}; // three text lines of flight data
lv_obj_t * uic_FlightLabels[MAXPLANES][MAXLINES] = {0};

lv_obj_t * ui_CompasLabels[4] = {0};
lv_obj_t * uic_CompasLabels[4] = {0};
const char * CompassCardinals[] = {"N", "E", "S", "W"};

lv_obj_t * ui_PlaneCounter = NULL;
lv_obj_t * uic_PlaneCounter = NULL;

// Globale stijlinstellingen zodat er geen lokale variabelen op de stack belanden
lv_obj_t * ui_TouchPanel = NULL;

#define RADARMAXTIME 300 // max time for radar is 300 second, then close it automatically
uint16_t  RadarDownTimer;


struct FlightBlip {
    int16_t rel_x;
    int16_t rel_y;
    uint16_t heading;
    char callsign[8];
    char type[5];
    uint16_t altitude_ft; // Bevat het Flight Level (bijv. 40 voor 4025 ft)
} __attribute__((packed));

struct RadarDataPacket {
    uint8_t packet_type;  // Wordt 0xAA
    uint8_t num_flights;  // Max 5
    FlightBlip flights[5];
} __attribute__((packed));

// Zorg dat de pointers naar je handgemaakte widgets hier ook bekend zijn:
//extern lv_obj_t * ui_FlightBlips[5];
//extern lv_obj_t * uic_FlightLabels[5];

RadarDataPacket flightdata;
volatile bool bRadarAllowWidgetUpdate = false;

void message_get_flight_date_handler(char *incomingMessage)
{ memcpy(&flightdata, incomingMessage, sizeof(flightdata));

  // Controleer of het unieke ID (0xAA) klopt
  if(flightdata.packet_type == 0xAA) 
  { Serial.println("packet = 0xAA");
    // Neem de LVGL-mutex om veilig in de widgets te schrijven
    if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    { if (lv_scr_act() == ui_RadarScreen && bRadarAllowWidgetUpdate) 
      { int center_x = 240;
        int center_y = 240;
        int scale_factor = 1; // Pas dit aan om in/uit te zoomen op de radar
        Serial.print("packet->num_flights  ->  ");
        Serial.println(flightdata.num_flights);
        Serial.printf("sizeof(RadarDataPacket)=%d\n", sizeof(RadarDataPacket));
        Serial.printf("sizeof(FlightBlip)=%d\n", sizeof(FlightBlip));
               
        for (int i = 0; i < MAXPLANES; i++) 
        { if (i < flightdata.num_flights) 
          { // fade in/out airplanes as they enter or leave the screen, using opa value
            int16_t dist_x = flightdata.flights[i].rel_x * scale_factor;
            int16_t dist_y = flightdata.flights[i].rel_y * scale_factor;
            float abs_distance = sqrt((dist_x * dist_x) + (dist_y * dist_y));
            uint8_t dynamic_opa = 255; // assume max brightness
            if (abs_distance > 180.0) {
                if (abs_distance >= 210.0) {
                    dynamic_opa = 0; // totally invisible when over the edge
                } else {
                    // scale brightness linear between afstand 180 en 210
                    float percentage = (abs_distance - 180.0) / (210.0 - 180.0);
                    dynamic_opa = 255 - (uint8_t)(percentage * 255.0);
                }
            }
            lv_obj_set_style_img_recolor_opa(uic_FlightBlips[i], dynamic_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            
            
            // apply the right offsets to map 0,0 to center of our puck display, for the 32x32 airplane image 
            Serial.printf("x=%d y=%d\n", (int)flightdata.flights[i].rel_x, (int)flightdata.flights[i].rel_y);
            int16_t pixel_x = center_x + (flightdata.flights[i].rel_x * scale_factor) - 16; // 16 half size airplane image
            int16_t pixel_y = center_y - (flightdata.flights[i].rel_y * scale_factor) - 16; // 16 half size airplane image
            Serial.printf("px=%d py=%d\n", pixel_x, pixel_y);
            // position the airplane image
            lv_obj_set_pos(uic_FlightBlips[i], pixel_x, pixel_y);
            lv_img_set_angle(uic_FlightBlips[i], (int)flightdata.flights[i].heading * 10);
            lv_obj_clear_flag(uic_FlightBlips[i], LV_OBJ_FLAG_HIDDEN);
            
            // format de 3 regels tekst (Vluchtnummer, Type, Flight Level)
            // Voorbeeld output: 
            // LXAIE
            // C172
            // 32444 ft

            Serial.printf("Callsign %s\n", flightdata.flights[i].callsign);
            Serial.printf("Type %s\n", flightdata.flights[i].type);
            Serial.printf("Altitude %d\n",  (int)flightdata.flights[i].altitude_ft);
            Serial.printf("Heading %d\n",  (int)flightdata.flights[i].heading);

            for(uint16_t l=0; l<MAXLINES; l++)
            //for(uint16_t l=0; l<1; l++)
            { lv_obj_set_pos(uic_FlightLabels[i][l], pixel_x + 34, pixel_y +(l*17)); // 32 is width of airplane image
              //lv_obj_set_pos(uic_FlightLabels[i][l], pixel_x + 32, pixel_y); // 32 is width of airplane image
              if(l==0)
              { lv_label_set_text_fmt(uic_FlightLabels[i][l], "%s\n%s\n%d ft", 
                                flightdata.flights[i].callsign, 
                                flightdata.flights[i].type,
                                flightdata.flights[i].altitude_ft);
                lv_label_set_text_fmt(uic_FlightLabels[i][l], "%s", 
                                flightdata.flights[i].callsign);                
              }
              else if(l==1)
              { lv_label_set_text_fmt(uic_FlightLabels[i][l], "%s", 
                                flightdata.flights[i].type);
              }                  
              else if(l==2)
              { lv_label_set_text_fmt(uic_FlightLabels[i][l], "%d ft", 
                                flightdata.flights[i].altitude_ft);
              }      
              lv_obj_set_style_text_opa(uic_FlightLabels[i][l], dynamic_opa, LV_PART_MAIN | LV_STATE_DEFAULT);            
              lv_obj_clear_flag(uic_FlightLabels[i][l], LV_OBJ_FLAG_HIDDEN);
            }        
          } 
          else 
          { // Als er minder dan 5 vliegtuigen zijn, verbergen we de overige
            lv_obj_add_flag(uic_FlightBlips[i], LV_OBJ_FLAG_HIDDEN);
            for(uint16_t l=0; l<MAXLINES; l++)
            lv_obj_add_flag(uic_FlightLabels[i][l], LV_OBJ_FLAG_HIDDEN);
          } // if (i < flightdata.num_flights) 
        } // for (int i = 0; i < MAXPLANES; i++)   
        if(flightdata.num_flights>0)
        { lv_label_set_text_fmt(ui_PlaneCounter, "%d PLANES", flightdata.num_flights);
        }
        else 
        { lv_label_set_text_fmt(ui_PlaneCounter, "%c%d.%d - %c%d.%d", (DataFromDisplay.ns_cal<0)?'S':'N', abs(DataFromDisplay.ns_cal)/10, abs(DataFromDisplay.ns_cal)%10, (DataFromDisplay.ew_cal<0)?'W':'E', abs(DataFromDisplay.ew_cal)/10, abs(DataFromDisplay.ew_cal)%10);
        }
        Serial.printf("Puck: %d live vliegtuigen op de radar getekend!\n", flightdata.num_flights);
      } // if (lv_scr_act() == ui_RadarScreen)
      xSemaphoreGive(lvgl_mutex);
    } // if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(20)) == pdTRUE) 
  } // if(flightdata.packet_type == 0xAA) 
}



static lv_timer_t * radar_refresh_timer = NULL;
// timer-callback for requesting flight data
static void radar_refresh_timer_cb(lv_timer_t * timer) 
{ char content[32];
  RadarDownTimer--;
  if(RadarDownTimer>1)
  { sprintf(content, "%d %f %f", 0, (float)DataFromDisplay.ns_cal/10, (float)DataFromDisplay.ew_cal/10);
    AddToQueueForGlobe(content, MESSAGE_GET_FLIGHT_DATA); 
  }
  else 
  { lv_scr_load(uic_Home); 
    Serial.println("Radarscreen clicked for exit");
  }
}

static void radar_screen_event_cb(lv_event_t * e) 
{   char content[32];
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_SCREEN_LOAD_START) 
    { Serial.println("Radar screen event: LV_EVENT_SCREEN_LOAD_START");
      bRadarAllowWidgetUpdate = true;
      RadarDownTimer = RADARMAXTIME;
      // Start de oneindige sweep-animatie pas als het scherm écht activeert!
      lv_anim_t a;
      lv_anim_init(&a);
      lv_anim_set_var(&a, uic_RadarSweep);
      lv_anim_set_exec_cb(&a, radar_sweep_anim_cb);
      lv_anim_set_values(&a, 0, 359);  
      lv_anim_set_time(&a, 5000); // 5 seconden per omwenteling
      lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); 
      lv_anim_start(&a);

      if(radar_refresh_timer == NULL) 
      { radar_refresh_timer = lv_timer_create(radar_refresh_timer_cb, 1000, NULL);
      }
      
      sprintf(content, "%d %f %f", 0, (float)DataFromDisplay.ns_cal/10, (float)DataFromDisplay.ew_cal/10);
      AddToQueueForGlobe(content, MESSAGE_GET_FLIGHT_DATA); 
      return;
   }

    if(code == LV_EVENT_SCREEN_UNLOAD_START) 
    { bRadarAllowWidgetUpdate = false;
      Serial.println("Radar screen event: LV_EVENT_SCREEN_UNLOAD_START");
      // hide aircrafts and labels
      for (int i = 0; i < MAXPLANES; i++) 
      { lv_obj_add_flag(uic_FlightBlips[i], LV_OBJ_FLAG_HIDDEN);
        for(uint16_t l = 0; l < MAXLINES; l++) lv_obj_add_flag(uic_FlightLabels[i][l], LV_OBJ_FLAG_HIDDEN);
      }
      lv_label_set_text(ui_PlaneCounter, "SCANNING");
      if(radar_refresh_timer != NULL) 
      { //lv_timer_pause(radar_refresh_timer); // Of 
        //Serial.println("Radar verlaten: Timer gepauzeerd.");
        lv_timer_del(radar_refresh_timer);
        radar_refresh_timer = NULL;
        Serial.println("Radar screen closed: Timer gedelete.");
      }
      return;
    }

    if(code == LV_EVENT_CLICKED) 
    { Serial.println("Radarscreen clicked for exit");
      beepforMs(50);
      lv_scr_load(uic_Home); 
      return;
    }

    Serial.println("Radarscreen event ????");
}

// De animatie-callback die de groene radarsweep continu rondjes laat draaien
// Array van EXACT 2 punten. Het startpunt staat muurvast in het centrum (240,240).
static lv_point_t radar_line_points[2] = { {240, 240}, {240, 30} }; 

static void radar_sweep_anim_cb(void * var, int32_t v) {
    lv_obj_t * line = (lv_obj_t *)var;
    
    lv_point_t * points = (lv_point_t *)lv_obj_get_user_data(line);
    if (points == NULL) return; // safety

    // do the math
    float angle_rad = (float)v * M_PI / 180.0;
    // length sweeper
    float radius = 224.0; 

    points[0].x = 240;
    points[0].y = 240;

    // end point
    points[1].x = (int16_t)(240.0 + (radius * sin(angle_rad)));
    points[1].y = (int16_t)(240.0 - (radius * cos(angle_rad))); 
    
    // tell lvgl the new position
    lv_line_set_points(line, points, 2);
}














void Create_Manual_Radar_Screen(void) 
{
    // 1. Maak het Radarscherm aan (leeg basisscherm)
    ui_RadarScreen = lv_obj_create(NULL);
    uic_RadarScreen = ui_RadarScreen;
    lv_obj_clear_flag(ui_RadarScreen, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_bg_color(ui_RadarScreen, lv_color_make(0, 10, 0), LV_PART_MAIN); // deep black, hint of green
    lv_obj_add_event_cb(ui_RadarScreen, radar_screen_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(ui_RadarScreen, radar_screen_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    
    // 2. Maak de Radar-Achtergrond (Cirkels en Grid)
    // We gebruiken een simpele ronde LVGL-vlak met een groene rand als radar-grid
    ui_radar_grid = lv_obj_create(ui_RadarScreen);
    lv_obj_set_size(ui_radar_grid, 416, 416);
    lv_obj_align(ui_radar_grid, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(ui_radar_grid, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_radar_grid, LV_OPA_TRANSP, LV_PART_MAIN); // Transparante binnenkant
    lv_obj_set_style_border_color(ui_radar_grid, lv_color_make(0, 150, 0), LV_PART_MAIN); // Donkergroene buitenring
    lv_obj_set_style_border_width(ui_radar_grid, 2, LV_PART_MAIN);

    // Voeg een kleinere binnenring toe voor het diepte-effect
    ui_radar_grid1 = lv_obj_create(ui_RadarScreen);
    lv_obj_set_size(ui_radar_grid1, 312, 312);
    lv_obj_align(ui_radar_grid1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(ui_radar_grid1, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_radar_grid1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_radar_grid1, lv_color_make(0, 128, 0), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_radar_grid1, 2, LV_PART_MAIN);

// Voeg een kleinere binnenring toe voor het diepte-effect
    ui_radar_grid2 = lv_obj_create(ui_RadarScreen);
    lv_obj_set_size(ui_radar_grid2, 208, 208);
    lv_obj_align(ui_radar_grid2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(ui_radar_grid2, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_radar_grid2, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_radar_grid2, lv_color_make(0, 128, 0), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_radar_grid2, 2, LV_PART_MAIN);

    // Voeg een kleinere binnenring toe voor het diepte-effect
    ui_radar_grid3 = lv_obj_create(ui_RadarScreen);
    lv_obj_set_size(ui_radar_grid3, 104, 104);
    lv_obj_align(ui_radar_grid3, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(ui_radar_grid3, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_radar_grid3, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_radar_grid3, lv_color_make(0, 128, 0), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_radar_grid3, 2, LV_PART_MAIN);

    // cross line vertical and horizontal
    ui_RadarLineVert = lv_line_create(ui_RadarScreen);
    vert_points[0].x = 240; vert_points[0].y = 32;  // start top
    vert_points[1].x = 240; vert_points[1].y = 448; // end bottom
    lv_line_set_points(ui_RadarLineVert, vert_points, 2);
    lv_obj_set_size(ui_RadarLineVert, 480, 480);
    lv_obj_align(ui_RadarLineVert, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_line_color(ui_RadarLineVert, lv_color_make(0, 100, 20), LV_PART_MAIN);
    lv_obj_set_style_line_width(ui_RadarLineVert, 2, LV_PART_MAIN); // Strakke, dunne lijn

    ui_RadarLineHor = lv_line_create(ui_RadarScreen);
    hor_points[0].x = 32;  hor_points[0].y = 240; // start left
    hor_points[1].x = 448; hor_points[1].y = 240; // end right
    lv_line_set_points(ui_RadarLineHor, hor_points, 2);
    lv_obj_set_size(ui_RadarLineHor, 480, 480);
    lv_obj_align(ui_RadarLineHor, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_line_color(ui_RadarLineHor, lv_color_make(0, 100, 20), LV_PART_MAIN); 
    lv_obj_set_style_line_width(ui_RadarLineHor, 2, LV_PART_MAIN);


    
    // 4 labels, N, E, S, W
    for (uint16_t i = 0; i < 4; i++) 
    {   ui_CompasLabels[i] = lv_label_create(ui_RadarScreen);
        uic_CompasLabels[i]  = ui_CompasLabels[i];
        lv_obj_set_width(ui_CompasLabels[i], LV_SIZE_CONTENT);   
        lv_obj_set_height(ui_CompasLabels[i], LV_SIZE_CONTENT);    
        // calculate x and y positions for N, E, S and W
        int16_t pos_x = (i == 1) ? 226 : ((i == 3) ? -226 : 0);
        int16_t pos_y = (i == 0) ? -226 : ((i == 2) ? 226 : 0);
        lv_obj_set_x(ui_CompasLabels[i], pos_x);
        lv_obj_set_y(ui_CompasLabels[i], pos_y);
        lv_obj_set_align(ui_CompasLabels[i], LV_ALIGN_CENTER);
        lv_label_set_text(ui_CompasLabels[i], CompassCardinals[i]);
        lv_obj_set_style_text_color(ui_CompasLabels[i], lv_color_hex(0xC0C0C0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui_CompasLabels[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(ui_CompasLabels[i], LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_CompasLabels[i], &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    ui_PlaneCounter = lv_label_create(ui_RadarScreen);
    uic_PlaneCounter = ui_PlaneCounter;
    lv_obj_set_width(ui_PlaneCounter, LV_SIZE_CONTENT);   
    lv_obj_set_height(ui_PlaneCounter, LV_SIZE_CONTENT);    
    lv_obj_set_x(ui_PlaneCounter, 0);
    lv_obj_set_y(ui_PlaneCounter, 180);
    lv_obj_set_align(ui_PlaneCounter, LV_ALIGN_CENTER);
    lv_label_set_text(ui_PlaneCounter, "SCANNING");
    lv_obj_set_style_text_color(ui_PlaneCounter, lv_color_hex(0x3399ff), LV_PART_MAIN | LV_STATE_DEFAULT); // light blue
    lv_obj_set_style_text_opa(ui_PlaneCounter, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_PlaneCounter, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_PlaneCounter, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    
    // make a sweeping green radar scanning line
    ui_RadarSweep = lv_line_create(ui_RadarScreen);
    uic_RadarSweep = ui_RadarSweep;
    private_points[0].x = 240; private_points[0].y = 240;
    private_points[1].x = 240; private_points[1].y = 30;
    lv_obj_set_user_data(ui_RadarSweep, private_points);
    lv_line_set_points(ui_RadarSweep, private_points, 2); 
    lv_obj_set_size(ui_RadarSweep, 480, 480); 
    lv_obj_align(ui_RadarSweep, LV_ALIGN_TOP_LEFT, 0, 0); 
    lv_obj_set_style_line_color(ui_RadarSweep, lv_color_make(0, 255, 0), LV_PART_MAIN); 
    lv_obj_set_style_line_width(ui_RadarSweep, 3, LV_PART_MAIN);

    extern lv_img_dsc_t ui_img_airplane_24x24_png;
    extern lv_img_dsc_t ui_img_airplane_32x32_png;

    // 5 little airplane images
    for (uint16_t i = 0; i < MAXPLANES; i++) 
    { ui_FlightBlips[i] = lv_img_create(ui_RadarScreen);
      uic_FlightBlips[i] = ui_FlightBlips[i];
      //lv_img_set_src(ui_FlightBlips[i], &ui_img_airplane_24x24_png);
      lv_img_set_src(ui_FlightBlips[i], &ui_img_airplane_32x32_png);
      lv_obj_set_width(ui_FlightBlips[i], LV_SIZE_CONTENT);   /// 1
      lv_obj_set_height(ui_FlightBlips[i], LV_SIZE_CONTENT);    /// 1
      lv_obj_set_x(ui_FlightBlips[i], 0);
      lv_obj_set_y(ui_FlightBlips[i], 0);
      lv_obj_center(ui_FlightBlips[i]);
      lv_obj_set_align(ui_FlightBlips[i], LV_ALIGN_TOP_LEFT); // relative to parent
      lv_obj_set_style_img_recolor(ui_FlightBlips[i], lv_color_hex(0xC0C000), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_img_recolor_opa(ui_FlightBlips[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_flag(ui_FlightBlips[i], LV_OBJ_FLAG_HIDDEN);
    }


    // 5 flight informating data boxes
    for (uint16_t i = 0; i < MAXPLANES; i++) 
    {   for (uint16_t l = 0; l < MAXLINES; l++) // three text lines per box
        //for (uint16_t l = 0; l < 1; l++) // three text lines per box
        {  ui_FlightLabels[i][l]  = lv_label_create(ui_RadarScreen);
           uic_FlightLabels[i][l]  = ui_FlightLabels[i][l];
           lv_obj_set_width(ui_FlightLabels[i][l], LV_SIZE_CONTENT);   /// 1
           lv_obj_set_height(ui_FlightLabels[i][l], LV_SIZE_CONTENT);    /// 1
           lv_obj_set_x(ui_FlightLabels[i][l], 0);
           lv_obj_set_y(ui_FlightLabels[i][l], 0);
           lv_obj_set_align(ui_FlightLabels[i][l], LV_ALIGN_TOP_LEFT);
           lv_label_set_text(ui_FlightLabels[i][l], "");
           
           if(l==0)lv_obj_set_style_text_color(ui_FlightLabels[i][l], lv_color_hex(0xC0C0C0), LV_PART_MAIN | LV_STATE_DEFAULT);
           else if(l==1)lv_obj_set_style_text_color(ui_FlightLabels[i][l], lv_color_hex(0x6699ff), LV_PART_MAIN | LV_STATE_DEFAULT);
           else lv_obj_set_style_text_color(ui_FlightLabels[i][l], lv_color_hex(0xffcc66), LV_PART_MAIN | LV_STATE_DEFAULT);
           lv_obj_set_style_text_opa(ui_FlightLabels[i][l], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
           lv_obj_set_style_text_align(ui_FlightLabels[i][l], LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
           lv_obj_set_style_text_font(ui_FlightLabels[i][l], &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT); // &lv_font_montserrat_12 is a bit too small 
           lv_obj_add_flag(ui_FlightLabels[i][l], LV_OBJ_FLAG_HIDDEN);
        }
    }

    ui_TouchPanel = lv_obj_create(ui_RadarScreen);
    lv_obj_set_size(ui_TouchPanel, 480, 480);
    lv_obj_align(ui_TouchPanel, LV_ALIGN_CENTER, 0, 0);
    // make panel totally invisible
    lv_obj_set_style_bg_opa(ui_TouchPanel, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(ui_TouchPanel, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(ui_TouchPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ui_TouchPanel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_TouchPanel, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_add_event_cb(ui_TouchPanel, radar_screen_event_cb, LV_EVENT_CLICKED, NULL);
  
}

