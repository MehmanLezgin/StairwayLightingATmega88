#pragma once

#include "../Effect.h"

namespace Effects
{
    PWM_INT simpleFadeOut(Effect::Context &ctx)
    {
        const uint16_t FADE_TIME = 1500;

        if (ctx.dt >= FADE_TIME)
        {
            ctx.effect->finish();
            return 0;
        }

        uint32_t progress = (ctx.dt * ctx.currentValue) / FADE_TIME;

        if (progress >= ctx.currentValue)
            return 0;

        return ctx.currentValue - progress;
    }

}

namespace Effects
{
    PWM_INT fade1(Effect::Context &ctx)
    {
        // Adjust configuration settings here
        const uint16_t STEP_DELAY = 500; // Delay between the start of consecutive stairs (smaller = tighter wave)
        const uint16_t FADE_TIME = 1500; // How long a single stair takes to complete its individual fade

        const uint8_t TOTAL_STEPS = STAIRS_PWM_CHANNELS;

        // Total effect time handles the staggered starts + the final step's fade duration
        const uint32_t TOTAL_EFFECT_DURATION = ((TOTAL_STEPS - 1) * STEP_DELAY) + FADE_TIME;

        // 1. Boundary & Finish evaluation
        if (ctx.dt >= TOTAL_EFFECT_DURATION)
        {
            ctx.effect->finish();
            return ctx.isLightOut ? 0 : ctx.maxBrightness;
        }

        if (ctx.stepIdx >= TOTAL_STEPS)
        {
            return ctx.isLightOut ? 0 : ctx.maxBrightness;
        }

        // Determine the physical stair step based on running direction
        uint8_t physicalStep = (ctx.dir == -1) ? (TOTAL_STEPS - 1 - ctx.stepIdx) : ctx.stepIdx;

        // 2. Compute the start time for this specific step
        uint32_t stepStartTime = physicalStep * STEP_DELAY;

        if (ctx.dt < stepStartTime)
        {
            return ctx.currentValue; // Not started yet
        }

        // 3. Compute local time bounded strictly to the individual fade window [0 ... FADE_TIME]
        uint32_t localDt = ctx.dt - stepStartTime;
        if (localDt > FADE_TIME)
            localDt = FADE_TIME;

        // Quick exit if already past the individual fade window
        PWM_INT targetValue = ctx.isLightOut ? 0 : ctx.maxBrightness;
        if (localDt == FADE_TIME)
            return targetValue;

        // 4. Generate high-precision Cosine S-Curve interpolation factor (0 to 1000)
        // Formula: progress = (1 - cos(pi * localDt / FADE_TIME)) / 2
        // Uses integer math scaling to keep execution extremely fast and fluid
        uint32_t angle = (localDt * 180) / FADE_TIME;

        // Fast integer approximation of: (1.0f - cosf(angle_rad)) * 500.0f
        // Yields an exceptionally smooth acceleration and deceleration profile
        int32_t cosVal = 1000 - ((angle * angle * 2) / 65); // Basic Taylor series approach
        if (cosVal < -1000)
            cosVal = -1000;
        if (cosVal > 1000)
            cosVal = 1000;

        uint32_t sCurveProgress = (1000 - cosVal) / 2; // Scaled [0 ... 1000]

        // 5. Interpolate relative to current state using the smooth progression map
        if (ctx.isLightOut)
        {
            // Fade Out: Ease down from currentValue to 0
            uint32_t delta = ctx.currentValue;
            uint32_t progressAmount = (sCurveProgress * delta) / 1000;
            return (progressAmount >= delta) ? 0 : (delta - progressAmount);
        }
        else
        {
            // Fade In: Ease up from currentValue to maxBrightness
            uint32_t delta = ctx.maxBrightness - ctx.currentValue;
            uint32_t progressAmount = (sCurveProgress * delta) / 1000;
            return ctx.currentValue + progressAmount;
        }
    }

}

#define BREATH_MIN_BRIGHTNESS 13
#define BREATH_CYCLE_DURATION 7000
#define BREATH_FADE_OUT_MS 60

namespace Effects
{
    PWM_INT cozyBreathing(Effect::Context &ctx)
    {
        if (ctx.stepIdx != 0 && ctx.stepIdx != 12)
            return 0;

        const uint8_t BREATH_MAX_BRIGHTNESS = BREATH_MIN_BRIGHTNESS + 7;

        const uint32_t DYNAMIC_RANGE = BREATH_MAX_BRIGHTNESS - BREATH_MIN_BRIGHTNESS;

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

        // if (ctx.isLightOut && ctx.dt >= BREATH_FADE_OUT_MS && ctx.stepIdx == (TOTAL_STEPS - 1))
        // {
        //     ctx.effect->finish();
        // }

        return finalValue;
    }
}
