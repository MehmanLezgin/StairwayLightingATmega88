#pragma once

#include "../Effect.h"

namespace Effects
{
    PWM_INT fade1(Effect::Context &ctx)
    {
        const uint8_t TOTAL_STEPS = 13;
        const uint16_t FADE_DURATION = 3000;
        const uint16_t STEP_DELAY = 500;
        const uint32_t TOTAL_EFFECT_DURATION = ((TOTAL_STEPS - 1) * STEP_DELAY) + FADE_DURATION;

        if (ctx.stepIdx >= TOTAL_STEPS)
        {
            if (ctx.dt >= TOTAL_EFFECT_DURATION && ctx.stepIdx == 15)
            {
                ctx.effect->finish();
            }
            return 0;
        }

        uint8_t physicalStep = ctx.stepIdx;
        if (ctx.dir == -1)
        {
            physicalStep = TOTAL_STEPS - 1 - ctx.stepIdx;
        }

        uint32_t stepStartTime = physicalStep * STEP_DELAY;
        PWM_INT finalValue = 0;

        if (ctx.dt < stepStartTime)
        {
            finalValue = ctx.currentValue;
        }
        else if (ctx.dt >= stepStartTime && (ctx.dt - stepStartTime) < FADE_DURATION)
        {
            uint32_t localDt = ctx.dt - stepStartTime;
            
            if (ctx.isLightOut)
            {
                PWM_INT currentFadingStep = ((uint32_t)localDt * ctx.maxBrightness) / FADE_DURATION;
                if (currentFadingStep >= ctx.maxBrightness) finalValue = 0;
                else finalValue = ctx.maxBrightness - currentFadingStep;
            }
            else
            {
                PWM_INT startBrightness = ctx.currentValue;
                if (startBrightness >= ctx.maxBrightness)
                {
                    finalValue = ctx.maxBrightness;
                }
                else
                {
                    PWM_INT remainingRange = ctx.maxBrightness - startBrightness;
                    finalValue = startBrightness + (((uint32_t)localDt * remainingRange) / FADE_DURATION);
                }
            }
        }
        else
        {
            finalValue = ctx.isLightOut ? 0 : ctx.maxBrightness;
        }

        if (ctx.dt >= TOTAL_EFFECT_DURATION && ctx.stepIdx == 15)
        {
            ctx.effect->finish();
        }

        return finalValue;
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
