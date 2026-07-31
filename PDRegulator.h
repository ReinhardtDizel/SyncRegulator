/**
 * @file PDRegulator.h
 * @brief ПД-регулятор с мёртвой зоной и гистерезисом.
 *
 * Порядок вычислений:
 *   1. rawErr = Py_norm - Pk_raw
 *   2. filtErr = скользящее_среднее(rawErr)
 *   3. diff = filtErr - Pd           ← Pd вычитается ПОСЛЕ фильтра
 *   4. ΔF = clip(Kp × diff / 10, ±max_dF)
 *
 * Мёртвая зона с гистерезисом:
 *   - Вход в зону:  |filtErr| ≤ dead_in
 *   - Выход из зоны: |filtErr| ≥ dead_out
 *
 * Параметры передаются через структуру PDParams.
 * Реализует интерфейс SyncController.
 */

#ifndef PDREGULATOR_H
#define PDREGULATOR_H

#include <stdint.h>
#include <RegulatorCore.h>
#include "SyncController.h"

/**
 * @brief Параметры ПД-регулятора.
 */
struct PDParams {
    int32_t Kp;        ///< Коэффициент усиления (реальное усиление = Kp / 10)
    int32_t Pd;        ///< Желаемое опережение, импульсы каландра
    int32_t dead_in;   ///< Порог входа в мёртвую зону
    int32_t dead_out;  ///< Порог выхода из мёртвой зоны
    int32_t max_dF;    ///< Максимальное изменение частоты за шаг (0.01 Гц)
};

class PDRegulator : public SyncController {
private:
    bool _inDeadzone;
    int32_t _lastFilteredError;
    int32_t _lastDeltaF;
    MovingAverage<3> _errorFilter;

public:
    PDRegulator()
        : _inDeadzone(false)
        , _lastFilteredError(0)
        , _lastDeltaF(0)
    {}

    /**
     * @brief Рассчитать управляющее воздействие.
     *
     * @param py_norm  Нормализованные импульсы кирпласта.
     * @param pk_raw   Сырые импульсы каландра.
     * @param params   Указатель на PDParams.
     * @return deltaF в единицах 0.01 Гц.
     */
    int16_t compute(int32_t py_norm, int32_t pk_raw,
                    const void* params) override
    {
        const PDParams* p = static_cast<const PDParams*>(params);
        if (!p) return 0;

        // 1. Ошибка без Pd
        int32_t rawErr = py_norm - pk_raw;

        // 2. Фильтрация ошибки
        _errorFilter.add(rawErr);
        int32_t filtErr = _errorFilter.average();
        _lastFilteredError = filtErr;

        // 3. Вычитаем Pd ПОСЛЕ фильтра
        int32_t diff = filtErr - p->Pd;

        // 4. Мёртвая зона с гистерезисом
        int32_t absErr = (filtErr < 0) ? -filtErr : filtErr;

        if (_inDeadzone) {
            if (absErr <= p->dead_out) {
                _lastDeltaF = 0;
                return 0;
            }
            _inDeadzone = false;
        } else {
            if (absErr <= p->dead_in) {
                _inDeadzone = true;
                _lastDeltaF = 0;
                return 0;
            }
        }

        // 5. Расчёт ΔF
        int32_t delta = (p->Kp * diff) / 10;

        // 6. Ограничение
        if (delta > p->max_dF)  delta = p->max_dF;
        if (delta < -p->max_dF) delta = -p->max_dF;

        _lastDeltaF = delta;
        return (int16_t)delta;
    }

    /** Сбросить состояние: мёртвая зона, фильтр, последняя ΔF */
    void reset() {
        _inDeadzone = false;
        _lastFilteredError = 0;
        _lastDeltaF = 0;
        _errorFilter.reset();
    }

    /** Находится ли регулятор в мёртвой зоне */
    bool inDeadzone() const { return _inDeadzone; }

    /** Последняя фильтрованная ошибка (для диагностики) */
    int32_t getFilteredError() const { return _lastFilteredError; }

    /** Последнее рассчитанное ΔF (для диагностики) */
    int32_t getLastDeltaF() const { return _lastDeltaF; }
};

#endif // PDREGULATOR_H