#pragma once 

void rotaryencoder_init();
bool getEncoderButtonToggleState();
uint32_t getEncoderValue();
void setEncoderRange(uint32_t value, uint32_t min, uint32_t max);   