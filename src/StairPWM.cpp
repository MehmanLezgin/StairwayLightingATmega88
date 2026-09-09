#include "StairPWM.h"

void StairPWM::begin(const uint8_t *pins)   
{
    for (uint8_t i = 0; i < STAIRS_PWM_CHANNELS; i++)
        channels[i].begin(pins[i]);

    lastTick = micros();
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

void StairPWM::update()
{
    uint32_t now = micros();

    if ((uint16_t)(now - lastTick) < PWM_TICK_US)
        return;

    lastTick += PWM_TICK_US;

    phase++;

    for (uint8_t i = 0; i < STAIRS_PWM_CHANNELS; i++)
        channels[i].softwareUpdate(phase);
}