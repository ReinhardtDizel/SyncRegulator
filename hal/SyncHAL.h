/**
 * @file SyncHAL.h
 * @brief Аппаратная абстракция GPIO и прерываний для энкодеров.
 *
 * Время и критические секции — через TimeProvider.
 */

#ifndef SYNC_HAL_H
#define SYNC_HAL_H

#include <stdint.h>

class SyncHAL {
public:
    virtual ~SyncHAL() {}

    virtual void pinModeInputPullup(uint8_t pin) = 0;
    virtual void attachInterruptExt(uint8_t pin, void (*isr)(), int mode) = 0;

    static const int EDGE_RISING = 1;
};

#endif