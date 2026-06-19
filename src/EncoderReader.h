/**
 * @file EncoderReader.h
 * @brief Измерение импульсов энкодера и расчёт линейной скорости.
 *
 * Подсчитывает импульсы по прерываниям, периодически вычисляет
 * скорость (м/мин) со скользящим усреднением и хранит последнюю
 * дельту импульсов для использования в регуляторе.
 * Интервал измерения задаётся при создании объекта.
 */
#ifndef ENCODER_READER_H
#define ENCODER_READER_H

#include <stdint.h>
#include <Arduino.h>
#include "MovingAverage.h"

class EncoderReader {
private:
    volatile uint32_t _counter;      // счётчик импульсов (растёт в прерывании)
    uint32_t _lastCounter;           // предыдущий снимок счётчика
    uint32_t _lastTime;              // время последнего обновления (мс)
    uint32_t _delta;                 // импульсы за последний интервал
    int32_t  _coeffMicro;            // A * 1e6
    int32_t  _offsetMilli;           // B * 1000
    MovingAverage<4> _filter;        // фильтр скорости (окно 4)
    uint8_t  _pin;                   // номер пина энкодера
    const uint32_t _intervalMs;      // интервал измерения (мс)

public:
    /**
     * @brief Конструктор.
     * @param pin          Номер пина энкодера.
     * @param coeffMicro   Коэффициент A (импульсы → м/мин), умноженный на 1 000 000.
     * @param offsetMilli  Смещение B (м/мин), умноженное на 1000.
     * @param intervalMs   Интервал обновления в миллисекундах (по умолчанию 500).
     */
    EncoderReader(uint8_t pin, int32_t coeffMicro, int32_t offsetMilli,
                  uint32_t intervalMs = 500)
        : _counter(0), _lastCounter(0), _lastTime(0), _delta(0),
          _coeffMicro(coeffMicro), _offsetMilli(offsetMilli),
          _pin(pin), _intervalMs(intervalMs) {}

    /** Инициализировать пин, сбросить счётчик, повесить прерывание */
    void begin() {
        pinMode(_pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(_pin),
            []() { instance() ? instance()->_counter++ : (void)0; }, RISING);
        _lastTime = millis();
        _lastCounter = 0;
        noInterrupts();
        _counter = 0;
        interrupts();
        instance() = this;
    }

    /**
     * @brief Обновить данные (вызывать в цикле).
     * @return true, если прошёл заданный интервал и данные обновлены.
     */
    bool update() {
        unsigned long now = millis();
        if (now - _lastTime < _intervalMs) return false;

        uint32_t cur;
        noInterrupts();
        cur = _counter;
        interrupts();

        _delta = cur - _lastCounter;
        _lastCounter = cur;
        _lastTime = now;

        int32_t speedMilli = (int32_t)((_delta * _coeffMicro) / 1000000) + _offsetMilli;
        _filter.add(speedMilli);
        return true;
    }

    /** Текущая линейная скорость (м/мин) */
    float getSpeed() const { return _filter.average() / 1000.0f; }

    /** Сырые импульсы за последний завершённый интервал */
    uint32_t getRawImpulses() const { return _delta; }

    /** Сбросить фильтр скорости */
    void resetFilter() { _filter.reset(); }

private:
    static EncoderReader*& instance() {
        static EncoderReader* ptr = nullptr;
        return ptr;
    }
};

#endif