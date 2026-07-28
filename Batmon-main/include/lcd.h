#pragma once

void lcd_init(void);
void drawScreen_State(bool* old_state, bool* state);
void drawScreen_Voltage(float* old_voltage, float* voltage_current);
void drawScreen_Disconnect(float* old_voltage_disconnect, float* voltage_disconnect);
void drawScreen_Timer(int* old_time, int* timer_min, int* old_time_s, int* timer_seconds);

void draw_lcd_selector(bool* selector_on, int lcd_selected);
void draw_static_overview_display();
void draw_voltage_graph_display(bool doInitScreen);

void highlight_state();
void highlight_state_off();
void highlight_disconnect();
void highlight_disconnect_off();
void highlight_timer();
void highlight_timer_off();