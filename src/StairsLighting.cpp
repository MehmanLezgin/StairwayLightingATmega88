#include "StairsLighting.h"
#include "effect/func/fade.h"
#include "StairPWM.h"

#define FADE_OUT_EFFECT Effects::simpleFadeOut
#define EFFECT Effects::fade1
#define STANDBY_EFFECT Effects::cozyBreathing

StairsLighting::StairsLighting(
    AsyncUltrasonic &lower,
    AsyncUltrasonic &upper,
    Keyboard &keyboard,
    LdrSensor &ldrSensor)
    : _sonarLower(lower),
      _sonarUpper(upper),
      _keyboard(keyboard),
      _ldrSensor(ldrSensor),
      _effect(STAIRS_PWM_CHANNELS)
{
}

void StairsLighting::begin()
{
    StairPWM::getInstance().begin(45);
    _sonarLower.begin();
    _sonarUpper.begin();
    setStandbyLight(true);
}

void StairsLighting::update()
{
    _keyboard.update();

    uint32_t now = millis();

    const bool isLightOut = state == STATE_LIGHT_EFFECT_OUT;
    const int8_t dir = direction == DIRECTION_DOWN ? -1 : direction == DIRECTION_UP ? +1
                                                                                    : 0;

    bool isDark = _ldrSensor.isDark();

    _effect.update(dir, isLightOut);

    uint32_t stateChangetimeDiff = now - _lastLightReadyTime;

    if (stateChangetimeDiff > LIGHT_UP_INTERVAL_MS >> 1)
    {
        if (state == STATE_LIGHT_OFF && isDark)
        {
            state = STATE_LIGHT_STANDBY;
            _lastLightReadyTime = now;
            _effect.start(STANDBY_EFFECT);
        }
        else if (state == STATE_LIGHT_STANDBY && !isDark)
        {
            state = STATE_LIGHT_OFF;
            _lastLightReadyTime = now;
            _effect.start(FADE_OUT_EFFECT);
        }
    }

    if (isDark && !isMainEffectRunning())
    {
        _sonarLower.update();
        _sonarUpper.update();
        updateSensors();
    }
    const Direction currentDir = readDirection();

    if (currentDir == DIRECTION_NONE)
    {
        if (state == STATE_LIGHT_ON && stateChangetimeDiff > LIGHT_STAY_TIME_MS)
        {
            lightOff();
            return;
        }
    }
    else if (isDark)
    {
        if (state == STATE_LIGHT_ON && stateChangetimeDiff > LIGHT_STAY_TIME_MS / 2)
        {
            _lastLightReadyTime = now;
        }
        else if (isStandbyOrOff() && stateChangetimeDiff >= LIGHT_UP_INTERVAL_MS)
        {
            lightOn(currentDir);
            return;
        }
    }

    if (!isMainEffectRunning())
    {
        if (state == STATE_LIGHT_EFFECT_IN)
        {
            state = STATE_LIGHT_ON;
            _lastLightReadyTime = now;
        }
        else if (state == STATE_LIGHT_EFFECT_OUT)
        {
            state = STATE_LIGHT_OFF;
            _lastLightReadyTime = now;
            direction = DIRECTION_NONE;
        }
    }
}

void StairsLighting::updateSensors()
{
    const uint32_t now = millis();

    if (now - _lastMeasureTime < MEASURE_INTERVAL_MS)
        return;

    AsyncUltrasonic &sensor = _sensorIndex ? _sonarLower : _sonarUpper;

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
    if (!_ldrSensor.isDark() || isMainEffectRunning())
        return DIRECTION_NONE;

    if (_keyboard.isPressed(KEY_A))
        return DIRECTION_UP;
    else if (_keyboard.isPressed(KEY_B))
        return DIRECTION_DOWN;

    const uint16_t lower = _sonarLower.getDistance();

    if (_sonarLower.isReady() && lower && lower < DETECTION_THRESHOLD_CM)
    {
        return DIRECTION_UP;
    }

    const uint16_t upper = _sonarUpper.getDistance();

    if (_sonarUpper.isReady() && upper && upper < DETECTION_THRESHOLD_CM)
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

void StairsLighting::lightOff()
{
    state = STATE_LIGHT_EFFECT_OUT;
    _effect.start(EFFECT);
}

void StairsLighting::lightOn(Direction dir)
{
    direction = dir;
    state = STATE_LIGHT_EFFECT_IN;
    _effect.start(EFFECT);
}

bool StairsLighting::isStandbyEffectRunning()
{
    return _effect.isRunning() && state == STATE_LIGHT_STANDBY;
}

bool StairsLighting::isMainEffectRunning()
{
    return _effect.isRunning() && !isStandbyOrOff();
}

bool StairsLighting::isStandbyOrOff()
{
    return state == STATE_LIGHT_STANDBY || state == STATE_LIGHT_OFF;
}