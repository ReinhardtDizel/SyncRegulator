#ifndef ENCODER_READER_H
#define ENCODER_READER_H

#include <stdint.h>
#include "MovingAverage.h"
#include "hal/SyncHAL.h"

class EncoderReader {
private:
    volatile uint32_t _counter;
    uint32_t _lastCounter;
    uint32_t _lastTime;
    uint32_t _delta;
    int32_t  _coeffMicro;
    int32_t  _offsetMilli;
    MovingAverage<4> _filter;
    uint8_t  _pin;
    const uint32_t _intervalMs;
    SyncHAL& _hal;

public:
    EncoderReader(SyncHAL& hal, uint8_t pin, int32_t coeffMicro, int32_t offsetMilli,
                  uint32_t intervalMs = 500)
        : _counter(0), _lastCounter(0), _lastTime(0), _delta(0),
          _coeffMicro(coeffMicro), _offsetMilli(offsetMilli),
          _pin(pin), _intervalMs(intervalMs), _hal(hal) {}

    void begin() {
        _hal.pinModeInputPullup(_pin);
        _hal.attachInterruptExt(_pin, []() {
            if (instance()) instance()->_counter++;
        }, SyncHAL::EDGE_RISING);
        _lastTime = _hal.millis();
        _lastCounter = 0;
        _hal.disableInterrupts();
        _counter = 0;
        _hal.enableInterrupts();
        instance() = this;
    }

    bool update() {
        unsigned long now = _hal.millis();
        if (now - _lastTime < _intervalMs) return false;

        uint32_t cur;
        _hal.disableInterrupts();
        cur = _counter;
        _hal.enableInterrupts();

        _delta = cur - _lastCounter;
        _lastCounter = cur;
        _lastTime = now;

        int32_t speedMilli = (int32_t)((_delta * _coeffMicro) / 1000000) + _offsetMilli;
        _filter.add(speedMilli);
        return true;
    }

    float getSpeed() const { return _filter.average() / 1000.0f; }
    uint32_t getRawImpulses() const { return _delta; }
    void resetFilter() { _filter.reset(); }

private:
    static EncoderReader*& instance() {
        static EncoderReader* ptr = nullptr;
        return ptr;
    }
};

#endif