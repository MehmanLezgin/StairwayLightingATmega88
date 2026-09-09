#pragma once
#include <Arduino.h>

#define PWM_TICK_US  16

class PwmPin {
private:
    uint8_t pin;
    uint8_t brightness;
    bool hardware;

    volatile uint8_t* outReg;
    uint8_t mask;

public:
    void begin(uint8_t p);

    void set(uint8_t value);

    uint8_t get() const;

    void softwareUpdate(uint8_t phase);
};