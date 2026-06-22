/**
 * @file SyncHAL_Avr.h
 * @brief Реализация SyncHAL для AVR с прямым управлением регистрами и прерываниями
 * @author Рейнгардт Михаил Петрович
 * @version 1.0.0
 * @date 2026
 */

#ifndef SYNC_HAL_AVR_H
#define SYNC_HAL_AVR_H

#include "SyncHAL.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

extern void (*intFunc[2])();

class SyncHAL_Avr : public SyncHAL {
public:
    SyncHAL_Avr() {}
    ~SyncHAL_Avr() {}

    void pinModeInputPullup(uint8_t pin) override {
        volatile uint8_t* ddr = _getDdr(pin);
        volatile uint8_t* port = _getPort(pin);
        uint8_t bit = _getBit(pin);
        if (ddr && port) {
            *ddr &= ~(1 << bit);
            *port |= (1 << bit);
        }
    }

    void attachInterruptExt(uint8_t pin, void (*isr)(), int mode) override {
        uint8_t intNum = 0xFF;
        if (pin == 16) intNum = 0;
        else if (pin == 17) intNum = 1;
        else return;
        if (intNum == 0) intFunc[0] = isr;
        else intFunc[1] = isr;
        uint8_t mask = 0;
        switch (mode) {
            case 1: mask = (1 << ISC01) | (1 << ISC00); break;
            case 2: mask = (1 << ISC01); break;
            case 3: mask = (1 << ISC00); break;
            default: mask = 0; break;
        }
        if (intNum == 0) {
            EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | mask;
            EIMSK |= (1 << INT0);
        } else {
            EICRA = (EICRA & ~((1 << ISC11) | (1 << ISC10))) | (mask << 2);
            EIMSK |= (1 << INT1);
        }
    }

    unsigned long millis() override {
        return ::millis();
    }

    void delayMs(uint32_t ms) override {
        ::delay(ms);
    }

    void disableInterrupts() override {
        cli();
    }

    void enableInterrupts() override {
        sei();
    }

private:
    volatile uint8_t* _getDdr(uint8_t pin) {
        if (pin >= 1 && pin <= 8)   return &DDRB;
        if (pin >= 14 && pin <= 21) return &DDRD;
        if (pin >= 22 && pin <= 29) return &DDRC;
        return nullptr;
    }

    volatile uint8_t* _getPort(uint8_t pin) {
        if (pin >= 1 && pin <= 8)   return &PORTB;
        if (pin >= 14 && pin <= 21) return &PORTD;
        if (pin >= 22 && pin <= 29) return &PORTC;
        return nullptr;
    }

    uint8_t _getBit(uint8_t pin) {
        if (pin >= 1 && pin <= 8)   return pin - 1;
        if (pin >= 14 && pin <= 21) return pin - 14;
        if (pin >= 22 && pin <= 29) return pin - 22;
        return 0;
    }
};

#endif