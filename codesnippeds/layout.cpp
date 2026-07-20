#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void drawScreen()
{
    tft.fillScreen(TFT_BLACK);

    // ---------- upper left ----------
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextDatum(TL_DATUM);

    tft.setTextFont(4);
    tft.drawString("12.65V", 15, 20);

    tft.setTextFont(2);
    tft.drawString("Current", 15, 65);

    // ---------- upper right ----------
    tft.setTextFont(4);
    tft.drawString("12.22V", 175, 20);

    tft.setTextFont(2);
    tft.drawString("Disconnect", 175, 65);

    // ---------- lower left ----------
    tft.setTextFont(6);
    tft.drawString("12", 15, 105);

    // superscript
    tft.setTextFont(2);
    tft.drawString("12", 72, 110);

    tft.setTextFont(2);
    tft.drawString("Timer", 15, 145);

    // ---------- lower right ----------
    tft.setTextFont(6);
    tft.drawString("ON", 175, 100);

    tft.setTextFont(2);
    tft.drawString("State", 175, 145);
}

void setup()
{
    tft.init();

    // TTGO T-Display 320x170 usually requires landscape orientation
    tft.setRotation(1);

    drawScreen();
}

void loop()
{
}
