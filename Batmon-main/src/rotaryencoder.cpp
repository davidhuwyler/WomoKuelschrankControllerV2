#include <Arduino.h>
#include <ESP32Encoder.h>
#include "rotaryencoder.h"

#define ENCODER_A_PIN   36
#define ENCODER_B_PIN   37
#define ENCODER_BTN_PIN 38


volatile bool encoderButtonPressed = false;
volatile bool encoderButtonToggle = false;
volatile bool oneSecondTick = false;

static uint32_t rotaryModuloVal=0xFFFFFFFF;
static uint32_t rotaryMinVal = 0;
static uint32_t rotaryMaxVal = 0xFFFFFFFF;

unsigned long buttonPressedTime = 0;
unsigned long buttonReleaseTime = 0;
bool longpressDone = true;


ESP32Encoder encoder;

void setEncoderRange(uint32_t value, uint32_t min, uint32_t max) {
    encoder.setCount(value);
    rotaryMinVal = min;
    rotaryMaxVal = max;
}

uint32_t getEncoderValue() {
    int32_t val = encoder.getCount();
    if (val < rotaryMinVal) {
        val = rotaryMinVal;
        encoder.setCount(val);
    } else if (val > rotaryMaxVal) {
        val = rotaryMaxVal;
        encoder.setCount(val);
    }
    return val;
}
enum enumScreenMode getScreenMode(enum enumScreenMode currentScreenMode) {
    if((buttonReleaseTime - buttonPressedTime) > 1000 && !longpressDone && encoderButtonPressed==false) {
        encoderButtonToggle = false;
        longpressDone = true;
        Serial.println("DEBUG long press" + String(buttonPressedTime) + ", " + String(buttonReleaseTime));
        return currentScreenMode==SCREEN_MODE_OVERVIEW?SCREEN_MODE_VOLTAGE_GRAPH : currentScreenMode==SCREEN_MODE_VOLTAGE_GRAPH?SCREEN_MODE_OVERVIEW : currentScreenMode;
    }
    else 
    {
        if(currentScreenMode == SCREEN_MODE_VOLTAGE_GRAPH) {
            return SCREEN_MODE_VOLTAGE_GRAPH;
        }
        return encoderButtonToggle ? SCREEN_MODE_SELECTED_VALUE : SCREEN_MODE_OVERVIEW;
    }    
}


void IRAM_ATTR encoderButtonPressedISR()
{
    if(digitalRead(ENCODER_BTN_PIN) == LOW) {
        if(encoderButtonPressed) {
            // Button is already pressed, ignore this interrupt
            return;
        }
        encoderButtonPressed = true;
        buttonPressedTime = millis();
    } else {
        if(!encoderButtonPressed) {
            // Button is already released, ignore this interrupt
            return;
        }
        encoderButtonPressed = false;
        buttonReleaseTime = millis();
        encoderButtonToggle = !encoderButtonToggle;
        longpressDone = false;
    }
}

void rotaryencoder_init()
{
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    encoder.attachHalfQuad(ENCODER_A_PIN, ENCODER_B_PIN);
    encoder.setCount(0);
    setEncoderRange(0, 0, 2); 

    // ---------------- Button ----------------
    pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_BTN_PIN),
        encoderButtonPressedISR,
        CHANGE);
}
