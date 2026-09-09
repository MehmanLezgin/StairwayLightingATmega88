#include "Effect.h"

Effect::Effect(StairPWM &pwm, uint8_t count)
    : pwm(pwm), count(count) {}

void Effect::start(EffectFunction fn)
{
    function = fn;
    startTime = millis();
    running = true;
}

void Effect::stop()
{
    running = false;

    for (uint8_t i = 0; i < count; i++)
        pwm.set(i, 0);
}

void Effect::update(int8_t direction, bool isLightUp)
{
    if (!running)
        return;

    uint32_t dt = millis() - startTime;

    Effect::Context ctx = {
        .effect = this,
        .dt = dt,
        .stepIdx = 0,
        .dir = direction,
        .isLightOut = isLightUp
    };


    for (uint8_t stepIdx = 0; stepIdx < count; stepIdx++)
    {
        ctx.stepIdx = stepIdx;
        pwm.set(stepIdx, function(ctx));
    }
}

bool Effect::isRunning() const
{
    return running;
}

void Effect::finish()
{
    running = false;
}