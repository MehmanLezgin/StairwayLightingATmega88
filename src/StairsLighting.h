#pragma once

#include <stdint.h>

#include "StairPwm.h"
#include "effect/Effect.h"
#include "hardware/AsyncUltrasonic.h"
#include "hardware/Keyboard.h"
#include "hardware/LdrSensor.h"

enum class SensorPos
{
    UPPER,
    LOWER
};

class StairsLighting
{
public:
    enum class Direction : uint8_t
    {
        NONE, UP, DOWN
    };

private:
    enum class State : uint8_t
    {
        LIGHT_ON,
        LIGHT_OFF,
        LIGHT_EFFECT_IN,
        LIGHT_EFFECT_OUT,
        LIGHT_STANDBY
    };

    AsyncUltrasonic &_sonarLower;
    AsyncUltrasonic &_sonarUpper;
    Keyboard &_keyboard;
    LdrSensor &_ldrSensor;

    Effect _effect;
    Direction direction = Direction::NONE;
    State state = State::LIGHT_OFF;

    bool _standbyLightEnabled = false;
    uint8_t _standbyLightBrightness = 12;

    uint32_t _lastLightReadyTime = 0;
    uint32_t _lastMeasureTime = 0;
    SensorPos _nextSensor = SensorPos::UPPER;

    static constexpr uint32_t MEASURE_INTERVAL_MS = 100;
    static constexpr uint16_t DETECTION_THRESHOLD_CM = 60;
    static constexpr uint16_t LIGHT_STAY_TIME_MS = 15000;
    static constexpr uint16_t LIGHT_UP_INTERVAL_MS = 2000;

    void triggerNextSensor();

public:
    StairsLighting(
        AsyncUltrasonic &lower,
        AsyncUltrasonic &upper,
        Keyboard &keyboard,
        LdrSensor &ldrSensor);

    void begin();
    void update();

    void setStandbyLight(bool enabled);
    void setStandbyLightBrightness(uint8_t brightness);

    Direction readDirection();

    void lightOff();
    void lightOn(Direction dir);
    bool isStandbyEffectRunning();
    bool isMainEffectRunning();
    bool isStandbyOrOff();

    void enterStandby(uint32_t now);

    void enterLightOff(uint32_t now);

    void updateEffects(uint32_t now);

    void finishLightEffect(uint32_t now);

    void updateDayNightState(
        uint32_t now,
        bool isDark);

    void updateUltrasonicSensors(bool isDark);

    bool handleLightState(
        uint32_t now,
        Direction dir,
        bool isDark);

    AsyncUltrasonic& nextSensor();
};