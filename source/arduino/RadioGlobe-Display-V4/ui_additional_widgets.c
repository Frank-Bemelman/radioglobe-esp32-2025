// added by Frank
// Squareline maxes out at 150 widgets
// so have to add more manually

#include "ui.h"
#include "ui_additional_widgets.h"

lv_obj_t * uic_Open_Portal_Text;
lv_obj_t * uic_Portal_Button;
lv_obj_t * uic_CalibrationInstruction2;
lv_obj_t * uic_CalibrationInstruction;
lv_obj_t * uic_Text_Volume;
lv_obj_t * uic_ToneControlInstruction;
lv_obj_t * uic_TextSecretCodeToUnlock;
lv_obj_t * uic_Text_Bass;
lv_obj_t * uic_SetFuzzinessButtonTextL;
lv_obj_t * uic_SetFuzzinessButtonL;
lv_obj_t * uic_SetFuzzinessButtonTextR;
lv_obj_t * uic_SetFuzzinessButtonR;
lv_obj_t * uic_SetFuzzinessValue;
lv_obj_t * uic_mainscreen_speakeron;
lv_obj_t * uic_mainscreen_speakeroff;
lv_obj_t * uic_MusicLibraryButton;
lv_obj_t * uic_PresetFlag1 = NULL;
lv_obj_t * uic_PresetFlag2 = NULL;
lv_obj_t * uic_PresetFlag3 = NULL;
lv_obj_t * uic_PresetFlag4 = NULL;

lv_obj_t * uic_GlobeChannel = NULL;
lv_obj_t * uic_GlobeSDText  = NULL;
lv_obj_t * uic_GlobeSDSizeText  = NULL;

lv_obj_t * uic_BatteryText  = NULL;
lv_obj_t * uic_BatteryLevel  = NULL;

lv_obj_t * uic_BatteryLevelHome = NULL;

lv_obj_t * uic_PresetFlag = NULL;







lv_obj_t * ui_Open_Portal_Text = NULL;
lv_obj_t * ui_Portal_Button = NULL; 
lv_obj_t * ui_CalibrationInstruction2 = NULL;
lv_obj_t * ui_CalibrationInstruction = NULL;
lv_obj_t * ui_Text_Volume = NULL;
lv_obj_t * ui_ToneControlInstruction = NULL;
lv_obj_t * ui_TextSecretCodeToUnlock = NULL;
lv_obj_t * ui_Text_Bass = NULL;
lv_obj_t * ui_SetFuzzinessButtonTextL = NULL;
lv_obj_t * ui_SetFuzzinessButtonL = NULL;
lv_obj_t * ui_SetFuzzinessButtonTextR = NULL;
lv_obj_t * ui_SetFuzzinessButtonR = NULL;
lv_obj_t * ui_SetFuzzinessValue = NULL;
lv_obj_t * ui_mainscreen_speakeron = NULL;
lv_obj_t * ui_mainscreen_speakeroff = NULL;
//lv_obj_t * ui_MusicLibraryButton = NULL;

lv_obj_t * ui_PresetFlag1 = NULL;
lv_obj_t * ui_PresetFlag2 = NULL;
lv_obj_t * ui_PresetFlag3 = NULL;
lv_obj_t * ui_PresetFlag4 = NULL;

lv_obj_t * ui_GlobeChannel = NULL;
lv_obj_t * ui_GlobeSDText  = NULL;
lv_obj_t * ui_GlobeSDSizeText  = NULL;

lv_obj_t * ui_BatteryText  = NULL;
lv_obj_t * ui_BatteryLevel  = NULL;


lv_obj_t * ui_BatteryLevelHome = NULL;

lv_obj_t * ui_PresetFlag = NULL;


void ui_event_PresetFlag(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
      
        //station1(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        //station1(e);
    }
}


void ui_event_Portal_Button(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) {
        PortalButton(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        PortalButton(e);
        FuzzinessButtonR(e);
    //    beep(e);
    }
}



void ui_event_SetFuzzinessButtonL(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { FuzzinessButtonL(e);
  }
  if(event_code == LV_EVENT_CLICKED) 
  { FuzzinessButtonL(e);
  }
}



void ui_event_SetFuzzinessButtonR(lv_event_t * e)
{ lv_event_code_t event_code = lv_event_get_code(e);

  if(event_code == LV_EVENT_LONG_PRESSED_REPEAT) 
  { FuzzinessButtonR(e);
  }
  if(event_code == LV_EVENT_CLICKED) 
  { FuzzinessButtonR(e);
  }
}

//void ui_event_MusicLibraryButton(lv_event_t * e)
//{
//    lv_event_code_t event_code = lv_event_get_code(e);
//
//    if(event_code == LV_EVENT_CLICKED) {
//        GlobePlaySD(e);
//        beep(e);
//    }
//}

// not used anymore
void ui_additional_widgets_init(void)
{   ui_Open_Portal_Text = lv_label_create(ui_SetupScreen);
    lv_obj_set_width(ui_Open_Portal_Text, 140);
    lv_obj_set_height(ui_Open_Portal_Text, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Open_Portal_Text, -160);
    lv_obj_set_y(ui_Open_Portal_Text, 46);
    lv_obj_set_align(ui_Open_Portal_Text, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_Open_Portal_Text, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(ui_Open_Portal_Text, "PORTAL");
    lv_obj_set_style_text_color(ui_Open_Portal_Text, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Open_Portal_Text, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Open_Portal_Text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Open_Portal_Text, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Portal_Button = lv_img_create(ui_SetupScreen); 
    lv_img_set_src(ui_Portal_Button, &ui_img_cog_png);
    lv_obj_set_width(ui_Portal_Button, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Portal_Button, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Portal_Button, -160);
    lv_obj_set_y(ui_Portal_Button, 0);
    lv_obj_set_align(ui_Portal_Button, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Portal_Button, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_Portal_Button, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_Portal_Button, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                           _ui_theme_color_turquoise);
    ui_object_set_themeable_style_property(ui_Portal_Button, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                           _ui_theme_alpha_turquoise);

    lv_obj_add_event_cb(ui_Portal_Button, ui_event_Portal_Button, LV_EVENT_ALL, NULL);   // in calibration.ino                                         
    uic_Open_Portal_Text = ui_Open_Portal_Text;
    uic_Portal_Button = ui_Portal_Button;

    


}

void ui_additional_text_init(void)
{ ui_CalibrationInstruction2 = lv_label_create(ui_SetupScreen);
  lv_obj_set_width(ui_CalibrationInstruction2, LV_SIZE_CONTENT);   /// 327
  lv_obj_set_height(ui_CalibrationInstruction2, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_CalibrationInstruction2, 0);
  lv_obj_set_y(ui_CalibrationInstruction2, 50);
  lv_obj_set_align(ui_CalibrationInstruction2, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_CalibrationInstruction2, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_CalibrationInstruction2, "Long Press Button To Select");
  lv_obj_set_style_text_font(ui_CalibrationInstruction2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_CalibrationInstruction2 = ui_CalibrationInstruction2;

  ui_CalibrationInstruction = lv_label_create(ui_CalibrationScreen);
  lv_obj_set_width(ui_CalibrationInstruction, LV_SIZE_CONTENT);   /// 327
  lv_obj_set_height(ui_CalibrationInstruction, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_CalibrationInstruction, 0);
  lv_obj_set_y(ui_CalibrationInstruction, 150);
  lv_obj_set_align(ui_CalibrationInstruction, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_CalibrationInstruction, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_CalibrationInstruction, "Long Press Button To Select");
  lv_obj_set_style_text_font(ui_CalibrationInstruction, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_CalibrationInstruction2 = ui_CalibrationInstruction;

  
  ui_Text_Volume = lv_label_create(ui_Home);
  lv_obj_set_width(ui_Text_Volume, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Text_Volume, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_Text_Volume, 0);
  lv_obj_set_y(ui_Text_Volume, 196);
  lv_obj_set_align(ui_Text_Volume, LV_ALIGN_CENTER);
  //lv_label_set_text(ui_Text_Volume, "VOLUME");
  lv_label_set_text(ui_Text_Volume, "");
  lv_obj_set_style_text_color(ui_Text_Volume, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Text_Volume, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Text_Volume, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_Text_Volume = ui_Text_Volume;

  ui_ToneControlInstruction = lv_label_create(ui_ToneControlScreen);
  lv_obj_set_width(ui_ToneControlInstruction, LV_SIZE_CONTENT);   /// 327
  lv_obj_set_height(ui_ToneControlInstruction, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_ToneControlInstruction, 0);
  lv_obj_set_y(ui_ToneControlInstruction, -125);
  lv_obj_set_align(ui_ToneControlInstruction, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_ToneControlInstruction, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_ToneControlInstruction, "Long Press Button To Save");
  lv_obj_set_style_text_font(ui_ToneControlInstruction, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_ToneControlInstruction = ui_ToneControlInstruction;

  ui_TextSecretCodeToUnlock = lv_label_create(ui_PasswordScreen);
  lv_obj_set_width(ui_TextSecretCodeToUnlock, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_TextSecretCodeToUnlock, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_TextSecretCodeToUnlock, 0);
  lv_obj_set_y(ui_TextSecretCodeToUnlock, -130);
  lv_obj_set_align(ui_TextSecretCodeToUnlock, LV_ALIGN_CENTER);
  lv_label_set_text(ui_TextSecretCodeToUnlock, "SECRET CODE TO UNLOCK");
  lv_obj_set_style_text_align(ui_TextSecretCodeToUnlock, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_TextSecretCodeToUnlock, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_TextSecretCodeToUnlock = ui_TextSecretCodeToUnlock;

  ui_Text_Bass = lv_label_create(ui_ToneControlScreen);
  lv_obj_set_width(ui_Text_Bass, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Text_Bass, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_Text_Bass, -120);
  lv_obj_set_y(ui_Text_Bass, 25);
  lv_obj_set_align(ui_Text_Bass, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Text_Bass, "BASS");
  lv_obj_set_style_text_color(ui_Text_Bass, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Text_Bass, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Text_Bass, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_Text_Bass = ui_Text_Bass;


  ui_SetFuzzinessButtonL = lv_btn_create(ui_SetupScreen);
  lv_obj_set_width(ui_SetFuzzinessButtonL, 50);
  lv_obj_set_height(ui_SetFuzzinessButtonL, 50);
  lv_obj_set_x(ui_SetFuzzinessButtonL, -110);
  lv_obj_set_y(ui_SetFuzzinessButtonL, 110);
  lv_obj_set_align(ui_SetFuzzinessButtonL, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SetFuzzinessButtonL, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
  lv_obj_clear_flag(ui_SetFuzzinessButtonL, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonL, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                         _ui_theme_color_turquoise);
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonL, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                         _ui_theme_alpha_turquoise);
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonL, LV_PART_MAIN | LV_STATE_PRESSED, LV_STYLE_SHADOW_COLOR,
                                           _ui_theme_color_yellow);
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonL, LV_PART_MAIN | LV_STATE_PRESSED, LV_STYLE_SHADOW_OPA,
                                           _ui_theme_alpha_yellow);
  lv_obj_set_style_shadow_width(ui_SetFuzzinessButtonL, 5, LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_shadow_spread(ui_SetFuzzinessButtonL, 5, LV_PART_MAIN | LV_STATE_PRESSED);


  lv_obj_add_event_cb(ui_SetFuzzinessButtonL, ui_event_SetFuzzinessButtonL, LV_EVENT_ALL, NULL);   // in calibration.ino                                         

  ui_SetFuzzinessButtonTextL = lv_label_create(ui_SetupScreen);
  lv_obj_set_width(ui_SetFuzzinessButtonTextL, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SetFuzzinessButtonTextL, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_SetFuzzinessButtonTextL, -110);
  lv_obj_set_y(ui_SetFuzzinessButtonTextL, 110-2);
  lv_obj_set_align(ui_SetFuzzinessButtonTextL, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SetFuzzinessButtonTextL, "-");
  lv_obj_set_style_text_color(ui_SetFuzzinessButtonTextL, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SetFuzzinessButtonTextL, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_SetFuzzinessButtonTextL, &lv_font_montserrat_42, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SetFuzzinessButtonR = lv_btn_create(ui_SetupScreen);
  lv_obj_set_width(ui_SetFuzzinessButtonR, 50);
  lv_obj_set_height(ui_SetFuzzinessButtonR, 50);
  lv_obj_set_x(ui_SetFuzzinessButtonR, 110);
  lv_obj_set_y(ui_SetFuzzinessButtonR, 110);
  lv_obj_set_align(ui_SetFuzzinessButtonR, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SetFuzzinessButtonR, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
  lv_obj_clear_flag(ui_SetFuzzinessButtonR, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonR, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                         _ui_theme_color_turquoise);
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonR, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                         _ui_theme_alpha_turquoise);
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonR, LV_PART_MAIN | LV_STATE_PRESSED, LV_STYLE_SHADOW_COLOR,
                                           _ui_theme_color_yellow);
  ui_object_set_themeable_style_property(ui_SetFuzzinessButtonR, LV_PART_MAIN | LV_STATE_PRESSED, LV_STYLE_SHADOW_OPA,
                                           _ui_theme_alpha_yellow);
  lv_obj_set_style_shadow_width(ui_SetFuzzinessButtonR, 5, LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_shadow_spread(ui_SetFuzzinessButtonR, 5, LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_add_event_cb(ui_SetFuzzinessButtonR, ui_event_SetFuzzinessButtonR, LV_EVENT_ALL, NULL);   // in calibration.ino                                         

  ui_SetFuzzinessButtonTextR = lv_label_create(ui_SetupScreen);
  lv_obj_set_width(ui_SetFuzzinessButtonTextR, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SetFuzzinessButtonTextR, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_SetFuzzinessButtonTextR, 110);
  lv_obj_set_y(ui_SetFuzzinessButtonTextR, 110);
  lv_obj_set_align(ui_SetFuzzinessButtonTextR, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SetFuzzinessButtonTextR, "+");
  lv_obj_set_style_text_color(ui_SetFuzzinessButtonTextR, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SetFuzzinessButtonTextR, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_SetFuzzinessButtonTextR, &lv_font_montserrat_42, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SetFuzzinessValue = lv_label_create(ui_SetupScreen);
  lv_obj_set_width(ui_SetFuzzinessValue, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_SetFuzzinessValue, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_SetFuzzinessValue, 0);
  lv_obj_set_y(ui_SetFuzzinessValue, 110);
  lv_obj_set_align(ui_SetFuzzinessValue, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_SetFuzzinessValue, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_SetFuzzinessValue, "FUZZINESS X");
  lv_obj_set_style_text_align(ui_SetFuzzinessValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_SetFuzzinessValue, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);


  uic_SetFuzzinessButtonL = ui_SetFuzzinessButtonL;
  uic_SetFuzzinessButtonTextL = ui_SetFuzzinessButtonTextL;
  uic_SetFuzzinessButtonR = ui_SetFuzzinessButtonR;
  uic_SetFuzzinessButtonTextR = ui_SetFuzzinessButtonTextR;
  uic_SetFuzzinessValue = ui_SetFuzzinessValue;

  ui_mainscreen_speakeron = lv_img_create(ui_Home);
  lv_img_set_src(ui_mainscreen_speakeron, &ui_img_volume_up_48x48_png);
  lv_obj_set_width(ui_mainscreen_speakeron, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_mainscreen_speakeron, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_mainscreen_speakeron, 0);
  lv_obj_set_y(ui_mainscreen_speakeron, 160);
  lv_obj_set_align(ui_mainscreen_speakeron, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_mainscreen_speakeron, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
  lv_obj_clear_flag(ui_mainscreen_speakeron, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
  ui_object_set_themeable_style_property(ui_mainscreen_speakeron, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
                                         _ui_theme_color_turquoise);
  ui_object_set_themeable_style_property(ui_mainscreen_speakeron, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
                                         _ui_theme_alpha_turquoise);

  ui_mainscreen_speakeroff = lv_img_create(ui_Home);
  lv_img_set_src(ui_mainscreen_speakeroff, &ui_img_volume_off_48x48_png);
  lv_obj_set_width(ui_mainscreen_speakeroff, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_mainscreen_speakeroff, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_mainscreen_speakeroff, 0);
  lv_obj_set_y(ui_mainscreen_speakeroff, 160);
  lv_obj_set_align(ui_mainscreen_speakeroff, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_mainscreen_speakeroff, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
  lv_obj_clear_flag(ui_mainscreen_speakeroff, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
  lv_obj_set_style_img_recolor(ui_mainscreen_speakeroff, lv_color_hex(0x7F7F7F), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_img_recolor_opa(ui_mainscreen_speakeroff, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
 
  uic_mainscreen_speakeron = ui_mainscreen_speakeron;
  uic_mainscreen_speakeroff = ui_mainscreen_speakeroff;

//  ui_MusicLibraryButton = lv_img_create(ui_Favorites);
//  lv_img_set_src(ui_MusicLibraryButton, &icon_musiclib75x75_png);
//  lv_obj_set_width(ui_MusicLibraryButton, LV_SIZE_CONTENT);   /// 1
//  lv_obj_set_height(ui_MusicLibraryButton, LV_SIZE_CONTENT);    /// 1
//  lv_obj_set_x(ui_MusicLibraryButton, 170);
//  lv_obj_set_y(ui_MusicLibraryButton, 0);
//  lv_obj_set_align(ui_MusicLibraryButton, LV_ALIGN_CENTER);
//  lv_obj_add_flag(ui_MusicLibraryButton, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
//  lv_obj_clear_flag(ui_MusicLibraryButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
//  ui_object_set_themeable_style_property(ui_MusicLibraryButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR,
//                                         _ui_theme_color_turquoise);
//  ui_object_set_themeable_style_property(ui_MusicLibraryButton, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_IMG_RECOLOR_OPA,
//                                         _ui_theme_alpha_turquoise);
//  lv_obj_add_event_cb(ui_MusicLibraryButton, ui_event_MusicLibraryButton, LV_EVENT_ALL, NULL);   // in calibration.ino                                         
//
//  uic_MusicLibraryButton = ui_MusicLibraryButton;


  ui_PresetFlag = lv_img_create(ui_Favorites);
  lv_img_set_src(ui_PresetFlag, &ui_img_xx_png);
  lv_obj_set_width(ui_PresetFlag, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_PresetFlag, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_PresetFlag, 155);
  lv_obj_set_y(ui_PresetFlag, -104);
  lv_obj_set_align(ui_PresetFlag, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_PresetFlag, LV_OBJ_FLAG_HIDDEN);   
  lv_obj_clear_flag(ui_PresetFlag, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_img_set_zoom(ui_PresetFlag, 192);
  uic_PresetFlag = ui_PresetFlag;
  

  lv_obj_add_event_cb(ui_PresetFlag, ui_event_PresetFlag, LV_EVENT_ALL, NULL);
  
  ui_GlobeChannel = lv_label_create(ui_Favorites);
  lv_obj_set_width(ui_GlobeChannel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_GlobeChannel, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_GlobeChannel, -165);
  lv_obj_set_y(ui_GlobeChannel, -130);
  lv_obj_set_align(ui_GlobeChannel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_GlobeChannel, "Globe");
  lv_obj_set_style_text_color(ui_GlobeChannel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_GlobeChannel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_GlobeChannel = ui_GlobeChannel;


  ui_GlobeSDText = lv_label_create(ui_Favorites);
  lv_obj_set_width(ui_GlobeSDText, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_GlobeSDText, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_GlobeSDText, -165);
  lv_obj_set_y(ui_GlobeSDText, 10);
  lv_obj_set_align(ui_GlobeSDText, LV_ALIGN_CENTER);
  lv_label_set_text(ui_GlobeSDText, "Globe SD");
  lv_obj_set_style_text_color(ui_GlobeSDText, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_GlobeSDText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_GlobeSDText = ui_GlobeSDText;

  ui_GlobeSDSizeText = lv_label_create(ui_Favorites);
  lv_obj_set_width(ui_GlobeSDSizeText, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_GlobeSDSizeText, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_GlobeSDSizeText, -165);
  lv_obj_set_y(ui_GlobeSDSizeText, 60);
  lv_obj_set_align(ui_GlobeSDSizeText, LV_ALIGN_CENTER);
  lv_label_set_text(ui_GlobeSDSizeText, "XX GB");
  lv_obj_set_style_text_color(ui_GlobeSDSizeText, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_GlobeSDSizeText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_GlobeSDSizeText = ui_GlobeSDSizeText;

  ui_BatteryText = lv_label_create(ui_Favorites);
  lv_obj_set_width(ui_BatteryText, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_BatteryText, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_BatteryText, -165);
  lv_obj_set_y(ui_BatteryText, 80);
  lv_obj_set_align(ui_BatteryText, LV_ALIGN_CENTER);
  lv_label_set_text(ui_BatteryText, "Battery");
  lv_obj_set_style_text_color(ui_BatteryText, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_BatteryText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_BatteryText = ui_BatteryText;

  ui_BatteryLevel = lv_label_create(ui_Favorites);
  lv_obj_set_width(ui_BatteryLevel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_BatteryLevel, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_BatteryLevel, -165);
  lv_obj_set_y(ui_BatteryLevel, 130);
  lv_obj_set_align(ui_BatteryLevel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_BatteryLevel, "50%");
  lv_obj_set_style_text_color(ui_BatteryLevel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_BatteryLevel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_BatteryLevel = ui_BatteryLevel;


  ui_BatteryLevelHome = lv_label_create(ui_Home);
  lv_obj_set_height(ui_BatteryLevelHome, 18);
  lv_obj_set_width(ui_BatteryLevelHome, LV_SIZE_CONTENT);   /// 52
  lv_obj_set_x(ui_BatteryLevelHome, 162);
  lv_obj_set_y(ui_BatteryLevelHome, 109);
  lv_obj_set_align(ui_BatteryLevelHome, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_BatteryLevelHome, LV_LABEL_LONG_CLIP);
  lv_label_set_text(ui_BatteryLevelHome, "");
  lv_obj_clear_flag(ui_BatteryLevelHome,
                      LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
  lv_obj_set_style_text_align(ui_BatteryLevelHome, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  uic_BatteryLevelHome = ui_BatteryLevelHome;

    

}
