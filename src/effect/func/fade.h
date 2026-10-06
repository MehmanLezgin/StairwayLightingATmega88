#pragma once

#include "../Effect.h"

namespace Effects
{
    PWM_INT fade1(Effect::Context &ctx)
{
    const uint16_t STEP_DELAY = 500;
    const uint8_t TOTAL_STEPS = STAIRS_PWM_CHANNELS;
    const uint32_t TOTAL_EFFECT_DURATION = TOTAL_STEPS * STEP_DELAY;

    // 1. Boundary & Finish evaluation
    if (ctx.dt >= TOTAL_EFFECT_DURATION || ctx.stepIdx >= TOTAL_STEPS)
    {
        if (ctx.dt >= TOTAL_EFFECT_DURATION) ctx.effect->finish();
        return ctx.isLightOut ? 0 : ctx.maxBrightness;
    }

    uint32_t stepStartTime = ctx.stepIdx * STEP_DELAY;
    if (ctx.dt < stepStartTime)
    {
        return ctx.currentValue;
    }

    // 2. Compute local timeframe bounded strictly [0 ... STEP_DELAY]
    uint32_t localDt = ctx.dt - stepStartTime;
    if (localDt > STEP_DELAY) localDt = STEP_DELAY;

    // 3. Establish structural target boundaries
    PWM_INT targetValue = ctx.isLightOut ? 0 : ctx.maxBrightness;
    
    // Quick exit if already at the target
    if (ctx.currentValue == targetValue) return targetValue;

    // 4. Linear interpolation between ctx.currentValue and targetValue
    if (ctx.isLightOut)
    {
        // Fade Out: Interpolate downwards from currentValue to 0
        uint32_t delta = ctx.currentValue;
        uint32_t progress = (localDt * delta) / STEP_DELAY;
        return (progress >= delta) ? 0 : (delta - progress);
    }
    else
    {
        // Fade In: Interpolate upwards from currentValue to maxBrightness
        uint32_t delta = ctx.maxBrightness - ctx.currentValue;
        uint32_t progress = (localDt * delta) / STEP_DELAY;
        return ctx.currentValue + progress;
    }
}

}

#define BREATH_MAX_BRIGHTNESS 100
#define BREATH_MIN_BRIGHTNESS 8
#define BREATH_CYCLE_DURATION 6000
#define BREATH_FADE_OUT_MS 60

namespace Effects
{
    PWM_INT cozyBreathing(Effect::Context &ctx)
    {
        const uint32_t DYNAMIC_RANGE = BREATH_MAX_BRIGHTNESS - BREATH_MIN_BRIGHTNESS;
        const uint8_t TOTAL_STEPS = 16;

        uint16_t cycleTime = ctx.dt % BREATH_CYCLE_DURATION;
        uint16_t t = ((uint32_t)cycleTime * 1023) / BREATH_CYCLE_DURATION;

        uint32_t waveForm = 0;

        if (t < 440)
        {
            waveForm = ((uint32_t)t * 1023) / 440;
        }
        else if (t >= 440 && t < 560)
        {
            waveForm = 1023;
        }
        else
        {
            uint16_t tRemaining = 1023 - t;
            waveForm = ((uint32_t)tRemaining * 1023) / 463;
        }

        uint32_t curveResult = (waveForm * waveForm) / 1023;
        PWM_INT targetBrightness = BREATH_MIN_BRIGHTNESS + ((curveResult * DYNAMIC_RANGE) / 1023);
        PWM_INT finalValue = targetBrightness;

        if (ctx.isLightOut)
        {
            if (ctx.dt >= BREATH_FADE_OUT_MS)
            {
                finalValue = 0;
            }
            else
            {
                finalValue = ((uint32_t)targetBrightness * (BREATH_FADE_OUT_MS - ctx.dt)) / BREATH_FADE_OUT_MS;
            }
        }

        // Master Finish Check for Breathing Shutdown sequence:
        // Terminate the effect loop ONLY on the 16th step handler after the clock hits 0.
        if (ctx.isLightOut && ctx.dt >= BREATH_FADE_OUT_MS && ctx.stepIdx == (TOTAL_STEPS - 1))
        {
            ctx.effect->finish();
        }

        return finalValue;
    }
}
