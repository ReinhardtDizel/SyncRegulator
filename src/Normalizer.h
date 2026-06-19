/**
 * @file Normalizer.h
 * @brief Приведение импульсов к общему масштабу (умножение на K_norm).
 *
 * P_norm = round(P_raw * K_norm), где K_norm хранится как целое ×1000.
 */
#ifndef NORMALIZER_H
#define NORMALIZER_H

#include <stdint.h>

class Normalizer {
private:
    int32_t _knorm_milli;   // K_norm * 1000 (например, 219 для 0.219)

public:
    Normalizer(int32_t knorm_milli = 219) : _knorm_milli(knorm_milli) {}

    /** Нормализовать сырые импульсы кирпласта */
    int32_t normalize(int32_t rawImpulses) const {
        if (_knorm_milli <= 0) return 0;
        return (rawImpulses * _knorm_milli + 500) / 1000;   // с округлением
    }

    /** Установить новый коэффициент нормализации (×1000) */
    void setKnorm(int32_t knorm_milli) { _knorm_milli = knorm_milli; }

    /** Получить текущий коэффициент (×1000) */
    int32_t getKnorm() const { return _knorm_milli; }
};

#endif