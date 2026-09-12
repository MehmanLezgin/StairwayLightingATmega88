#include "hardware/AsyncUltrasonic.h"
#include "StairsLighting.h"
#include "StairPWM.h"
#include "hardware/Keyboard.h"
#include "hardware/LdrSensor.h"

// #define __DEBUG__STAIRS__

#define PIN_KEYBOARD A6
#define PIN_LDR A7

#define LDR_THRESHOLD 100

AsyncUltrasonic sonarUpper(DDRC, PORTC, PINC, PC0);
AsyncUltrasonic sonarLower(DDRB, PORTB, PINB, PB5);
Keyboard keyboard(PIN_KEYBOARD);
LdrSensor ldrSensor(PIN_LDR, LDR_THRESHOLD);

StairsLighting stairsLighting(sonarLower, sonarUpper, keyboard, ldrSensor);

#ifdef __DEBUG__STAIRS__
const uint8_t LED_PINS[STAIRS_PWM_CHANNELS] =
    {
        2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, A1, A2, A3, A4, A5};
#else

const uint8_t LED_PINS[STAIRS_PWM_CHANNELS] =
    {
        4, 3, 2, A5, A4, A3, A2, A1,
        5, 6, 7, 8, 9, 10, 11, 12};
#endif

void setup()
{
    Serial.begin(9600);

    StairPWM::getInstance().begin();

    stairsLighting.begin();

    PCICR |= (1 << PCIE0) | (1 << PCIE1);
    PCMSK0 |= (1 << PCINT5);
    PCMSK1 |= (1 << PCINT8);
}

void loop()
{
    stairsLighting.update();
}

ISR(PCINT1_vect)
{
    const auto state = sonarUpper.getState();

    if (state >= AsyncUltrasonic::WAITING_FOR_HIGH &&
        state <= AsyncUltrasonic::MEASURING)
    {
        sonarUpper.handleInterrupt();
    }
}

ISR(PCINT0_vect)
{
    const auto state = sonarLower.getState();

    if (state >= AsyncUltrasonic::WAITING_FOR_HIGH &&
        state <= AsyncUltrasonic::MEASURING)
    {
        sonarLower.handleInterrupt();
    }
}

ISR(TIMER2_COMPA_vect)
{
    static uint8_t phase = 0;
    phase++;

    for (uint8_t i = 0; i < STAIRS_PWM_CHANNELS; i++)
    {
        // Вызываем обновленный быстрый метод
        StairPWM::getInstance().update(i, phase);
    }
}
