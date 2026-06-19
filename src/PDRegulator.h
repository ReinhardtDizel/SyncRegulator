/**
 * @file PDRegulator.h
 * @brief ПД-регулятор с мёртвой зоной и гистерезисом.
 *
 * Реализует интерфейс SyncController.
 * Использует скользящее среднее для фильтрации ошибки.
 * Параметры передаются через структуру PDParams.
 */
#ifndef PDREGULATOR_H
#define PDREGULATOR_H

#include <stdint.h>
#include "SyncController.h"
#include "MovingAverage.h"

/**
 * @brief Параметры ПД-регулятора с мёртвой зоной.
 */
struct PDParams {
    int32_t Kp;        ///< коэффициент усиления (реальное усиление = Kp / 10)
    int32_t Pd;        ///< желаемое опережение (импульсы)
    int32_t dead_in;   ///< вход в мёртвую зону (импульсы)
    int32_t dead_out;  ///< выход из мёртвой зоны (импульсы)
    int32_t max_dF;    ///< максимальное изменение частоты за шаг (0.01 Гц)
};

class PDRegulator : public SyncController {
private:
    bool _inDeadzone;
    int32_t _lastFilteredError;
    MovingAverage<3> _errorFilter;

public:
    PDRegulator() : _inDeadzone(false), _lastFilteredError(0) {}

    /**
     * @brief Рассчитать управляющее воздействие (deltaF).
     *
     * @param py_norm Нормализованные импульсы кирпласта.
     * @param pk_raw  Сырые импульсы каландра.
     * @param params  Указатель на структуру PDParams.
     * @return deltaF в единицах 0.01 Гц.
     */
    int16_t compute(int32_t py_norm, int32_t pk_raw,
                    const void* params) override {
        const PDParams* p = static_cast<const PDParams*>(params);
        if (!p) return 0;

        int32_t rawErr = py_norm - pk_raw - p->Pd;
        _errorFilter.add(rawErr);
        int32_t filtErr = _errorFilter.average();
        _lastFilteredError = filtErr;

        int32_t absErr = (filtErr < 0) ? -filtErr : filtErr;

        if (_inDeadzone) {
            if (absErr <= p->dead_out) return 0;
            _inDeadzone = false;
        } else {
            if (absErr <= p->dead_in) {
                _inDeadzone = true;
                return 0;
            }
        }

        int32_t delta = (p->Kp * filtErr) / 10;   // Kp/10 – реальный коэффициент
        if (delta > p->max_dF) delta = p->max_dF;
        if (delta < -p->max_dF) delta = -p->max_dF;

        return (int16_t)delta;
    }

    /** Сбросить состояние мёртвой зоны и фильтр ошибки */
    void reset() {
        _inDeadzone = false;
        _errorFilter.reset();
    }

    /** Находится ли регулятор в мёртвой зоне? */
    bool inDeadzone() const { return _inDeadzone; }

    /** Последняя фильтрованная ошибка (для диагностики) */
    int32_t getFilteredError() const { return _lastFilteredError; }
};

#endif // PDREGULATOR_H