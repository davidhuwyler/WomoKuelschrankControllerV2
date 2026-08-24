#include <Arduino.h>
#include <TFT_eSPI.h>
#include "lcd.h"
#include "batmon.h"

TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

#define TEXT_FONT_VALUE 4
#define TEXT_FONT_LABEL 2

#define DISCONNECT_HIGHLIGHT_FROM_X 135  // Upper Right Quadrant Highlighter From Point: X
#define DISCONNECT_HIGHLIGHT_FROM_Y 67  // Upper Right Quadrant Highlighter From Point: Y
#define DISCONNECT_HIGHLIGHT_TO_X 225  // Upper Right Quadrant Highlighter To Point: X
#define DISCONNECT_HIGHLIGHT_TO_Y DISCONNECT_HIGHLIGHT_FROM_Y  // Upper Right Quadrant Highlighter To Point: Y
#define DISCONNECT_VALUE_X DISCONNECT_HIGHLIGHT_FROM_X
#define DISCONNECT_VALUE_Y 20
#define DISCONNECT_TEXT_X DISCONNECT_HIGHLIGHT_FROM_X
#define DISCONNECT_TEXT_Y 51

#define TIMER_HIGHLIGHT_FROM_X 15  // Lower Left Quadrant Highlighter From Point: X
#define TIMER_HIGHLIGHT_FROM_Y 134  // Lower Left Quadrant Highlighter From Point: Y
#define TIMER_HIGHLIGHT_TO_X 105  // Lower Left Quadrant Highlighter To Point: X
#define TIMER_HIGHLIGHT_TO_Y TIMER_HIGHLIGHT_FROM_Y  // Lower Left Quadrant Highlighter To Point: Y
#define TIMER_VALUE_X TIMER_HIGHLIGHT_FROM_X
#define TIMER_VALUE_Y 87
#define TIMER_SECONDS_VALUE_X TIMER_VALUE_X+50
#define TIMER_SECONDS_VALUE_Y TIMER_VALUE_Y

#define TIMER_TEXT_X TIMER_HIGHLIGHT_FROM_X
#define TIMER_TEXT_Y 118

#define STATE_HIGHLIGHT_FROM_X DISCONNECT_HIGHLIGHT_FROM_X  // Lower Right Quadrant Highlighter From Point: X
#define STATE_HIGHLIGHT_FROM_Y TIMER_HIGHLIGHT_FROM_Y  // Lower Right Quadrant Highlighter From Point: Y
#define STATE_HIGHLIGHT_TO_X DISCONNECT_HIGHLIGHT_TO_X  // Lower Right Quadrant Highlighter To Point: X
#define STATE_HIGHLIGHT_TO_Y TIMER_HIGHLIGHT_FROM_Y  // Lower Right Quadrant Highlighter To Point: Y
#define STATE_VALUE_X STATE_HIGHLIGHT_FROM_X
#define STATE_VALUE_Y TIMER_VALUE_Y
#define STATE_TEXT_X STATE_HIGHLIGHT_FROM_X
#define STATE_TEXT_Y TIMER_TEXT_Y

#define CURRENT_VALUE_X TIMER_VALUE_X
#define CURRENT_VALUE_Y DISCONNECT_VALUE_Y
#define CURRENT_TEXT_X TIMER_TEXT_X
#define CURRENT_TEXT_Y DISCONNECT_TEXT_Y


void lcd_init() {
  tft.init();
  draw_static_overview_display();
}


void drawScreen_State(bool* old_state, bool* state) {
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(TEXT_FONT_VALUE);
    tft.drawString(String(*old_state? "ON" : "OFF"), STATE_VALUE_X, STATE_VALUE_Y);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(*state? "ON" : "OFF" ), STATE_VALUE_X, STATE_VALUE_Y);
    
    *old_state = *state;
}


void drawScreen_Voltage(float* old_voltage, float* voltage_current) {
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(TEXT_FONT_VALUE);
    tft.drawString(String(*old_voltage), CURRENT_VALUE_X, CURRENT_VALUE_Y);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(*voltage_current), CURRENT_VALUE_X, CURRENT_VALUE_Y);
    
    *old_voltage = *voltage_current;
} 

void drawScreen_Disconnect(float* old_voltage_disconnect, float* voltage_disconnect) {
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(TEXT_FONT_VALUE);
    tft.drawString(String(*old_voltage_disconnect), DISCONNECT_VALUE_X, DISCONNECT_VALUE_Y);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(*voltage_disconnect), DISCONNECT_VALUE_X, DISCONNECT_VALUE_Y);
    
    *old_voltage_disconnect = *voltage_disconnect;
}

void drawScreen_Timer(int* old_time, int* timer_min, int* old_time_s, int* timer_seconds) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(TEXT_FONT_VALUE);
        tft.drawString(String(*old_time), TIMER_VALUE_X, TIMER_VALUE_Y);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*timer_min), TIMER_VALUE_X, TIMER_VALUE_Y);
        *old_time = *timer_min;

        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextFont(1);
        tft.drawString(String(*old_time_s), TIMER_SECONDS_VALUE_X, TIMER_SECONDS_VALUE_Y);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(*timer_seconds), TIMER_SECONDS_VALUE_X, TIMER_SECONDS_VALUE_Y);
        *old_time_s = *timer_seconds;

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
    tft.drawLine(STATE_HIGHLIGHT_FROM_X, STATE_HIGHLIGHT_FROM_Y, STATE_HIGHLIGHT_TO_X, STATE_HIGHLIGHT_TO_Y, TFT_RED);
}
void highlight_state_off()
{
    tft.drawLine(STATE_HIGHLIGHT_FROM_X, STATE_HIGHLIGHT_FROM_Y, STATE_HIGHLIGHT_TO_X, STATE_HIGHLIGHT_TO_Y, TFT_BLACK);
}

void highlight_disconnect()
{
    tft.drawLine(DISCONNECT_HIGHLIGHT_FROM_X, DISCONNECT_HIGHLIGHT_FROM_Y, DISCONNECT_HIGHLIGHT_TO_X, DISCONNECT_HIGHLIGHT_TO_Y, TFT_RED);
}
void highlight_disconnect_off()
{
    tft.drawLine(DISCONNECT_HIGHLIGHT_FROM_X, DISCONNECT_HIGHLIGHT_FROM_Y, DISCONNECT_HIGHLIGHT_TO_X, DISCONNECT_HIGHLIGHT_TO_Y, TFT_BLACK);
}

void highlight_timer()
{
    tft.drawLine(TIMER_HIGHLIGHT_FROM_X, TIMER_HIGHLIGHT_FROM_Y, TIMER_HIGHLIGHT_TO_X, TIMER_HIGHLIGHT_TO_Y, TFT_RED);
}
void highlight_timer_off()
{
    tft.drawLine(TIMER_HIGHLIGHT_FROM_X, TIMER_HIGHLIGHT_FROM_Y, TIMER_HIGHLIGHT_TO_X, TIMER_HIGHLIGHT_TO_Y, TFT_BLACK);
}


void draw_static_overview_display()
{
    tft.setRotation(1); // Set the rotation of the display (0-3)
    tft.fillScreen(TFT_BLACK); // Clear the screen with black color

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // Set text datum to top-left (anchor point)
    tft.setTextFont(TEXT_FONT_LABEL);
    tft.drawString("Voltage", CURRENT_TEXT_X, CURRENT_TEXT_Y);
    tft.drawString("Disconnect", DISCONNECT_TEXT_X, DISCONNECT_TEXT_Y);
    tft.drawString("Timer", TIMER_TEXT_X, TIMER_TEXT_Y);
    tft.drawString("State", STATE_TEXT_X, STATE_TEXT_Y);
}




//------------------------Voltage Graph-------------------------

void draw_voltage_graph_display(bool doInitScreen)
{
    static BM6Data data_point;
    static BM6Data old_data_point;
    // Pixel Range X= 225 - 15 = 210
    // Pixel Range Y= 134 - 20 = 114
    // Voltage Range = 10V - 14V
    // Temp Range = 0° - 40°
    uint16_t x_range=210;
    uint16_t y_range=114;
    uint32_t fillLevel = batmonRing_getFillLevel();
    uint32_t current_intex;
    if(fillLevel)
    {
        current_intex=fillLevel-1;
    }
    get_batmon_data(&data_point);

    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // Set text datum to top-left (anchor point)
    tft.setTextFont(1);
    tft.drawString(String(old_data_point.voltage), CURRENT_TEXT_X+50, 1);
    tft.drawString(String(old_data_point.temperature), DISCONNECT_TEXT_X+73, 1);


    if(doInitScreen)
    {
        tft.setRotation(1); // Set the rotation of the display (0-3)
        tft.fillScreen(TFT_BLACK); // Clear the screen with black color

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(TL_DATUM); // Set text datum to top-left (anchor point)
        tft.setTextFont(1);
        tft.drawString("Voltage:", CURRENT_TEXT_X, 1);
        tft.drawString("Temperature:", 120, 1);
        tft.drawString("FS: ", 120, 11);
        tft.drawString("T: ", 120+60,11);

        tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
        tft.drawString("14V", 0, 13);
        tft.drawString("12V", 0, 70);
        tft.drawString("10V", 0, 120);

        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("40", 0, 21);     
        tft.drawString("20", 0, 78);    
        tft.drawString("0" , 0, 128);

        tft.drawLine(15, 134, 225, 134, TFT_WHITE);
        tft.drawLine(15, 134, 15, 20, TFT_WHITE);
    }

    if (current_intex == 0) {
        return;
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // Set text datum to top-left (anchor point)
    tft.setTextFont(1);
    tft.drawString(String(data_point.voltage), CURRENT_TEXT_X+50, 1);
    tft.drawString(String(data_point.temperature), 120+73, 1);
    tft.drawString(String(fillLevel*5), 120+73, 11);

    if(fillLevel < x_range)
    {
        fillLevel=x_range;
    } 

    tft.drawString(String(fillLevel*5), 120+17, 11);

    float x_delta=((float)fillLevel)/x_range;
    // Serial.println("fillLevel: "+String(fillLevel));
    // Serial.println("x_delta: "+String(x_delta));

    for(int i=0; i<x_range; i++)
    {
        BM6Data* data;
        uint16_t x_pos=15+1+i;
        uint16_t y_pos=134+1;
        tft.drawLine(x_pos, y_pos-2, x_pos, y_pos-y_range-2,TFT_BLACK);
        tft.drawPixel(x_pos,20+1,TFT_DARKGREY);
        tft.drawPixel(x_pos,49+1,TFT_DARKGREY);
        tft.drawPixel(x_pos,77+1,TFT_DARKGREY);
        tft.drawPixel(x_pos,106+1,TFT_DARKGREY);

        data=batmonRing_getValue((uint16_t)(x_delta*(float)i));
        //Serial.println("data_point ["+String((uint16_t)(x_delta*(float)i))+"]: "+String(data_point->voltage)+", "+String(data_point->temperature));
        uint16_t y_voltage = y_pos - ((y_range * (data->voltage - 1000)) / 400);
        uint16_t y_temp =    y_pos - ((y_range * data->temperature) / 40);
        tft.drawPixel(x_pos,y_voltage,TFT_SKYBLUE);
        tft.drawPixel(x_pos,y_temp,TFT_RED); 
    }
    old_data_point = data_point;
}

