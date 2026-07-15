// added by Frank
// Squareline maxes out at 150 widgets
// so have to add more manually

#ifndef UI_ADDITIONAL_WIDGETS_H
#define UI_ADDITIONAL_WIDGETS_H

#ifdef __cplusplus
extern "C" {
#endif

// SCREEN: various
extern void ui_additional_widgets_init(void);
extern void ui_additional_text_init(void);

extern void FuzzinessButtonL(lv_event_t * e);
extern void FuzzinessButtonR(lv_event_t * e);
extern void PortalButton(lv_event_t * e);
extern void GlobePlaySD(lv_event_t * e);



extern const lv_img_dsc_t icon_musiclib75x75_png;

extern lv_obj_t * ui_Open_Portal_Text;

extern void ui_event_Portal_Button(lv_event_t * e);
extern lv_obj_t * ui_Portal_Button;
extern lv_obj_t * ui_CalibrationInstruction2;
extern lv_obj_t * ui_CalibrationInstruction;
extern lv_obj_t * ui_Text_Volume;
extern lv_obj_t * ui_ToneControlInstruction;
extern lv_obj_t * ui_TextSecretCodeToUnlock;
extern lv_obj_t * ui_Text_Bass;
extern void ui_event_SetFuzzinessButtonL(lv_event_t * e);
extern lv_obj_t * ui_SetFuzzinessButtonL;
extern lv_obj_t * ui_SetFuzzinessButtonTextL;
extern void ui_event_SetFuzzinessButtonR(lv_event_t * e);
extern lv_obj_t * ui_SetFuzzinessButtonR;
extern lv_obj_t * ui_SetFuzzinessButtonTextR;
extern lv_obj_t * ui_SetFuzzinessValue;
extern lv_obj_t * ui_mainscreen_speakeron;
extern lv_obj_t * ui_mainscreen_speakeroff;
//extern lv_obj_t * ui_MusicLibraryButton;
extern lv_obj_t * ui_PresetFlag;
extern void ui_event_PresetFlag(lv_event_t * e);

extern lv_obj_t * ui_GlobeSDText;
extern lv_obj_t * ui_GlobeSDSizeText;
extern lv_obj_t * ui_BatteryText;
extern lv_obj_t * ui_BatteryLevel;

extern lv_obj_t * ui_BatteryVoltage;








// CUSTOM VARIABLES
extern lv_obj_t * uic_Portal_Button;
extern lv_obj_t * uic_Open_Portal_Text;
extern lv_obj_t * uic_CalibrationInstruction2;
extern lv_obj_t * uic_CalibrationInstruction;
extern lv_obj_t * uic_Text_Volume;
extern lv_obj_t * uic_ToneControlInstruction;
extern lv_obj_t * uic_TextSecretCodeToUnlock;
extern lv_obj_t * uic_SetFuzzinessButtonL;
extern lv_obj_t * uic_SetFuzzinessButtonTextL;
extern lv_obj_t * uic_SetFuzzinessButtonR;
extern lv_obj_t * uic_SetFuzzinessButtonTextR;
extern lv_obj_t * uic_SetFuzzinessValue;
extern lv_obj_t * uic_mainscreen_speakeron;
extern lv_obj_t * uic_mainscreen_speakeroff;
extern lv_obj_t * uic_MusicLibraryButton;
extern lv_obj_t * uic_PresetFlag;

extern lv_obj_t * uic_GlobeSDText;
extern lv_obj_t * uic_GlobeSDSizeText;
extern lv_obj_t * uic_BatteryText;
extern lv_obj_t * uic_BatteryLevel;


extern lv_obj_t * uic_BatteryVoltage;










#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
