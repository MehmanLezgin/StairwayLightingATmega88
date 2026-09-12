#include "Keyboard.h"
#include <Arduino.h>

#define __KEYBOARD_GET_CURRENT_KEY_FLAG(flags) ((flags >> 0) & 0x0F)
#define __KEYBOARD_GET_PREVIOUS_KEY_FLAG(flags) ((flags >> 4) & 0x0F)
#define __KEYBOARD_GET_RAW_KEY_FLAG(flags) ((flags >> 8) & 0x0F)

#define __KEYBOARD_SET_CURRENT_KEY_FLAG(flags, value) (flags = (flags & ~(0x0F << 0)) | ((value & 0x0F) << 0))
#define __KEYBOARD_SET_PREVIOUS_KEY_FLAG(flags, value) (flags = (flags & ~(0x0F << 4)) | ((value & 0x0F) << 4))
#define __KEYBOARD_SET_RAW_KEY_FLAG(flags, value) (flags = (flags & ~(0x0F << 8)) | ((value & 0x0F) << 8))

#define __KEYBOARD_GET_STATE_KEY_FLAG(flags) ((flags >> 12) & 0x03)

#define __KEYBOARD_SET_STATE_NONE_KEY_FLAG(flags) (flags = (flags & ~(0x03 << 12)) | (__KEYBOARD_STATE_NONE << 12))
#define __KEYBOARD_SET_STATE_CLICKED_KEY_FLAG(flags) (flags = (flags & ~(0x03 << 12)) | (__KEYBOARD_STATE_CLICKED << 12))
#define __KEYBOARD_SET_STATE_RELEASED_KEY_FLAG(flags) (flags = (flags & ~(0x03 << 12)) | (__KEYBOARD_STATE_RELEASED << 12))

#define __KEYBOARD_STATE_NONE 0
#define __KEYBOARD_STATE_CLICKED 1
#define __KEYBOARD_STATE_RELEASED 2

Keyboard::Keyboard(uint8_t pin) : _pin(pin) {}

void Keyboard::begin()
{
    pinMode(_pin, INPUT);
}

int8_t Keyboard::scanKey()
{
    int16_t adcValue = analogRead(_pin);

    if (adcValue < 50)
        return KEY_A; // ~0V (ADC 0)
    if (adcValue > 470 && adcValue < 550)
        return KEY_B; // ~2.5V (ADC 512)
    if (adcValue > 640 && adcValue < 720)
        return KEY_C; // ~3.33V (ADC 682)

    return __NOT_A_KEY__; // ~5V (ADC 1023)
}

void Keyboard::update()
{
    int8_t newKey = scanKey();
    uint8_t rawKey = __KEYBOARD_GET_RAW_KEY_FLAG(keyFlags);

    __KEYBOARD_SET_STATE_NONE_KEY_FLAG(keyFlags);

    if (newKey != rawKey)
    {
        __KEYBOARD_SET_RAW_KEY_FLAG(keyFlags, newKey);
        debounceTickCounter = millis();
        return;
    }

    if (millis() - debounceTickCounter < DEBOUNCE_DELAY)
    {
        return;
    }

    uint8_t previousKey = __KEYBOARD_GET_CURRENT_KEY_FLAG(keyFlags);
    uint8_t currentKey = rawKey;

    if (currentKey == previousKey)
        return;

    __KEYBOARD_SET_PREVIOUS_KEY_FLAG(keyFlags, previousKey);
    __KEYBOARD_SET_CURRENT_KEY_FLAG(keyFlags, currentKey);

    if (previousKey == __NOT_A_KEY__ && currentKey != __NOT_A_KEY__)
    {
        __KEYBOARD_SET_STATE_CLICKED_KEY_FLAG(keyFlags);
    }
    else if (previousKey != __NOT_A_KEY__ && currentKey == __NOT_A_KEY__)
    {
        __KEYBOARD_SET_STATE_RELEASED_KEY_FLAG(keyFlags);
    }
}

int8_t Keyboard::getKey() { return __KEYBOARD_GET_CURRENT_KEY_FLAG(keyFlags); }

int8_t Keyboard::getPreviousKey() { return __KEYBOARD_GET_PREVIOUS_KEY_FLAG(keyFlags); }

bool Keyboard::isPressed(int8_t key) { return getKey() == key; }
bool Keyboard::isClicked(int8_t key) { return isClicked() && getKey() == key; }
bool Keyboard::isReleased(int8_t key) { return isReleased() && getPreviousKey() == key; }

bool Keyboard::isPressed() { return getKey() != __NOT_A_KEY__; }
bool Keyboard::isClicked() { return __KEYBOARD_GET_STATE_KEY_FLAG(keyFlags) == __KEYBOARD_STATE_CLICKED; }
bool Keyboard::isReleased() { return __KEYBOARD_GET_STATE_KEY_FLAG(keyFlags) == __KEYBOARD_STATE_RELEASED; }