#include <Arduino.h>
#include "batmon.h"
#include "tft.h"

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
bool lcd_change_val_mode = false;

/* ------------------------  100ms irq timer ------------------------ */
int isr_sample_voltage_trigger = 0;
int isr_count_seconds_trigger = 0;

hw_timer_t *timer = nullptr;
void IRAM_ATTR timerISR()
{
  if(lcd_change_val_mode==false)
  {
    draw_lcd_selector()
    isr_sample_voltage_trigger++;
    isr_count_seconds_trigger++;
    
    if(isr_sample_voltage_trigger == 20)
    {
        sample_voltage();
        drawScreen_Voltage();
        isr_sample_voltage_trigger = 0;
    }
  }
      
  if(isr_count_seconds_trigger == 10)
  {
    handle_time_every_second();
    drawScreen_Timer();
    isr_count_seconds_trigger = 0;    
  }    
}


/* ------------------------  functions ------------------------ */
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
      drawScreen_State();
      digitalWrite(POWER_OUT_PIN, LOW);
    }
    else:
    {
      digitalWrite(POWER_OUT_PIN, LOW);
    }

}

/* ------------------------  setup ------------------------ */
void setup() {
  Serial.begin(115200);
  batmon_init();
  lcd_init();
  rotaryencoder_init();

  pinMode(POWER_OUT_PIN, OUTPUT);

  timer = timerBegin(1000000);              // 1 MHz timer clock
  timerAttachInterrupt(timer, &timerISR);
  timerAlarm(timer, 100000, true, 0);      // 100ms periodic interrupt

}




/* ------------------------  loop ------------------------ */
void loop() {   
  if(lcd_change_val_mode == true) {
    change_lcd_val();
  }
  else
  {
    if(lcd_selected != rotary.value())
    {
      lcd_selected = rotary.value();
      draw_lcd_selector();
    }
  }
  eval_power_output()
}
