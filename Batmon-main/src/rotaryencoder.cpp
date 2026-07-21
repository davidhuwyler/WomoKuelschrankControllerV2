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
bool getEncoderButtonToggleState() {
    return encoderButtonToggle;
}


void IRAM_ATTR encoderButtonISR()
{
    encoderButtonPressed = true;
    encoderButtonToggle = !encoderButtonToggle;
}

void rotaryencoder_init()
{
    // ---------------- Encoder ----------------
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    encoder.attachHalfQuad(ENCODER_A_PIN, ENCODER_B_PIN);
    encoder.setCount(0);

    // ---------------- Button ----------------
    pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_BTN_PIN),
        encoderButtonISR,
        FALLING);
}
