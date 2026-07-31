/**
 * @file MovingAverage.h
 * @brief Шаблонный фильтр скользящего среднего.
 * 
 * Хранит последние N значений и вычисляет среднее арифметическое.
 * Используется для сглаживания скорости и ошибки.
 */

#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <stdint.h>

template<int N>
class MovingAverage {
private:
    int32_t _buffer[N];
    int     _index;
    int     _count;
    int32_t _sum;

public:
    MovingAverage() : _index(0), _count(0), _sum(0) {
        for (int i = 0; i < N; i++) _buffer[i] = 0;
    }

    /** Добавить новое значение */
    void add(int32_t value) {
        _sum -= _buffer[_index];
        _buffer[_index] = value;
        _sum += value;
        _index = (_index + 1) % N;
        if (_count < N) _count++;
    }

    /** Получить текущее среднее (или 0, если нет данных) */
    int32_t average() const {
        return (_count > 0) ? (_sum / _count) : 0;
    }

    /** Сбросить фильтр */
    void reset() {
        _index = 0;
        _count = 0;
        _sum   = 0;
        for (int i = 0; i < N; i++) _buffer[i] = 0;
    }
};

#endif // MOVING_AVERAGE_H