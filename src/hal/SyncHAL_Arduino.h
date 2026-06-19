/**
 * @file SyncHAL_Arduino.h
 * @brief Реализация SyncHAL для платформы Arduino.
 */
#ifndef SYNC_HAL_ARDUINO_H
#define SYNC_HAL_ARDUINO_H

#include "SyncHAL.h"
#include <Arduino.h>   // Теперь можно, потому что имена больше не конфликтуют

class SyncHAL_Arduino : public SyncHAL {
public:
    void pinModeInputPullup(uint8_t pin) override {
        pinMode(pin, INPUT_PULLUP);
    }

    void attachInterruptExt(uint8_t pin, void (*isr)(), int mode) override {
        // Используем макрос digitalPinToInterrupt из Arduino (он безопасен)
        attachInterrupt(digitalPinToInterrupt(pin), isr, mode);
    }

    unsigned long millis() override {
        return ::millis();
    }

    void delayMs(uint32_t ms) override {
        ::delay(ms);
    }

    void disableInterrupts() override {
        noInterrupts();
    }

    void enableInterrupts() override {
        interrupts();
    }
};

#endif