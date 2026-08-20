#pragma once 

enum enumScreenMode {
  SCREEN_MODE_OVERVIEW = 0,
  SCREEN_MODE_SELECTED_VALUE = 1,
  SCREEN_MODE_VOLTAGE_GRAPH = 2
};

void rotaryencoder_init();
enum enumScreenMode getScreenMode(enum enumScreenMode currentScreenMode);
uint32_t getEncoderValue();
void setEncoderRange(uint32_t value, uint32_t min, uint32_t max);   