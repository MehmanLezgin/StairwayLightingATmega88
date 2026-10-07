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
    const uint32_t now = millis();
    const bool isDark = _ldrSensor.isDark();

    _keyboard.update();
    updateEffects(now);
    updateDayNightState(now, isDark);
    updateUltrasonicSensors(isDark);

    const Direction dir = readDirection();

    if (handleLightState(now, dir, isDark))
        return;

    finishLightEffect(now);
}

void StairsLighting::updateEffects(uint32_t now)
{
    const bool isLightOut = state == State::LIGHT_EFFECT_OUT;

    const int8_t dir =
        direction == Direction::DOWN ? -1 : direction == Direction::UP ? 1
                                                                     : 0;

    _effect.update(dir, isLightOut);
}

void StairsLighting::updateDayNightState(
    uint32_t now,
    bool isDark)
{
    if (now - _lastLightReadyTime <= (LIGHT_UP_INTERVAL_MS >> 1))
        return;

    if (state == State::LIGHT_OFF && isDark)
        enterStandby(now);
    else if (state == State::LIGHT_STANDBY && !isDark)
        enterLightOff(now);
}

void StairsLighting::enterStandby(uint32_t now)
{
    state = State::LIGHT_STANDBY;
    _lastLightReadyTime = now;
    _effect.start(STANDBY_EFFECT);
}

void StairsLighting::enterLightOff(uint32_t now)
{
    state = State::LIGHT_OFF;
    _lastLightReadyTime = now;
    _effect.start(FADE_OUT_EFFECT);
}

void StairsLighting::updateUltrasonicSensors(bool isDark)
{
    if (!isDark || isMainEffectRunning())
        return;

    _sonarLower.update();
    _sonarUpper.update();
    triggerNextSensor();
}

void StairsLighting::triggerNextSensor()
{
    const uint32_t now = millis();

    if (now - _lastMeasureTime < MEASURE_INTERVAL_MS)
        return;

    AsyncUltrasonic& sensor = nextSensor();

    if (!sensor.isReady())
        return;

    if (!sensor.trigger())
        return;

    _lastMeasureTime = now;
    
    _nextSensor = _nextSensor == SensorPos::UPPER
        ? SensorPos::LOWER
        : SensorPos::UPPER;
}

AsyncUltrasonic& StairsLighting::nextSensor()
{
    return _nextSensor == SensorPos::UPPER
        ? _sonarUpper
        : _sonarLower;
}

bool StairsLighting::handleLightState(
    uint32_t now,
    Direction dir,
    bool isDark)
{
    const uint32_t stateTime = now - _lastLightReadyTime;

    if (dir == Direction::NONE)
    {
        if (state == State::LIGHT_ON &&
            stateTime > LIGHT_STAY_TIME_MS)
        {
            lightOff();
            return true;
        }

        return false;
    }

    if (!isDark)
        return false;

    if (state == State::LIGHT_ON &&
        stateTime > LIGHT_STAY_TIME_MS / 2)
    {
        _lastLightReadyTime = now;
    }
    else if (isStandbyOrOff() &&
             stateTime >= LIGHT_UP_INTERVAL_MS)
    {
        lightOn(dir);
        return true;
    }

    return false;
}

void StairsLighting::finishLightEffect(uint32_t now)
{
    if (isMainEffectRunning())
        return;

    if (state == State::LIGHT_EFFECT_IN)
    {
        state = State::LIGHT_ON;
        _lastLightReadyTime = now;
        return;
    }

    if (state == State::LIGHT_EFFECT_OUT)
    {
        state = State::LIGHT_OFF;
        _lastLightReadyTime = now;
        direction = Direction::NONE;
    }
}

StairsLighting::Direction StairsLighting::readDirection()
{
    if (!_ldrSensor.isDark() || isMainEffectRunning())
        return Direction::NONE;

    if (_keyboard.isPressed(KEY_A))
        return Direction::UP;
    else if (_keyboard.isPressed(KEY_B))
        return Direction::DOWN;

    const uint16_t lower = _sonarLower.getDistance();

    if (_sonarLower.isReady() && lower && lower < DETECTION_THRESHOLD_CM)
    {
        return Direction::UP;
    }

    const uint16_t upper = _sonarUpper.getDistance();

    if (_sonarUpper.isReady() && upper && upper < DETECTION_THRESHOLD_CM)
    {
        return Direction::DOWN;
    }

    return Direction::NONE;
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
    state = State::LIGHT_EFFECT_OUT;
    _effect.start(EFFECT);
}

void StairsLighting::lightOn(Direction dir)
{
    direction = dir;
    state = State::LIGHT_EFFECT_IN;
    _effect.start(EFFECT);
}

bool StairsLighting::isStandbyEffectRunning()
{
    return _effect.isRunning() && state == State::LIGHT_STANDBY;
}

bool StairsLighting::isMainEffectRunning()
{
    return _effect.isRunning() && !isStandbyOrOff();
}

bool StairsLighting::isStandbyOrOff()
{
    return state == State::LIGHT_STANDBY || state == State::LIGHT_OFF;
}
