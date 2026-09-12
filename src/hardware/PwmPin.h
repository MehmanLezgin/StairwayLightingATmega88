#pragma once
#include <Arduino.h>

#define PWM_TICK_US  16

typedef uint8_t PWM_INT;

class PwmPin {
private:
    uint8_t pin;
    uint16_t brightness;
    volatile uint8_t* outReg;
    uint8_t mask;

public:
    void begin(uint8_t p);

    void set(PWM_INT value);

    PWM_INT get() const;

    void update(uint8_t phase);
};

