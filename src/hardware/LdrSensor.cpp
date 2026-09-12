#include "LdrSensor.h"
#include <Arduino.h>

#define LDR_HYSTERESIS 15

LdrSensor::LdrSensor(uint8_t pin, uint16_t threshold)
    : _pin(pin), _threshold(threshold), _isDarkState(false)
{
}

uint16_t LdrSensor::getRawValue()
{
    return analogRead(_pin);
}

void LdrSensor::setThreshold(uint16_t threshold)
{
    if (threshold > 1023)
        threshold = 1023;
    _threshold = threshold;
}

uint16_t LdrSensor::getThreshold() const
{
    return _threshold;
}

bool LdrSensor::isDark()
{
    uint16_t currentLight = analogRead(_pin);

    if (_isDarkState && currentLight > (_threshold + LDR_HYSTERESIS))
        _isDarkState = false;
    else if ((currentLight + LDR_HYSTERESIS) <= _threshold)
        _isDarkState = true;

    return _isDarkState;
}
