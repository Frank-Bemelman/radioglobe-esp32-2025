// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "ui.h"

lv_obj_t * uic_Big_Power_Off_Icon;
lv_obj_t * uic_Power;
lv_obj_t * ui_Power = NULL;
lv_obj_t * ui_Big_Power_Off_Icon = NULL;
// event funtions
void ui_event_Big_Power_Off_Icon(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        PowerCycle(e);
    }
    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
        PowerCycle(e);
    }
}

// build funtions

void ui_Power_screen_init(void)
{
    ui_Power = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Power, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_Big_Power_Off_Icon = lv_img_create(ui_Power);
    lv_img_set_src(ui_Big_Power_Off_Icon, &ui_img_power256x256_png);
    lv_obj_set_width(ui_Big_Power_Off_Icon, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Big_Power_Off_Icon, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_Big_Power_Off_Icon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Big_Power_Off_Icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_Big_Power_Off_Icon, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_Big_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_red);
    ui_object_set_themeable_style_property(ui_Big_Power_Off_Icon, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_red);

    lv_obj_add_event_cb(ui_Big_Power_Off_Icon, ui_event_Big_Power_Off_Icon, LV_EVENT_ALL, NULL);
    uic_Power = ui_Power;
    uic_Big_Power_Off_Icon = ui_Big_Power_Off_Icon;

}

void ui_Power_screen_destroy(void)
{
    if(ui_Power) lv_obj_del(ui_Power);

    // NULL screen variables
    uic_Power = NULL;
    ui_Power = NULL;
    uic_Big_Power_Off_Icon = NULL;
    ui_Big_Power_Off_Icon = NULL;

}
