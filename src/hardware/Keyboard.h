#pragma once

#include <stdint.h>

#define KEY_A 0
#define KEY_B 1
#define KEY_C 2
#define __NOT_A_KEY__ 3

#define DEBOUNCE_DELAY 15

class Keyboard
{
private:
    uint8_t _pin;
    uint16_t keyFlags = 0x0888;

    uint32_t debounceTickCounter = 0;

    int8_t scanKey();

public:
    Keyboard(uint8_t pin);

    void begin();
    void update();

    int8_t getKey();
    int8_t getPreviousKey();

    bool isPressed(int8_t key);
    bool isClicked(int8_t key);
    bool isReleased(int8_t key);

    bool isPressed();
    bool isClicked();
    bool isReleased();
};