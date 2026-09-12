#pragma once

#include <stdint.h>

#include "StairPwm.h"
#include "effect/Effect.h"
#include "hardware/AsyncUltrasonic.h"
#include "hardware/Keyboard.h"
#include "hardware/LdrSensor.h"

class StairsLighting
{
public:
    enum Direction : uint8_t
    {
        DIRECTION_NONE,
        DIRECTION_UP,
        DIRECTION_DOWN
    };

private:
    enum State : uint8_t
    {
        LIGHT_ON,
        LIGHT_OFF,
        LIGHT_EFFECT_IN,
        LIGHT_EFFECT_OUT
    };

    AsyncUltrasonic &_sonarLower;
    AsyncUltrasonic &_sonarUpper;
    Keyboard &_keyboard;
    LdrSensor &_ldrSensor;

    Effect _effect;
    Direction direction = DIRECTION_NONE;
    State state = LIGHT_OFF;

    bool _standbyLightEnabled = false;
    uint8_t _standbyLightBrightness = 1;

    uint32_t _lastLightReadyTime = 0;
    uint32_t _lastMeasureTime = 0;
    uint8_t _sensorIndex = 0;

    static constexpr uint32_t MEASURE_INTERVAL_MS = 100;
    static constexpr uint16_t DETECTION_THRESHOLD_CM = 45;
    static constexpr uint16_t LIGHT_STAY_TIME_MS = 10000;
    static constexpr uint16_t LIGHT_UP_INTERVAL_MS = 3000;

    void updateSensors();

public:
    StairsLighting(
        AsyncUltrasonic &lower,
        AsyncUltrasonic &upper,
        Keyboard &keyboard,
        LdrSensor &ldrSensor
    );

    void begin();
    void update();

    void setStandbyLight(bool enabled);
    void setStandbyLightBrightness(uint8_t brightness);

    Direction readDirection();

    void lightOff();
    void lightOn(Direction dir);
};