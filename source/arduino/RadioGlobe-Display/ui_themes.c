// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "ui.h"


const ui_theme_variable_t _ui_theme_color_turquoise[1] = {0x14EEB3};
const ui_theme_variable_t _ui_theme_alpha_turquoise[1] = {255};

const ui_theme_variable_t _ui_theme_color_lightblue[1] = {0x50C0FF};
const ui_theme_variable_t _ui_theme_alpha_lightblue[1] = {255};

const ui_theme_variable_t _ui_theme_color_yellow[1] = {0xFFFF00};
const ui_theme_variable_t _ui_theme_alpha_yellow[1] = {255};

const ui_theme_variable_t _ui_theme_color_green[1] = {0x00FF00};
const ui_theme_variable_t _ui_theme_alpha_green[1] = {255};

const ui_theme_variable_t _ui_theme_color_red[1] = {0xFF0000};
const ui_theme_variable_t _ui_theme_alpha_red[1] = {255};
uint8_t ui_theme_idx = UI_THEME_DEFAULT;


void ui_theme_set(uint8_t theme_idx)
{
    ui_theme_idx = theme_idx;
    _ui_theme_set_variable_styles(UI_VARIABLE_STYLES_MODE_FOLLOW);
}

