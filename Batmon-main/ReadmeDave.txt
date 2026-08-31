Product: TTGO T-Display esp32
Firmware tested an ready on 25.8.2026
---------------------------------------------------

One can store up to 22h of voltage + temp data and display it on the voltage graph (2s press on button).

Known issue:
If in voltage graph mode: Timer does not advance...

---------------------------------------------------
TFT Resolution:

#define TFT_WIDTH  135
#define TFT_HEIGHT 240


---------------------------------------------------
Rotaryencoder Pins:
#define ENCODER_A_PIN   36
#define ENCODER_B_PIN   37
#define ENCODER_BTN_PIN 32

Kühlschrank Enable Pin:
#define POWER_OUT_PIN 17


---------------------------------------------------
Changes in the TFT Library:

Change in:
C:\Users\david\Desktop\Repos\WomoKuelschrankControllerV2\Batmon-main\.pio\libdeps\esp32dev\TFT_eSPI\User_Setup_Select.h

Disable:
//#include <User_Setup.h>           // Default setup is root library folder

Enable:
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT

