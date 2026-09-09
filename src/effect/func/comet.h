#pragma once

#include "../Effect.h"

namespace Effects
{
    uint8_t comet(Effect::Context& ctx)
    {
        const uint8_t STEPS = 16;

        const uint8_t tail[] = {
            255,
            140,
            65,
            25,
            8};

        const uint8_t TAIL = sizeof(tail);

        uint16_t position = ctx.dt / 50;

        uint8_t phase = position % ((STEPS - 1) * 2);

        uint8_t head;

        if (phase < STEPS)
            head = phase;
        else
            head = (STEPS - 1) * 2 - phase;

        int16_t distance = head - ctx.stepIdx;

        if (distance < 0 || distance >= TAIL)
            return 0;

        return tail[distance];
    }
}

/*uint8_t comet(uint8_t step, uint32_t t)
{
    uint16_t head = t / 100;

    int16_t distance = head - step;

    if (distance < 0 || distance > 5)
        return 0;

    return 255 - distance * 50;
}*/

// uint8_t comet(uint8_t step, uint32_t t)
// {
//     uint16_t head = t / 50;

//     int16_t distance = head - step;

//     if (distance < 0 || distance >= sizeof(fade))
//         return 0;

//     return fade[distance];
// }