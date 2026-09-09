#include "AsyncUltrasonic.h"
#include "StairsLighting.h"

AsyncUltrasonic sonarUpper(DDRB, PORTB, PINB, PB5);
AsyncUltrasonic sonarLower(DDRC, PORTC, PINC, PC0);

StairsLighting stairsLighting(sonarLower, sonarUpper);

void setup()
{
    Serial.begin(9600);

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