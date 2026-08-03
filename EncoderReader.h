#ifndef ENCODER_READER_H
#define ENCODER_READER_H

#include <stdint.h>
#include <RegulatorCore.h>
#include "SyncHAL.h"

class EncoderReader {
public:
    static const uint8_t MAX_ENCODERS = 8;

    static EncoderReader* _instances[MAX_ENCODERS];
    static uint8_t        _nextIndex;

    static void isr0() { if (_instances[0]) _instances[0]->capture_isr(); }
    static void isr1() { if (_instances[1]) _instances[1]->capture_isr(); }
    static void isr2() { if (_instances[2]) _instances[2]->capture_isr(); }
    static void isr3() { if (_instances[3]) _instances[3]->capture_isr(); }
    static void isr4() { if (_instances[4]) _instances[4]->capture_isr(); }
    static void isr5() { if (_instances[5]) _instances[5]->capture_isr(); }
    static void isr6() { if (_instances[6]) _instances[6]->capture_isr(); }
    static void isr7() { if (_instances[7]) _instances[7]->capture_isr(); }

    typedef void (*IsrFunc)();
    static const IsrFunc _dispatchers[MAX_ENCODERS];

private:
    SyncHAL&      _hal;
    TimeProvider& _time;
    uint8_t       _index;
    uint8_t       _pin;
    uint32_t      _intervalMs;

    volatile uint32_t _counter;
    uint32_t    _snapshot;
    uint32_t    _delta;

    int32_t     _coeffMicro;
    int32_t     _offsetMilli;

    MovingAverage<4> _filter;
    bool _begun;

public:
    void capture_isr() { _counter++; }

    EncoderReader(SyncHAL& hal, TimeProvider& time, uint8_t pin,
                  int32_t coeffMicro, int32_t offsetMilli,
                  uint32_t intervalMs = 500)
        : _hal(hal), _time(time),
          _index(0xFF), _pin(pin), _intervalMs(intervalMs),
          _counter(0), _snapshot(0), _delta(0),
          _coeffMicro(coeffMicro), _offsetMilli(offsetMilli),
          _begun(false)
    {}

    bool begin() {
        if (_begun) return true;
        if (_intervalMs == 0) return false;
        if (_nextIndex >= MAX_ENCODERS) return false;

        _index = _nextIndex++;
        _instances[_index] = this;

        _hal.pinModeInputPullup(_pin);
        _hal.attachInterruptExt(_pin, _dispatchers[_index], SyncHAL::EDGE_RISING);

        _counter  = 0;
        _snapshot = 0;
        _delta    = 0;
        _filter.reset();
        _begun = true;
        return true;
    }

    void capture() {
        if (!_begun) return;

        uint32_t cur;
        _time.disableInterrupts();
        cur = _counter;
        _time.enableInterrupts();

        _delta = cur - _snapshot;
        _snapshot = cur;

        int32_t speedMilli;
        if (_coeffMicro >= 0) {
            speedMilli = (int32_t)((_delta * (uint32_t)_coeffMicro) / 1000000UL)
                       + _offsetMilli;
        } else {
            speedMilli = (int32_t)(((int64_t)_delta * _coeffMicro) / 1000000LL)
                       + _offsetMilli;
        }
        _filter.add(speedMilli);
    }

    uint32_t getDelta() const { return _delta; }

    float getSpeed() const {
        return _filter.average() / 1000.0f;
    }

    int32_t getSpeedMilli() const {
        return _filter.average();
    }

    uint32_t getInterval() const { return _intervalMs; }

    void setInterval(uint32_t ms) {
        if (ms == 0) return;
        _intervalMs = ms;
    }

    void setCoeff(int32_t coeffMicro) { _coeffMicro = coeffMicro; }
    void setOffset(int32_t offsetMilli) { _offsetMilli = offsetMilli; }
    void resetFilter() { _filter.reset(); }

    void resetCounter() {
        _time.disableInterrupts();
        _counter  = 0;
        _snapshot = 0;
        _delta    = 0;
        _time.enableInterrupts();
    }

private:
    EncoderReader(const EncoderReader&);
    EncoderReader& operator=(const EncoderReader&);
};

#endif
