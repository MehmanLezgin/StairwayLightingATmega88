#pragma once

#include <stdint.h>

#ifdef __AVR_ATmega328P__ 
#define __DEBUG__STAIRS__
#endif

#ifdef __DEBUG__STAIRS__
#define STAIRS_PWM_CHANNELS 13u
#else
#define STAIRS_PWM_CHANNELS 16u
#endif

#define PWM_MAX_VALUE 99u


typedef uint8_t PWM_INT;

class StairPWM
{
private:
    PWM_INT pwmValues[STAIRS_PWM_CHANNELS];
    PWM_INT _maxBrightness = 0xFF;
public:
    static StairPWM &getInstance()
    {
        static StairPWM instance; // Создается один раз при первом вызове, живет всегда
        return instance;
    }

    void begin(PWM_INT maxBrightness = 0xFF);

    void set(uint8_t channel, PWM_INT brightness);
    void setMaxBrightness(PWM_INT brightness);
    PWM_INT getMaxBrightness();

    PWM_INT get(uint8_t channel) const;
};