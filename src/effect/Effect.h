#pragma once

#include "StairPWM.h"

static const uint8_t fade[] = {
    255, 220, 180, 140, 100, 65, 35, 15, 0};

// static uint8_t lerp(uint8_t a, uint8_t b, uint8_t amount);
// static uint8_t smooth(uint8_t x);
// static uint8_t triangle(uint8_t x);
// uint8_t sequential(uint8_t step, uint32_t t);

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



class Effect
{
public:
    enum Result : uint8_t {
        RUNNING, FINISHED
    };

    enum LightAction : uint8_t {
        LIGHT_UP, LIGHT_OFF
    };

    struct Context {
        Effect* effect;
        uint32_t dt;
        uint8_t stepIdx;
        int8_t dir;
        bool isLightOut;
    };

    using EffectFunction = uint8_t (*)(Effect::Context& ctx);

private:
    StairPWM &pwm;
    EffectFunction function;

    uint8_t count;
    uint32_t startTime = 0;
    bool running = false;

public:
    Effect(StairPWM &pwm, uint8_t count);
    void start(EffectFunction fn);
    void stop();
    void update(int8_t direction, bool isLightUp);
    bool isRunning() const;
    void finish();
};