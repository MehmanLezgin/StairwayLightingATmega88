#include "AsyncUltrasonic.h"

AsyncUltrasonic::AsyncUltrasonic(
    volatile uint8_t &ddr,
    volatile uint8_t &port,
    volatile uint8_t &pinReg,
    uint8_t pinBit)
    : _ddr(ddr),
      _port(port),
      _pinReg(pinReg),
      _mask(1 << pinBit)
{
}

void AsyncUltrasonic::begin()
{
    _ddr |= _mask;
    _port &= ~_mask;

    _state = IDLE;
    _distance = 999;
}

bool AsyncUltrasonic::trigger()
{
    if (_state != IDLE)
        return false;

    _state = TRIGGERING;

    _triggerTime = micros();
    _lastActionTime = millis();

    // OUTPUT HIGH
    _ddr |= _mask;
    _port |= _mask;

    return true;
}

void AsyncUltrasonic::update()
{
    const uint32_t nowUs = micros();

    if (_state == TRIGGERING && nowUs - _triggerTime >= 10)
    {
        // LOW
        _port &= ~_mask;

        // Set pin as INPUT and activate INTERNAL PULL-UP
        _ddr &= ~_mask;  // Clear DDR bit to make it an Input
        _port |= _mask;  // Set PORT bit while DDR is 0 to enable the Internal Pull-Up

        _state = WAITING_FOR_HIGH;
    }

    if (_state >= WAITING_FOR_HIGH && millis() - _lastActionTime > 30)
    {
        _state = IDLE;
        _distance = 999;
    }
}


void AsyncUltrasonic::handleInterrupt()
{
    // Read the current live pin state
    const bool high = (_pinReg & _mask);

    if (_state == WAITING_FOR_HIGH)
    {
        // Since we are using an internal pull-up, the pin is naturally HIGH.
        // The sensor module pulls the line LOW during echo setup, then pushes it HIGH.
        if (high)
        {
            _echoStart = micros();
            _state = MEASURING;
        }
        return;
    }

    if (_state == MEASURING && !high)
    {
        const uint32_t duration = micros() - _echoStart;
        _distance = duration / 58;
        _state = IDLE;
    }
}


uint16_t AsyncUltrasonic::getDistance() const
{
    return _distance;
}

bool AsyncUltrasonic::isReady() const
{
    return _state == IDLE;
}

AsyncUltrasonic::State AsyncUltrasonic::getState() const
{
    return _state;
}