#include "AsyncUltrasonic.h"
#include "StairsLighting.h"
#include "StairPWM.h"

AsyncUltrasonic sonarUpper(DDRB, PORTB, PINB, PB5);
AsyncUltrasonic sonarLower(DDRC, PORTC, PINC, PC0);

StairsLighting stairsLighting(sonarLower, sonarUpper);

const uint8_t LED_PINS[STAIRS_PWM_CHANNELS] =
    {
        4, 3, 2, A5, A4, A3, A2, A1,
        5, 6, 7, 8, 9, 10, 11, 12};

void setup()
{
    Serial.begin(9600);

    StairPWM::getInstance().begin(LED_PINS);

    stairsLighting.begin();

    PCICR |= (1 << PCIE0) | (1 << PCIE1);
    PCMSK0 |= (1 << PCINT5);
    PCMSK1 |= (1 << PCINT8);
}

void loop()
{
    stairsLighting.update();
}

ISR(PCINT0_vect)
{
    const auto state = sonarUpper.getState();

    if (state >= AsyncUltrasonic::WAITING_FOR_HIGH &&
        state <= AsyncUltrasonic::MEASURING)
    {
        sonarUpper.handleInterrupt();
    }
}

ISR(PCINT1_vect)
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
        StairPWM::getInstance().update(i, phase);
    }
}