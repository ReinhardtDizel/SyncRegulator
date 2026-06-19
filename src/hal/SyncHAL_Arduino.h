/**
 * @file SyncHAL_Arduino.h
 * @brief Реализация SyncHAL для платформы Arduino (без прямой зависимости от Arduino.h).
 */
#ifndef SYNC_HAL_ARDUINO_H
#define SYNC_HAL_ARDUINO_H

#include "SyncHAL.h"

// Функции Arduino core (определены в wiring.c, компоновщик найдёт их автоматически)
extern "C" {
    void pinMode(uint8_t, uint8_t);
    unsigned long millis();
    void delay(unsigned long);
    void noInterrupts();
    void interrupts();
    int digitalPinToInterrupt(uint8_t pin);
    void attachInterrupt(uint8_t, void (*)(), int);
}

// Константа INPUT_PULLUP для AVR (совпадает со значением в Arduino.h)
#ifndef INPUT_PULLUP
    #define INPUT_PULLUP 2
#endif

class SyncHAL_Arduino : public SyncHAL {
public:
    void pinModeInputPullup(uint8_t pin) override {
        pinMode(pin, INPUT_PULLUP);
    }

    void attachInterrupt(uint8_t pin, void (*isr)(), int mode) override {
        ::attachInterrupt(digitalPinToInterrupt(pin), isr, mode);
    }

    unsigned long millis() override {
        return ::millis();
    }

    void delay(uint32_t ms) override {
        ::delay(ms);
    }

    void noInterrupts() override {
        ::noInterrupts();
    }

    void interrupts() override {
        ::interrupts();
    }
};

#endif