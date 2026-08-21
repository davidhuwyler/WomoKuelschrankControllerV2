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
    rotaryMinVal = min;
    rotaryMaxVal = max;

    if (max >= min) {
        uint64_t rangeSize = (uint64_t)max - (uint64_t)min + 1ULL;
        rotaryModuloVal = rangeSize <= UINT32_MAX ? (uint32_t)rangeSize : 0xFFFFFFFF;
    } else {
        rotaryModuloVal = 0;
    }

    int32_t normalizedValue = value;
    if (rotaryModuloVal != 0xFFFFFFFF && rotaryMaxVal >= rotaryMinVal) {
        int64_t rangeSize = (int64_t)rotaryModuloVal;
        int64_t offset = (int64_t)normalizedValue - (int64_t)rotaryMinVal;
        int64_t wrapped = ((offset % rangeSize) + rangeSize) % rangeSize;
        normalizedValue = (int32_t)(wrapped + (int64_t)rotaryMinVal);
    } else if (normalizedValue < (int32_t)rotaryMinVal) {
        normalizedValue = (int32_t)rotaryMinVal;
    } else if (normalizedValue > (int32_t)rotaryMaxVal) {
        normalizedValue = (int32_t)rotaryMaxVal;
    }

    encoder.setCount(normalizedValue);
}

uint32_t getEncoderValue() {
    int32_t val = encoder.getCount();

    if (rotaryModuloVal != 0xFFFFFFFF && rotaryMaxVal >= rotaryMinVal) {
        int64_t rangeSize = (int64_t)rotaryModuloVal;
        int64_t offset = (int64_t)val - (int64_t)rotaryMinVal;
        int64_t wrapped = ((offset % rangeSize) + rangeSize) % rangeSize;
        val = (int32_t)(wrapped + (int64_t)rotaryMinVal);
        encoder.setCount(val);
        return (uint32_t)val;
    }

    if (val < (int32_t)rotaryMinVal) {
        val = (int32_t)rotaryMinVal;
        encoder.setCount(val);
    } else if (val > (int32_t)rotaryMaxVal) {
        val = (int32_t)rotaryMaxVal;
        encoder.setCount(val);
    }
    return (uint32_t)val;
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
