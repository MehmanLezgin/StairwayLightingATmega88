#include "StairPWM.h"

void StairPWM::begin(const uint8_t *pins)
{
    for (uint8_t i = 0; i < STAIRS_PWM_CHANNELS; i++)
    {
        channels[i].begin(pins[i]);
    }

    cli();

    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;
    TCCR2A |= (1 << WGM21);
    TCCR2B |= (1 << CS21);
    OCR2A = 127;

    TIFR2 |= (1 << OCF2A);
    TIMSK2 |= (1 << OCIE2A);

    sei();
}

void StairPWM::set(uint8_t channel, uint8_t brightness)
{
    if (channel >= STAIRS_PWM_CHANNELS)
        return;

    channels[channel].set(brightness);
}

uint8_t StairPWM::get(uint8_t channel) const
{
    if (channel >= STAIRS_PWM_CHANNELS)
        return 0;

    return channels[channel].get();
}

void StairPWM::update(uint8_t channel, uint8_t phase)
{
    channels[channel].update(phase);
}
