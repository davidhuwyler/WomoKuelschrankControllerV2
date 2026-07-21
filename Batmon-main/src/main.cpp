#include <Arduino.h>
#include "batmon.h"
#include "lcd.h"
#include "rotaryencoder.h"

/* ------------------------  PINS ------------------------ */
#define POWER_OUT_PIN 25

/* ------------------------  globals ------------------------ */
float voltage_current = 0.00;
float voltage_disconnect = 12.20;
bool lcd_change_val_mode = false;
bool state = false;

bool timer_is_used = false;
int timer_min = 0;
int timer_seconds = 0;

bool selector_on = false;

float old_voltage = 0;
float old_voltage_disconnect = 0;
int old_time = 0;
int old_time_s = 0;
bool old_state = false;

int lcd_selected = 0; // 0=discon, 1=timer, 2=state

/* ------------------------  Prototypes ------------------------ */
void handle_time_every_second();
void sample_voltage();

/* ------------------------  100ms irq timer ------------------------ */
int isr_sample_voltage_trigger = 0;
int isr_count_seconds_trigger = 0;

hw_timer_t *timer = nullptr;
void IRAM_ATTR timerISR()
{
  if(lcd_change_val_mode==false)
  {
    draw_lcd_selector(&selector_on, lcd_selected);
    isr_sample_voltage_trigger++;
    isr_count_seconds_trigger++;
    
    if(isr_sample_voltage_trigger == 20)
    {
        sample_voltage();
        drawScreen_Voltage(&old_voltage, &voltage_current);
        isr_sample_voltage_trigger = 0;
    }
  }
      
  if(isr_count_seconds_trigger == 10)
  {
    handle_time_every_second();
    drawScreen_Timer(&old_time, &timer_min, &old_time_s, &timer_seconds);
    isr_count_seconds_trigger = 0;    
  }    
}


/* ------------------------  functions ------------------------ */
void drawScreen() {
    drawScreen_State(&old_state, &state);
    drawScreen_Voltage(&old_voltage, &voltage_current);
    drawScreen_Disconnect(&old_voltage_disconnect, &voltage_disconnect);
    drawScreen_Timer(&old_time, &timer_min, &old_time_s, &timer_seconds);
}

void set_state_value()
{
  highlight_state();
  highlight_disconnect_off();
  highlight_timer_off();
  setEncoderRange(0,0,1);
  while(lcd_change_val_mode == true)
  {
    if(getEncoderValue()==0)
    {
      state=false;
    }
    else
    {
      state=true;
    }
    drawScreen_State(&old_state, &state);
  }
}

void set_timer_value()
{
  highlight_state_off();
  highlight_disconnect_off();
  highlight_timer();
  setEncoderRange(timer_min,0,1440);
  while(lcd_change_val_mode == true)
  {
    timer_min = getEncoderValue();
    drawScreen_Timer(&old_time, &timer_min, &old_time_s, &timer_seconds);
  }
  if(timer_min != 0)
  {
    timer_is_used = true;
  }
}

void set_disconnect_value()
{
  highlight_state_off();
  highlight_disconnect();
  highlight_timer_off();
  setEncoderRange((int)(voltage_disconnect*100),1190,1300);
  while(lcd_change_val_mode == true)
  {
    voltage_disconnect = (getEncoderValue())/100;
    drawScreen_Disconnect(&old_voltage_disconnect, &voltage_disconnect);
  }
}


void change_lcd_val()
{
  if(lcd_selected == 0) // disconnect
  {
    set_disconnect_value();            
  }
  else if(lcd_selected == 1) // timer
  {
    set_timer_value();
  }
  else if(lcd_selected == 2) // state
  {
    set_state_value();
  }
  setEncoderRange(0, 0, 2); 
}


void handle_time_every_second()
{
  if(timer_is_used == true && timer_min > 0 && timer_seconds == 0)
  {
    timer_seconds = 60;
    timer_min = timer_min - 1;
  }

  if(timer_is_used == true && timer_seconds > 0)
  {
    timer_seconds = timer_seconds - 1;
  }
}

void sample_voltage()
{
  struct BM6Data data;
  get_batmon_data(&data);
  voltage_current = data.voltage;
}

void eval_power_output()
{
    if (   
       (voltage_current >= voltage_disconnect) &&
       (timer_is_used == false || timer_min > 0 || timer_seconds > 0) &&
       state==true
       )
    {
      digitalWrite(POWER_OUT_PIN, HIGH);
    }
    else if(state==false)
    {
      state=false;
      drawScreen_State(&old_state, &state);
      digitalWrite(POWER_OUT_PIN, LOW);
    }
    else
    {
      digitalWrite(POWER_OUT_PIN, LOW);
    }

}

/* ------------------------  setup ------------------------ */
void setup() {
  Serial.begin(115200);
  rotaryencoder_init();
  batmon_init();
  lcd_init();
  

  pinMode(POWER_OUT_PIN, OUTPUT);

  timer = timerBegin(0,80,true);              // 1 MHz timer clock
  timerAttachInterrupt(timer, &timerISR,true);
  timerAlarmWrite(timer, 1000000, true);      // 100ms periodic interrupt
  timerAlarmEnable(timer);
}




/* ------------------------  loop ------------------------ */
void loop() {   
  if(lcd_change_val_mode == true) {
    change_lcd_val();
  }
  else
  {
    if(lcd_selected != getEncoderValue())
    {
      lcd_selected = getEncoderValue();
      draw_lcd_selector(&selector_on, lcd_selected);
    }
  }
  eval_power_output();
}
