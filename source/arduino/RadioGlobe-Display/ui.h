// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui_helpers.h"
#include "ui_events.h"
#include "ui_theme_manager.h"
#include "ui_themes.h"

///////////////////// SCREENS ////////////////////

#include "ui_Home.h"
#include "ui_SetupScreen.h"
#include "ui_CalibrationScreen.h"
#include "ui_DatabaseScreen.h"
#include "ui_PasswordScreen.h"
#include "ui_StationSelectScreen.h"
#include "ui_Favorites.h"
#include "ui_ToneControlScreen.h"
#include "ui_CalibrationScreenAdvanced.h"
#include "ui_Power.h"

///////////////////// VARIABLES ////////////////////


// EVENTS

extern lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE(ui_img_xx_png);    // assets/xx.png
LV_IMG_DECLARE(ui_img_arrow_left_bold_32x32_png);    // assets/arrow_left_bold_32x32.png
LV_IMG_DECLARE(ui_img_arrow_right_bold_32x32_png);    // assets/arrow_right_bold_32x32.png
LV_IMG_DECLARE(ui_img_cog_png);    // assets/cog.png
LV_IMG_DECLARE(ui_img_batterylow_png);    // assets/batterylow.png
LV_IMG_DECLARE(ui_img_batterymedium_png);    // assets/batterymedium.png
LV_IMG_DECLARE(ui_img_batteryhigh_png);    // assets/batteryhigh.png
LV_IMG_DECLARE(ui_img_power75x75_png);    // assets/power75x75.png
LV_IMG_DECLARE(ui_img_home_png);    // assets/home.png
LV_IMG_DECLARE(ui_img_worldmap_bw_360x180_png);    // assets/worldmap_bw_360x180.png
LV_IMG_DECLARE(ui_img_circle16x16_png);    // assets/circle16x16.png
LV_IMG_DECLARE(ui_img_triangle_down_16x16_png);    // assets/triangle_down_16x16.png
LV_IMG_DECLARE(ui_img_power256x256_png);    // assets/power256x256.png

// FONTS
LV_FONT_DECLARE(ui_font_font1);

// UI INIT
void ui_init(void);
void ui_destroy(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
