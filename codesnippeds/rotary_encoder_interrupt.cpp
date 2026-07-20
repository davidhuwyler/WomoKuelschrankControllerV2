#include <Arduino.h>
#include <ESP32Encoder.h>

#define ENCODER_A_PIN   32
#define ENCODER_B_PIN   33
#define ENCODER_BTN_PIN 25

volatile bool encoderButtonPressed = false;
volatile bool oneSecondTick = false;

ESP32Encoder encoder;

// ESP32 hardware timer
hw_timer_t *timer = nullptr;

// ----------------------------------------------------
// Encoder button ISR
// ----------------------------------------------------
void IRAM_ATTR encoderButtonISR()
{
    encoderButtonPressed = true;
}

// ----------------------------------------------------
// 1 second timer ISR
// ----------------------------------------------------
void IRAM_ATTR timerISR()
{
    oneSecondTick = true;
}

void setup()
{
    Serial.begin(115200);

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

    // ---------------- Timer ----------------
    timer = timerBegin(1000000);              // 1 MHz timer clock
    timerAttachInterrupt(timer, &timerISR);
    timerAlarm(timer, 1000000, true, 0);      // 1 second periodic interrupt

    Serial.println("Started");
}

void loop()
{
    // Handle button press event
    if (encoderButtonPressed)
    {
        encoderButtonPressed = false;

        Serial.println("Button pressed");
    }

    // Every second
    if (oneSecondTick)
    {
        oneSecondTick = false;

        int32_t position = encoder.getCount();

        // limit to range 0..2
        if (position < 0)
        {
            position = 0;
            encoder.setCount(0);
        }

        if (position > 2)
        {
            position = 2;
            encoder.setCount(2);
        }

        Serial.print("Encoder position: ");
        Serial.println(position);
    }
}
