#include "PwmPin.h"

void PwmPin::begin(uint8_t p) {
        pin = p;
        brightness = 0;

        pinMode(pin, OUTPUT);

        // Hardware PWM pins of ATmega328P
        hardware =
            pin == 3  ||
            pin == 5  ||
            pin == 6  ||
            pin == 9  ||
            pin == 10 ||
            pin == 11;

        if (hardware) {
            analogWrite(pin, 0);
            return;
        }

        outReg = portOutputRegister(digitalPinToPort(pin));
        mask   = digitalPinToBitMask(pin);

        *outReg &= ~mask;
    }

    void PwmPin::set(uint8_t value) {
        brightness = value;

        if (hardware)
            analogWrite(pin, value);
    }

    uint8_t PwmPin::get() const {
        return brightness;
    }

    void PwmPin::softwareUpdate(uint8_t phase) {
        if (hardware)
            return;

        if (brightness > phase)
            *outReg |= mask;
        else
            *outReg &= ~mask;
    }