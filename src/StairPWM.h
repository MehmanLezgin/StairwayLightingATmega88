#pragma once

#include "PwmPin.h"
#include <stdint.h>

#ifndef STAIRS_PWM_CHANNELS
#define STAIRS_PWM_CHANNELS 16
#endif

class StairPWM {
private:
    PwmPin channels[STAIRS_PWM_CHANNELS];

    uint8_t phase = 0;
    uint32_t lastTick = 0;

public:

    void begin(const uint8_t* pins);

    void set(uint8_t channel, uint8_t brightness);

    uint8_t get(uint8_t channel) const;

    void update();
};