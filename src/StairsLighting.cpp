#include "StairsLighting.h"
#include "effect/func/fade.h"
#include "StairPWM.h"

#define EFFECT Effects::cozyBreathing


StairsLighting::StairsLighting(
    AsyncUltrasonic &lower,
    AsyncUltrasonic &upper)
    : _sonarLower(lower),
      _sonarUpper(upper),
      _effect(STAIRS_PWM_CHANNELS)
{
}

void StairsLighting::begin()
{
    _sonarLower.begin();
    _sonarUpper.begin();

    setStandbyLight(true);

    lightOn(DIRECTION_UP);
}

uint8_t j = 0;
uint32_t last = 0;

void StairsLighting::update()
{
    uint32_t now = millis();

    // if (now - last > 30)
    // {
    //     last = now;
    //     for (uint8_t i = 0; i < 16; i++)
    //     {
    //         StairPWM::getInstance().set(i, j);
    //     }
    //     j++;
    // }
    _sonarLower.update();
    _sonarUpper.update();
    const bool isLightOut = state == LIGHT_EFFECT_OUT;
    const int8_t dir = direction == DIRECTION_DOWN ? -1 : dir == DIRECTION_UP ? +1
                                                                              : 0;
    _effect.update(dir, isLightOut);
    updateSensors();

    uint32_t stateChangetimeDiff = now - _lastLightReadyTime;

    if (state == LIGHT_ON && stateChangetimeDiff > LIGHT_STAY_TIME_MS)
    {
        lightOff();
        return;
    }

    const Direction direction = readDirection();

    if (state == LIGHT_OFF && direction != DIRECTION_NONE) // && stateChangetimeDiff >= LIGHT_UP_INTERVAL_MS)
    {
        lightOn(direction);
        return;
    }

    if (!_effect.isRunning())
    {
        state = state == LIGHT_EFFECT_IN ? LIGHT_ON : LIGHT_OFF;
        _lastLightReadyTime = now;
    }
}

void StairsLighting::updateSensors()
{
    const uint32_t now = millis();

    if (now - _lastMeasureTime < MEASURE_INTERVAL_MS)
        return;

    AsyncUltrasonic &sensor =
        _sensorIndex ? _sonarLower : _sonarUpper;

    if (!sensor.isReady())
        return;

    if (sensor.trigger())
    {
        _lastMeasureTime = now;
        _sensorIndex ^= 1;
    }
}

StairsLighting::Direction StairsLighting::readDirection()
{
    const uint16_t lower = _sonarLower.getDistance();

    if (_sonarLower.isReady() &&
        lower &&
        lower < DETECTION_THRESHOLD_CM)
    {
        return DIRECTION_UP;
    }

    const uint16_t upper = _sonarUpper.getDistance();

    if (_sonarUpper.isReady() &&
        upper &&
        upper < DETECTION_THRESHOLD_CM)
    {
        return DIRECTION_DOWN;
    }

    return DIRECTION_NONE;
}

void StairsLighting::setStandbyLight(bool enabled)
{
    _standbyLightEnabled = enabled;
}

void StairsLighting::setStandbyLightBrightness(uint8_t brightness)
{
    _standbyLightBrightness = brightness;
}

bool StairsLighting::isDark()
{
    return true;
}

void StairsLighting::lightOff()
{
    direction = DIRECTION_NONE;
    state = LIGHT_EFFECT_OUT;
    _effect.start(EFFECT);
}

void StairsLighting::lightOn(Direction dir)
{
    direction = dir;
    state = LIGHT_EFFECT_IN;
    _effect.start(EFFECT);
}