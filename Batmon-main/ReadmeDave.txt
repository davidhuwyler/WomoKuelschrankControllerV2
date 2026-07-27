Product: TTGO T-Display
#define TFT_WIDTH  135
#define TFT_HEIGHT 240

Rotaryencoder Pins:
#define ENCODER_A_PIN   36
#define ENCODER_B_PIN   37
#define ENCODER_BTN_PIN 38

Kühlschrank Enable Pin:
#define POWER_OUT_PIN 17

Change in:
C:\Users\david\Desktop\Repos\WomoKuelschrankControllerV2\Batmon-main\.pio\libdeps\esp32dev\TFT_eSPI\User_Setup_Select.h

Disable:
//#include <User_Setup.h>           // Default setup is root library folder

Enable:
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT

