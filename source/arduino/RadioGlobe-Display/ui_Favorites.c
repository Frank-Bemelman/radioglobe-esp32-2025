// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "ui.h"

lv_obj_t * uic_led4;
lv_obj_t * uic_led3;
lv_obj_t * uic_led2;
lv_obj_t * uic_led1;
lv_obj_t * uic_StationPresetName4;
lv_obj_t * uic_StationPresetName3;
lv_obj_t * uic_StationPresetName2;
lv_obj_t * uic_StationPresetName1;
lv_obj_t * uic_StationPreset4;
lv_obj_t * uic_StationPreset3;
lv_obj_t * uic_StationPreset2;
lv_obj_t * uic_StationPreset1;
lv_obj_t * uic_IconStorage;
lv_obj_t * ui_Favorites = NULL;
lv_obj_t * ui_StationPreset1 = NULL;
lv_obj_t * ui_StationPreset2 = NULL;
lv_obj_t * ui_StationPreset3 = NULL;
lv_obj_t * ui_StationPreset4 = NULL;
lv_obj_t * ui_HomeButton6 = NULL;
lv_obj_t * ui_StationPresetName1 = NULL;
lv_obj_t * ui_StationPresetName2 = NULL;
lv_obj_t * ui_StationPresetName3 = NULL;
lv_obj_t * ui_StationPresetName4 = NULL;
lv_obj_t * ui_Label2 = NULL;
lv_obj_t * ui_Label3 = NULL;
lv_obj_t * ui_led1 = NULL;
lv_obj_t * ui_led2 = NULL;
lv_obj_t * ui_led3 = NULL;
lv_obj_t * ui_led4 = NULL;
// event funtions
void ui_event_StationPreset1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
        station1(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        station1(e);
    }
}

void ui_event_StationPreset2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
        station2(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        station2(e);
    }
}

void ui_event_StationPreset3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
        station3(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        station3(e);
    }
}

void ui_event_StationPreset4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
        station4(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        station4(e);
    }
}

void ui_event_HomeButton6(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_NONE, 500, 0, &ui_Home_screen_init);
        beep(e);
    }
}

// build funtions

void ui_Favorites_screen_init(void)
{
    ui_Favorites = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Favorites, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_StationPreset1 = lv_btn_create(ui_Favorites);
    lv_obj_set_width(ui_StationPreset1, 200);
    lv_obj_set_height(ui_StationPreset1, 50);
    lv_obj_set_x(ui_StationPreset1, 0);
    lv_obj_set_y(ui_StationPreset1, -105);
    lv_obj_set_align(ui_StationPreset1, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_StationPreset1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_StationPreset1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_StationPreset1, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_turquoise);
    ui_object_set_themeable_style_property(ui_StationPreset1, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_turquoise);

    ui_StationPreset2 = lv_btn_create(ui_Favorites);
    lv_obj_set_width(ui_StationPreset2, 200);
    lv_obj_set_height(ui_StationPreset2, 50);
    lv_obj_set_x(ui_StationPreset2, 0);
    lv_obj_set_y(ui_StationPreset2, -35);
    lv_obj_set_align(ui_StationPreset2, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_StationPreset2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_StationPreset2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_StationPreset2, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_turquoise);
    ui_object_set_themeable_style_property(ui_StationPreset2, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_turquoise);

    ui_StationPreset3 = lv_btn_create(ui_Favorites);
    lv_obj_set_width(ui_StationPreset3, 200);
    lv_obj_set_height(ui_StationPreset3, 50);
    lv_obj_set_x(ui_StationPreset3, 0);
    lv_obj_set_y(ui_StationPreset3, 35);
    lv_obj_set_align(ui_StationPreset3, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_StationPreset3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_StationPreset3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_StationPreset3, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_turquoise);
    ui_object_set_themeable_style_property(ui_StationPreset3, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_turquoise);

    ui_StationPreset4 = lv_btn_create(ui_Favorites);
    lv_obj_set_width(ui_StationPreset4, 200);
    lv_obj_set_height(ui_StationPreset4, 50);
    lv_obj_set_x(ui_StationPreset4, 0);
    lv_obj_set_y(ui_StationPreset4, 105);
    lv_obj_set_align(ui_StationPreset4, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_StationPreset4, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_StationPreset4, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_StationPreset4, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_turquoise);
    ui_object_set_themeable_style_property(ui_StationPreset4, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_turquoise);

    ui_HomeButton6 = lv_img_create(ui_Favorites);
    lv_img_set_src(ui_HomeButton6, &ui_img_home_png);
    lv_obj_set_width(ui_HomeButton6, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_HomeButton6, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_HomeButton6, 0);
    lv_obj_set_y(ui_HomeButton6, 195);
    lv_obj_set_align(ui_HomeButton6, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_HomeButton6, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_HomeButton6, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_HomeButton6, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_turquoise);
    ui_object_set_themeable_style_property(ui_HomeButton6, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_turquoise);

    ui_StationPresetName1 = lv_label_create(ui_Favorites);
    lv_obj_set_width(ui_StationPresetName1, 170);
    lv_obj_set_height(ui_StationPresetName1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_StationPresetName1, 0);
    lv_obj_set_y(ui_StationPresetName1, -105);
    lv_obj_set_align(ui_StationPresetName1, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_StationPresetName1, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(ui_StationPresetName1, "FAVORITE STATION 1");
    lv_obj_set_style_text_color(ui_StationPresetName1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_StationPresetName1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_StationPresetName1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StationPresetName1, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_StationPresetName2 = lv_label_create(ui_Favorites);
    lv_obj_set_width(ui_StationPresetName2, 170);
    lv_obj_set_height(ui_StationPresetName2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_StationPresetName2, 0);
    lv_obj_set_y(ui_StationPresetName2, -35);
    lv_obj_set_align(ui_StationPresetName2, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_StationPresetName2, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(ui_StationPresetName2, "FAVORITE STATION 2");
    lv_obj_set_style_text_color(ui_StationPresetName2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_StationPresetName2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_StationPresetName2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StationPresetName2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_StationPresetName3 = lv_label_create(ui_Favorites);
    lv_obj_set_width(ui_StationPresetName3, 170);
    lv_obj_set_height(ui_StationPresetName3, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_StationPresetName3, 0);
    lv_obj_set_y(ui_StationPresetName3, 35);
    lv_obj_set_align(ui_StationPresetName3, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_StationPresetName3, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(ui_StationPresetName3, "FAVORITE STATION 3");
    lv_obj_set_style_text_color(ui_StationPresetName3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_StationPresetName3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_StationPresetName3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StationPresetName3, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_StationPresetName4 = lv_label_create(ui_Favorites);
    lv_obj_set_width(ui_StationPresetName4, 170);
    lv_obj_set_height(ui_StationPresetName4, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_StationPresetName4, 0);
    lv_obj_set_y(ui_StationPresetName4, 105);
    lv_obj_set_align(ui_StationPresetName4, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_StationPresetName4, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(ui_StationPresetName4, "FAVORITE STATION 4");
    lv_obj_set_style_text_color(ui_StationPresetName4, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_StationPresetName4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_StationPresetName4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StationPresetName4, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Label2 = lv_label_create(ui_Favorites);
    lv_obj_set_width(ui_Label2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Label2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Label2, 1);
    lv_obj_set_y(ui_Label2, -190);
    lv_obj_set_align(ui_Label2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label2, "MY FAVORITES ");
    lv_obj_set_style_text_font(ui_Label2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Label3 = lv_label_create(ui_Favorites);
    lv_obj_set_width(ui_Label3, 243);
    lv_obj_set_height(ui_Label3, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Label3, 0);
    lv_obj_set_y(ui_Label3, -160);
    lv_obj_set_align(ui_Label3, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_Label3, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(ui_Label3, "Long Press To Store Current Station As Favorite  ---");
    lv_obj_set_style_text_font(ui_Label3, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_led1 = lv_img_create(ui_Favorites);
    lv_img_set_src(ui_led1, &ui_img_circle16x16_png);
    lv_obj_set_width(ui_led1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_led1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_led1, 120);
    lv_obj_set_y(ui_led1, -105);
    lv_obj_set_align(ui_led1, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_led1, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_led1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    ui_object_set_themeable_style_property(ui_led1, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_red);
    ui_object_set_themeable_style_property(ui_led1, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_red);

    ui_led2 = lv_img_create(ui_Favorites);
    lv_img_set_src(ui_led2, &ui_img_circle16x16_png);
    lv_obj_set_width(ui_led2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_led2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_led2, 120);
    lv_obj_set_y(ui_led2, -35);
    lv_obj_set_align(ui_led2, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_led2, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_led2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    ui_object_set_themeable_style_property(ui_led2, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_green);
    ui_object_set_themeable_style_property(ui_led2, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_green);

    ui_led3 = lv_img_create(ui_Favorites);
    lv_img_set_src(ui_led3, &ui_img_circle16x16_png);
    lv_obj_set_width(ui_led3, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_led3, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_led3, 120);
    lv_obj_set_y(ui_led3, 35);
    lv_obj_set_align(ui_led3, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_led3, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_led3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    ui_object_set_themeable_style_property(ui_led3, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_red);
    ui_object_set_themeable_style_property(ui_led3, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_red);

    ui_led4 = lv_img_create(ui_Favorites);
    lv_img_set_src(ui_led4, &ui_img_circle16x16_png);
    lv_obj_set_width(ui_led4, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_led4, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_led4, 120);
    lv_obj_set_y(ui_led4, 105);
    lv_obj_set_align(ui_led4, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_led4, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_led4, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    ui_object_set_themeable_style_property(ui_led4, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_green);
    ui_object_set_themeable_style_property(ui_led4, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_green);

    lv_obj_add_event_cb(ui_StationPreset1, ui_event_StationPreset1, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_StationPreset2, ui_event_StationPreset2, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_StationPreset3, ui_event_StationPreset3, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_StationPreset4, ui_event_StationPreset4, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_HomeButton6, ui_event_HomeButton6, LV_EVENT_ALL, NULL);
    uic_IconStorage = ui_Favorites;
    uic_StationPreset1 = ui_StationPreset1;
    uic_StationPreset2 = ui_StationPreset2;
    uic_StationPreset3 = ui_StationPreset3;
    uic_StationPreset4 = ui_StationPreset4;
    uic_StationPresetName1 = ui_StationPresetName1;
    uic_StationPresetName2 = ui_StationPresetName2;
    uic_StationPresetName3 = ui_StationPresetName3;
    uic_StationPresetName4 = ui_StationPresetName4;
    uic_led1 = ui_led1;
    uic_led2 = ui_led2;
    uic_led3 = ui_led3;
    uic_led4 = ui_led4;

}

void ui_Favorites_screen_destroy(void)
{
    if(ui_Favorites) lv_obj_del(ui_Favorites);

    // NULL screen variables
    uic_IconStorage = NULL;
    ui_Favorites = NULL;
    uic_StationPreset1 = NULL;
    ui_StationPreset1 = NULL;
    uic_StationPreset2 = NULL;
    ui_StationPreset2 = NULL;
    uic_StationPreset3 = NULL;
    ui_StationPreset3 = NULL;
    uic_StationPreset4 = NULL;
    ui_StationPreset4 = NULL;
    ui_HomeButton6 = NULL;
    uic_StationPresetName1 = NULL;
    ui_StationPresetName1 = NULL;
    uic_StationPresetName2 = NULL;
    ui_StationPresetName2 = NULL;
    uic_StationPresetName3 = NULL;
    ui_StationPresetName3 = NULL;
    uic_StationPresetName4 = NULL;
    ui_StationPresetName4 = NULL;
    ui_Label2 = NULL;
    ui_Label3 = NULL;
    uic_led1 = NULL;
    ui_led1 = NULL;
    uic_led2 = NULL;
    ui_led2 = NULL;
    uic_led3 = NULL;
    ui_led3 = NULL;
    uic_led4 = NULL;
    ui_led4 = NULL;

}
