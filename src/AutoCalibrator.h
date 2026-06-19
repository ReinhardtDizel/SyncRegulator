/**
 * @file AutoCalibrator.h
 * @brief Автоматическая калибровка коэффициента нормализации.
 *
 * Собирает заданное количество выборок сырых импульсов и вычисляет
 * новое значение K_norm, обновляя объект Normalizer.
 */
#ifndef AUTOCALIBRATOR_H
#define AUTOCALIBRATOR_H

#include <stdint.h>
#include "Normalizer.h"

class AutoCalibrator {
private:
    int32_t   _sumPy, _sumPk;
    int       _sampleCount;
    bool      _active;
    Normalizer& _normalizer;
    const int MAX_SAMPLES = 20;

public:
    AutoCalibrator(Normalizer& normalizer)
        : _sumPy(0), _sumPk(0), _sampleCount(0), _active(false), _normalizer(normalizer) {}

    /** Запустить сбор данных */
    void start() {
        _active = true;
        _sampleCount = 0;
        _sumPy = 0;
        _sumPk = 0;
    }

    /** Добавить пару сырых импульсов (Py, Pk) */
    void addSample(int32_t py, int32_t pk) {
        if (!_active) return;
        _sumPy += py;
        _sumPk += pk;
        _sampleCount++;

        if (_sampleCount >= MAX_SAMPLES) {
            finish();
        }
    }

    /** Прогресс калибровки в процентах (0..100) */
    int progress() const {
        return (_sampleCount * 100) / MAX_SAMPLES;
    }

    /** Активна ли калибровка */
    bool isActive() const { return _active; }

private:
    void finish() {
        if (_sampleCount == 0) {
            _active = false;
            return;
        }
        int32_t avgPy = _sumPy / _sampleCount;
        int32_t avgPk = _sumPk / _sampleCount;

        // Защита от некорректных данных
        if (avgPk == 0 || avgPy < 5 || avgPk < 5) {
            _active = false;
            return;
        }

        int32_t knorm = (avgPy * 1000) / avgPk;
        if (knorm < 10 || knorm > 10000) {
            _active = false;
            return;
        }

        _normalizer.setKnorm(knorm);
        _active = false;
    }
};

#endif