#pragma once
#include <stdint.h>

class LdrSensor
{
private:
    uint8_t _pin;
    uint16_t _threshold;
    bool _isDarkState;

public:
    LdrSensor(uint8_t pin, uint16_t threshold = 400);
    uint16_t getRawValue();
    void setThreshold(uint16_t threshold);
    uint16_t getThreshold() const;
    bool isDark();
};