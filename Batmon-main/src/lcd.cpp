#include <Arduino.h>
#include <TFT_eSPI.h>
#include "lcd.h"


TFT_eSPI tft = TFT_eSPI();  // Create object "tft"


void drawScreen_State(bool* old_state, bool* state) {
    if (*old_state != *state) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(2);
        tft.drawString(String(*old_state), 85, 80);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*state), 85, 80);
        
        *old_state = *state;
    }
}


void drawScreen_Voltage(float* old_voltage, float* voltage_current) {
    if (int(*old_voltage * 100) != int(*voltage_current * 100)) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(2);
        tft.drawString(String(*old_voltage), 15, 40);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*voltage_current), 15, 40);
        
        *old_voltage = *voltage_current;
    }
} 

void drawScreen_Disconnect(float* old_voltage_disconnect, float* voltage_disconnect) {
    if (*old_voltage_disconnect != *voltage_disconnect) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(2);
        tft.drawString(String(*old_voltage_disconnect), 85, 40);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*voltage_disconnect), 85, 40);
        
        *old_voltage_disconnect = *voltage_disconnect;
    }
}

void drawScreen_Timer(int* old_time, int* timer_min, int* old_time_s, int* timer_seconds) {
    if (*old_time != *timer_min) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(2);
        tft.drawString(String(*old_time), 15, 80);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*timer_min), 15, 80);
        
        *old_time = *timer_min;
    }

    if (*old_time_s != *timer_seconds) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(1);
        tft.drawString(String(*old_time_s), 50, 80);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*timer_seconds), 50, 80);
        
        *old_time_s = *timer_seconds;
    }
}

void draw_lcd_selector(bool* selector_on, int lcd_selected) {
    *selector_on = !*selector_on;
    if(lcd_selected == 0) // disconnect
    {
        highlight_timer_off();
        highlight_state_off();
        if(*selector_on==true)
        {
            highlight_disconnect(); 
        }
        else
        {
            highlight_disconnect_off();
        }
    }
    else if(lcd_selected == 1) // timer
    {
        highlight_disconnect_off();
        highlight_state_off();
        if(*selector_on==true)
        {
            highlight_timer(); 
        }
        else
        {
            highlight_timer_off();
        }
    }
    else if(lcd_selected == 2) // state
    {
        highlight_timer_off();
        highlight_disconnect_off();
        if(*selector_on==true)
        {
            highlight_state(); 
        }
        else
        {
            highlight_state_off();
        }
    }
}

void highlight_state()
{
    tft.drawLine(175, 165, 305, 165, TFT_RED);
}
void highlight_state_off()
{
    tft.drawLine(175, 165, 305, 165, TFT_BLACK);
}

void highlight_disconnect()
{
    tft.drawLine(175, 80, 305, 80, TFT_RED);
}
void highlight_disconnect_off()
{
    tft.drawLine(175, 80, 305, 80, TFT_BLACK);
}

void highlight_timer()
{
    tft.drawLine(15, 165, 145, 165, TFT_RED);
}
void highlight_timer_off()
{
    tft.drawLine(15, 165, 145, 165, TFT_BLACK);
}


void draw_static_display()
{
        // ---------- upper left ----------
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // Set text datum to top-left (anchor point)
    tft.setTextFont(2);
    tft.drawString("Current", 15, 65);
    tft.drawString("Disconnect", 175, 65);
    tft.drawString("Timer", 15, 145);
    tft.drawString("State", 175, 145);
}

void lcd_init() {
  tft.init();
  tft.setRotation(1); // Set the rotation of the display (0-3)
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color
  draw_static_display();
}