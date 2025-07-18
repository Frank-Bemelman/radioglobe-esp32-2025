#pragma once
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/task.h"

#include "TCA9554PWR.h"
#include "LVGL_Driver.h"
#include "Touch_CST820.h"

#define LCD_CLK_PIN   2
#define LCD_MOSI_PIN  1 
#define LCD_Backlight_PIN   6 
// Backlight   
#define PWM_Channel     1       // PWM Channel   
#define Frequency       20000   // PWM frequencyconst         
#define Resolution      10       // PWM resolution ratio     MAX:13
#define Dutyfactor      500     // PWM Dutyfactor      
#define Backlight_MAX   100      

// FB Seen elsewhere: https://www.openpilot.cc/archives/4466
//    #define ESP_PANEL_LCD_RGB_CLK_HZ            (16 * 1000 * 1000) same
//    #define ESP_PANEL_LCD_RGB_HPW               (4) // 4 vs 8 here
//    #define ESP_PANEL_LCD_RGB_HBP               (8) // 8 vs 10  here
//    #define ESP_PANEL_LCD_RGB_HFP               (8)  // 8 vs 50  here
//    #define ESP_PANEL_LCD_RGB_VPW               (4) // 4 vs 3  here
//    #define ESP_PANEL_LCD_RGB_VBP               (16) // 16 vs 8  here
//    #define ESP_PANEL_LCD_RGB_VFP               (16) 16 vs 8  here

#define ESP_PANEL_LCD_WIDTH                       (480)
#define ESP_PANEL_LCD_HEIGHT                      (480)
#define ESP_PANEL_LCD_COLOR_BITS                  (16)
#define ESP_PANEL_LCD_RGB_PIXEL_BITS              (16)    // 24 | 16
#define ESP_PANEL_LCD_RGB_DATA_WIDTH              (16)
#define ESP_PANEL_LCD_RGB_TIMING_FREQ_HZ          (16 * 1000 * 1000)  
#define ESP_PANEL_LCD_RGB_TIMING_HPW              (8)
#define ESP_PANEL_LCD_RGB_TIMING_HBP              (10)
#define ESP_PANEL_LCD_RGB_TIMING_HFP              (50)
#define ESP_PANEL_LCD_RGB_TIMING_VPW              (4) // changed from 3 to 4 18-JUL-25 FB just a gamble to fix the display glitches 
#define ESP_PANEL_LCD_RGB_TIMING_VBP              (8)
#define ESP_PANEL_LCD_RGB_TIMING_VFP              (8)
#define ESP_PANEL_LCD_RGB_FRAME_BUF_NUM           (2)     // 1/2/3
#define ESP_PANEL_LCD_RGB_BOUNCE_BUF_SIZE         (ESP_PANEL_LCD_WIDTH * 10)     // Bounce buffer size in bytes. This function is used to avoid screen drift.
                                                          // To enable the bounce buffer, set it to a non-zero value. Typically set to `ESP_PANEL_LCD_WIDTH * 10`
                                                          // The size of the Bounce Buffer must satisfy `width_of_lcd * height_of_lcd = size_of_buffer * N`,
                                                          // where N is an even number.


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your board spec ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ESP_PANEL_LCD_PIN_NUM_RGB_HSYNC           (38)
#define ESP_PANEL_LCD_PIN_NUM_RGB_VSYNC           (39)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DE              (40)
#define ESP_PANEL_LCD_PIN_NUM_RGB_PCLK            (41)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA0           (5)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA1           (45)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA2           (48)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA3           (47)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA4           (21)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA5           (14)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA6           (13)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA7           (12)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA8           (11)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA9           (10)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA10          (9)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA11          (46)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA12          (3)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA13          (8)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA14          (18)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DATA15          (17)
#define ESP_PANEL_LCD_PIN_NUM_RGB_DISP            (-1)

#define ESP_PANEL_LCD_BK_LIGHT_ON_LEVEL           (1)
#define ESP_PANEL_LCD_BK_LIGHT_OFF_LEVEL !ESP_PANEL_LCD_BK_LIGHT_ON_LEVEL

#define EXAMPLE_ENABLE_PRINT_LCD_FPS            (0)

extern uint8_t LCD_Backlight;
extern esp_lcd_panel_handle_t panel_handle;   
bool example_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);
void ST7701_Init();

void LCD_Init();
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend,uint8_t* color);

// backlight
void Backlight_Init();
void Set_Backlight(uint8_t Light);  