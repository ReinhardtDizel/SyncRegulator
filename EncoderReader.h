/**
 * @file EncoderReader.h
 * @brief Измерение скорости по импульсам инкрементального энкодера.
 *
 * Поддерживает до 8 независимых энкодеров одновременно.
 * Каждый экземпляр хранит свой интервал измерения — capture() можно
 * вызывать в любой момент, скорость будет пересчитана корректно.
 *
 * Зависимости:
 *   - RegulatorCore    (TimeProvider + MovingAverage)
 *   - SyncHAL.h        (GPIO + прерывания)
 */

#ifndef ENCODER_READER_H
#define ENCODER_READER_H

#include <stdint.h>
#include <RegulatorCore.h>
#include "SyncHAL.h"
#include "SyncHAL.h"

class EncoderReader {
public:
    /** Максимальное количество экземпляров */
    static const uint8_t MAX_ENCODERS = 8;

private:
    // --- Статическая диспетчеризация прерываний ---
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

    // --- Данные экземпляра ---
    SyncHAL&      _hal;
    TimeProvider& _time;
    uint8_t       _index;
    uint8_t       _pin;
    uint32_t      _intervalMs;

    volatile uint32_t _counter;   // меняется в ISR
    uint32_t    _snapshot;        // последнее зафиксированное значение
    uint32_t    _delta;           // приращение с прошлого capture()

    int32_t     _coeffMicro;      // A × 1_000_000  (для заданного интервала)
    int32_t     _offsetMilli;     // B × 1000

    MovingAverage<4> _filter;
    bool _begun;


public:
    /**
    void capture_isr() { _counter++; }
     * @brief Конструктор.
     *
     * @param hal         Абстракция GPIO и прерываний.
     * @param time        Единый источник времени и атомарных секций.
     * @param pin         Номер пина.
     * @param coeffMicro  Коэффициент A, умноженный на 1 000 000.
     *                    Должен быть рассчитан для переданного intervalMs.
     * @param offsetMilli Смещение B, умноженное на 1000.
     * @param intervalMs  Интервал измерения в миллисекундах (по умолчанию 500).
     */
    EncoderReader(SyncHAL& hal, TimeProvider& time, uint8_t pin,
                  int32_t coeffMicro, int32_t offsetMilli,
                  uint32_t intervalMs = 500)
        : _hal(hal), _time(time),
          _index(0xFF), _pin(pin), _intervalMs(intervalMs),
          _counter(0), _snapshot(0), _delta(0),
          _coeffMicro(coeffMicro), _offsetMilli(offsetMilli),
          _begun(false)
    {}

    /**
     * @brief Инициализировать пин и подключить прерывание.
     * @return true при успехе, false если превышено MAX_ENCODERS
     *         или intervalMs == 0.
     */
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

    /**
     * @brief Зафиксировать счётчик и вычислить скорость.
     *
     * Вызывается периодически (по внешнему таймеру).
     * Частота вызова = 1000 / intervalMs Гц.
     * После вызова getDelta() и getSpeed() возвращают актуальные данные.
     */
    void capture() {
        if (!_begun) return;

        uint32_t cur;
        _time.disableInterrupts();
        cur = _counter;
        _time.enableInterrupts();

        _delta = cur - _snapshot;
        _snapshot = cur;

        // speedMilli = delta × coeffMicro / 1_000_000 + offsetMilli
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

    // --- Геттеры ---

    /** Приращение импульсов с последнего capture() */
    uint32_t getDelta() const { return _delta; }

    /** Скорость в физических единицах (м/мин) */
    float getSpeed() const {
        return _filter.average() / 1000.0f;
    }

    /** Скорость в милли-единицах (×1000) для целочисленных расчётов */
    int32_t getSpeedMilli() const {
        return _filter.average();
    }

    /** Текущий интервал измерения */
    uint32_t getInterval() const { return _intervalMs; }

    // --- Управление ---

    /**
     * @brief Изменить интервал измерения.
     *
     * ВНИМАНИЕ: после изменения интервала коэффициент _coeffMicro
     * становится некорректным. Пользователь должен передать новый
     * коэффициент через setCoeff() или пересоздать объект.
     */
    void setInterval(uint32_t ms) {
        if (ms == 0) return;
        _intervalMs = ms;
    }

    /** Установить новый коэффициент (×1 000 000) */
    void setCoeff(int32_t coeffMicro) { _coeffMicro = coeffMicro; }

    /** Установить новое смещение (×1000) */
    void setOffset(int32_t offsetMilli) { _offsetMilli = offsetMilli; }

    /** Сбросить фильтр скорости */
    void resetFilter() { _filter.reset(); }

    /** Сбросить счётчик импульсов */
    void resetCounter() {
        _time.disableInterrupts();
        _counter  = 0;
        _snapshot = 0;
        _delta    = 0;
        _time.enableInterrupts();
    }

private:
    // Запрет копирования
    EncoderReader(const EncoderReader&);
    EncoderReader& operator=(const EncoderReader&);
};

#endif // ENCODER_READER_H