#include <Arduino.h>
#include <TFT_eSPI.h>
#include "tft.h"


TFT_eSPI tft = TFT_eSPI();  // Create object "tft"


def drawScreen_State() :
    global old_state
    if(old_state!=state):
        tft.text((85, 80), old_state, TFT.BLACK, sysfont, 2)
        tft.text((85, 80), state, TFT.WHITE, sysfont, 2)
        old_state = state

def drawScreen_Voltage() :
    global old_voltage
    if(int(old_voltage*100)!=int(voltage_current*100)):
        tft.text((15, 40), '{0:.2f}'.format(old_voltage), TFT.BLACK, sysfont, 2)
        tft.text((15, 40), '{0:.2f}'.format(voltage_current), TFT.WHITE, sysfont, 2)
        old_voltage = voltage_current

def drawScreen_Disconnect() :
    global old_voltage_disconnect
    if(old_voltage_disconnect!=voltage_disconnect):
        tft.text((85, 40), '{0:.2f}'.format(old_voltage_disconnect), TFT.BLACK, sysfont, 2)
        tft.text((85, 40), '{0:.2f}'.format(voltage_disconnect), TFT.WHITE, sysfont, 2)
        old_voltage_disconnect = voltage_disconnect

def drawScreen_Timer() :
    global old_time
    global old_time_s
    if old_time!=timer_min:
        tft.text((15, 80), str(old_time), TFT.BLACK, sysfont, 2)
        tft.text((15, 80), str(timer_min), TFT.WHITE, sysfont, 2)
        old_time = timer_min

    if old_time_s!=timer_seconds:
        tft.text((50, 80), str(old_time_s), TFT.BLACK, sysfont, 1)
        tft.text((50, 80), str(timer_seconds), TFT.WHITE, sysfont, 1)
        old_time_s = timer_seconds

def drawScreen():
    drawScreen_State()
    drawScreen_Voltage()
    drawScreen_Disconnect()
    drawScreen_Timer()

def draw_lcd_selector() :
    global lcd_change_val_mode
    global voltage_disconnect
    global selector_on    
    global timer    
    selector_on = not selector_on

    if lcd_selected == 0: # disconnect
        highlight_timer_off()
        highlight_state_off()
        if selector_on==True:
            highlight_disconnect() 
        else:
            highlight_disconnect_off()
    elif lcd_selected == 1: # timer
        highlight_disconnect_off()
        highlight_state_off()
        if selector_on==True:
            highlight_timer() 
        else:
            highlight_timer_off()
    elif lcd_selected == 2: # state
        highlight_timer_off()
        highlight_disconnect_off()
        if selector_on==True:
            highlight_state() 
        else:
            highlight_state_off()

void highlight_state(){
    tft.drawLine(15, 65, 130, 65, TFT_RED);
}
void highlight_state_off(){
    tft.line((85, 100), (145,100), tft.BLACK)
}

def highlight_disconnect():
    tft.line((85, 60), (145,60), tft.RED)
def highlight_disconnect_off():
    tft.line((85, 60), (145,60), tft.BLACK)

def highlight_timer():
    tft.line((15, 100), (75,100), tft.RED)
def highlight_timer_off():
    tft.line((15, 100), (75,100), tft.BLACK)

def set_state_value():
    global state
    highlight_state()
    highlight_disconnect_off()
    highlight_timer_off()
    rotary.set(value=0,min_val=0,max_val=1)
    while lcd_change_val_mode ==True:
        if rotary.value()==0:
            state="OFF"
        else:
            state="ON"
        drawScreen_State()



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