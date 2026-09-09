#pragma once

#include <Arduino.h>
#include <stdint.h>

class AsyncUltrasonic
{
public:
    enum State : uint8_t
    {
        IDLE,
        TRIGGERING,
        WAITING_FOR_HIGH,
        MEASURING
    };

    AsyncUltrasonic(
        volatile uint8_t& ddr,
        volatile uint8_t& port,
        volatile uint8_t& pinReg,
        uint8_t pinBit
    );

    void begin();
    bool trigger();
    void update();
    void handleInterrupt();

    uint16_t getDistance() const;
    bool isReady() const;
    State getState() const;

private:
    volatile uint8_t& _ddr;
    volatile uint8_t& _port;
    volatile uint8_t& _pinReg;

    const uint8_t _mask;

    volatile State _state = IDLE;

    uint32_t _triggerTime;
    volatile uint32_t _echoStart;

    volatile uint16_t _distance;

    uint32_t _lastActionTime;
};