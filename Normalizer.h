/**
 * @file Normalizer.h
 * @brief Приведение импульсов к общему масштабу.
 *
 *   P_norm = round(P_raw × K_norm / 1000)
 *
 * K_norm хранится как целое ×1000 (например, 0.219 → 219).
 * Все величины беззнаковые, отрицательных импульсов не бывает.
 */

#ifndef NORMALIZER_H
#define NORMALIZER_H

#include <stdint.h>

class Normalizer {
private:
    int32_t _knorm_milli;   // K_norm × 1000

public:
    Normalizer(int32_t knorm_milli = 219)
        : _knorm_milli(knorm_milli)
    {}

    /**
     * @brief Нормализовать сырые импульсы.
     * @param rawImpulses Сырые импульсы кирпласта (uint32_t или int32_t ≥ 0).
     * @return Нормализованные импульсы в масштабе каландра.
     */
    int32_t normalize(int32_t rawImpulses) const {
        if (_knorm_milli <= 0 || rawImpulses < 0) return 0;
        return (int32_t)(((uint32_t)rawImpulses * (uint32_t)_knorm_milli + 500UL) / 1000UL);
    }

    void setKnorm(int32_t knorm_milli) { _knorm_milli = knorm_milli; }
    int32_t getKnorm() const { return _knorm_milli; }
    float getKnormFloat() const { return (float)_knorm_milli / 1000.0f; }
};

#endif