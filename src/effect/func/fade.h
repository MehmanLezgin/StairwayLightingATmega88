#pragma once

#include "../Effect.h"

namespace Effects
{
    uint8_t fade(Effect::Context& ctx)
    {
        const uint8_t STEPS = 16;
        uint16_t position = ctx.dt / 20;
        if (position >= 0xff)
        {
            ctx.effect->finish();
            return ctx.isLightOut * 0xff - 0xff;
        }

        return ctx.isLightOut * 0xff - position;
    }
}