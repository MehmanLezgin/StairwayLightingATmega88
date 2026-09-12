#include "StairsLighting.h"
#include "effect/func/fade.h"
#include "StairPWM.h"

#define EFFECT Effects::fade1

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
    _sonarLower.begin();
    _sonarUpper.begin();
    setStandbyLight(true);
}

void StairsLighting::update()
{
    _keyboard.update();

    uint32_t now = millis();

    const bool isLightOut = state == LIGHT_EFFECT_OUT;
    const int8_t dir = direction == DIRECTION_DOWN ? -1 : direction == DIRECTION_UP ? +1
                                                                                    : 0;

    bool isDark = _ldrSensor.isDark();

    _effect.update(dir, isLightOut);
    uint32_t stateChangetimeDiff = now - _lastLightReadyTime;

    if (state == LIGHT_OFF && !_effect.isRunning() && stateChangetimeDiff > 500)
    {
        static uint32_t lastStandbyTime = 0;
        static uint16_t currentStandbyPwm = 0;

        uint16_t targetStandbyPwm = (isDark && _standbyLightEnabled) ? _standbyLightBrightness : 0;

        if (currentStandbyPwm != targetStandbyPwm)
        {
            if (now - lastStandbyTime >= 100)
            {
                lastStandbyTime = now;
                if (currentStandbyPwm < targetStandbyPwm)
                    currentStandbyPwm++;
                else
                    currentStandbyPwm--;
            }
        }

        for (uint8_t i = 0; i < STAIRS_PWM_CHANNELS; i++)
        {
            if (i == 0 || i == 12)
                StairPWM::getInstance().set(i, currentStandbyPwm);
            else
                StairPWM::getInstance().set(i, 0);
        }
    }

    if (isDark && !_effect.isRunning())
    {
        _sonarLower.update();
        _sonarUpper.update();
        updateSensors();
    }

    if (state == LIGHT_ON)
    {
        if (stateChangetimeDiff > LIGHT_STAY_TIME_MS)
        {
            lightOff();
            return;
        }
    }

    const Direction currentDir = readDirection();

    if (currentDir != DIRECTION_NONE)
    {
        if (state == LIGHT_ON)
        {
            if (stateChangetimeDiff > (LIGHT_STAY_TIME_MS / 2))
            {
                _lastLightReadyTime = now;
            }
        }
        else if (state == LIGHT_OFF)
        {
            if (isDark && stateChangetimeDiff >= LIGHT_UP_INTERVAL_MS)
            {
                lightOn(currentDir);
                return;
            }
        }
    }

    if (!_effect.isRunning())
    {
        if (state == LIGHT_EFFECT_IN)
        {
            state = LIGHT_ON;
            _lastLightReadyTime = now;
        }
        else if (state == LIGHT_EFFECT_OUT)
        {
            state = LIGHT_OFF;
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
    if (!_ldrSensor.isDark() || _effect.isRunning())
        return DIRECTION_NONE;

    if (_keyboard.isClicked(KEY_A))
        return DIRECTION_UP;
    else if (_keyboard.isClicked(KEY_B))
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
    state = LIGHT_EFFECT_OUT;
    _effect.start(EFFECT);
}

void StairsLighting::lightOn(Direction dir)
{
    direction = dir;
    state = LIGHT_EFFECT_IN;
    _effect.start(EFFECT);
}
