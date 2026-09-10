#include "PwmPin.h"

void PwmPin::begin(uint8_t p)
{
    pin = p;
    brightness = 0;

    pinMode(pin, OUTPUT);

    outReg = portOutputRegister(digitalPinToPort(pin));
    mask = digitalPinToBitMask(pin);

    *outReg &= ~mask;
}

void PwmPin::set(uint8_t value)
{
    brightness = value;
}

uint8_t PwmPin::get() const
{
    return brightness;
}

void PwmPin::update(uint8_t phase)
{
    if (brightness > phase)
        *outReg |= mask;
    else
        *outReg &= ~mask;
}