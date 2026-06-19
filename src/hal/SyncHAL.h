#ifndef SYNC_HAL_H
#define SYNC_HAL_H

#include <stdint.h>

class SyncHAL {
public:
    virtual ~SyncHAL() {}

    virtual void pinModeInputPullup(uint8_t pin) = 0;
    virtual void attachInterrupt(uint8_t pin, void (*isr)(), int mode) = 0;
    virtual unsigned long millis() = 0;
    virtual void delay(uint32_t ms) = 0;

    /** Запретить прерывания */
    virtual void noInterrupts() = 0;
    /** Разрешить прерывания */
    virtual void interrupts() = 0;

    /** Значение фронта RISING (для attachInterrupt) */
    static const int RISING = 1;
};

#endif