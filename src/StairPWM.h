#pragma once

#include <stdint.h>

#define STAIRS_PWM_CHANNELS 16u
#define PWM_MAX_VALUE 100u

typedef uint8_t PWM_INT;

class StairPWM
{
private:
    PWM_INT pwmValues[STAIRS_PWM_CHANNELS] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };
public:
    static StairPWM &getInstance()
    {
        static StairPWM instance; // Создается один раз при первом вызове, живет всегда
        return instance;
    }

    void begin();

    void set(uint8_t channel, PWM_INT brightness);

    PWM_INT get(uint8_t channel) const;

    void update(uint8_t channel, uint8_t phase);
};