#pragma once

#include <stdint.h>

static uint8_t lerp(uint8_t a, uint8_t b, uint8_t amount)
{
    return a + ((int16_t)(b - a) * amount) / 255;
}

static uint8_t smooth(uint8_t x)
{
    return ((uint16_t)x * x * (765 - 2 * x)) / 65025;
}