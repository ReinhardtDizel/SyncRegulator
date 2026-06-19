/**
 * @file SyncHAL.h
 * @brief Аппаратная абстракция для библиотеки SyncRegulator.
 *
 * Определяет интерфейс, необходимый для измерения импульсов энкодера.
 * Конкретная платформа (Arduino, AVR, STM32) должна реализовать этот класс.
 */
#ifndef SYNC_HAL_H
#define SYNC_HAL_H

#include <stdint.h>

class SyncHAL {
public:
    virtual ~SyncHAL() {}

    /** Настроить пин как вход с подтяжкой */
    virtual void pinModeInputPullup(uint8_t pin) = 0;

    /** Прикрепить прерывание к пину (вызывается по RISING фронту) */
    virtual void attachInterruptExt(uint8_t pin, void (*isr)(), int mode) = 0;

    /** Текущее время в миллисекундах */
    virtual unsigned long millis() = 0;

    /** Задержка в миллисекундах */
    virtual void delayMs(uint32_t ms) = 0;

    /** Запретить прерывания */
    virtual void disableInterrupts() = 0;

    /** Разрешить прерывания */
    virtual void enableInterrupts() = 0;

    /** Значение фронта RISING (для attachInterrupt) */
    static const int EDGE_RISING = 1;
};

#endif