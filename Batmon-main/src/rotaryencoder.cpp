#include <Arduino.h>
#include <ESP32Encoder.h>
#include "rotaryencoder.h"

#define ENCODER_A_PIN   32
#define ENCODER_B_PIN   33
#define ENCODER_BTN_PIN 25


volatile bool encoderButtonPressed = false;
volatile bool encoderButtonToggle = false;
volatile bool oneSecondTick = false;

ESP32Encoder encoder;

bool getEncoderButtonToggleState() {
    return encoderButtonToggle;
}


void IRAM_ATTR encoderButtonISR()
{
    encoderButtonPressed = true;
    encoderButtonToggle = !encoderButtonToggle;
}

void rotaryEncoder_init()
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

void loop()
{

}