#include "hardware/AsyncUltrasonic.h"
#include "StairsLighting.h"
#include "StairPWM.h"
#include "hardware/Keyboard.h"
#include "hardware/LdrSensor.h"


#define PIN_KEYBOARD A6
#define PIN_LDR A7

#define LDR_THRESHOLD 100

AsyncUltrasonic sonarUpper(DDRC, PORTC, PINC, PC0);
AsyncUltrasonic sonarLower(DDRB, PORTB, PINB, PB5);
Keyboard keyboard(PIN_KEYBOARD);
LdrSensor ldrSensor(PIN_LDR, LDR_THRESHOLD);

StairsLighting stairsLighting(sonarLower, sonarUpper, keyboard, ldrSensor);

void setup()
{
    stairsLighting.begin();
}

void loop()
{
    stairsLighting.update();
}

ISR(PCINT1_vect)
{
    PCMSK1 &= ~(1 << PCINT8);

    const auto state = sonarUpper.getState();

    if (state >= AsyncUltrasonic::WAITING_FOR_HIGH &&
        state <= AsyncUltrasonic::MEASURING)
    {
        sonarUpper.handleInterrupt();
    }

    PCMSK1 |= (1 << PCINT8);
}

ISR(PCINT0_vect)
{
    PCMSK0 &= ~(1 << PCINT5);

    const auto state = sonarLower.getState();

    if (state >= AsyncUltrasonic::WAITING_FOR_HIGH &&
        state <= AsyncUltrasonic::MEASURING)
    {
        sonarLower.handleInterrupt();
    }

    PCMSK0 |= (1 << PCINT5);
}