SET(SOURCES ui_Home.c
    ui_SetupScreen.c
    ui_CalibrationScreen.c
    ui_DatabaseScreen.c
    ui_PasswordScreen.c
    ui_StationSelectScreen.c
    ui_Favorites.c
    ui_ToneControlScreen.c
    ui_CalibrationScreenAdvanced.c
    ui_Power.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_xx_png.c
    ui_img_arrow_left_bold_32x32_png.c
    ui_img_arrow_right_bold_32x32_png.c
    ui_img_cog_png.c
    ui_img_batterylow_png.c
    ui_img_batterymedium_png.c
    ui_img_batteryhigh_png.c
    ui_img_power75x75_png.c
    ui_img_home_png.c
    ui_img_worldmap_bw_360x180_png.c
    ui_img_circle16x16_png.c
    ui_img_triangle_down_16x16_png.c
    ui_img_power256x256_png.c
    fonts/ui_font_font1.c)

add_library(ui ${SOURCES})
