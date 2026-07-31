/**
 * @file PwmCalibrator.h
 * @brief Калибровка выходного тракта ШИМ → частотный преобразователь.
 *
 * Два режима:
 *
 * 1) Ручная калибровка:
 *    - Оператор кнопками регулятора выставляет желаемую частоту freqCalc.
 *    - Регулятор выдаёт ШИМ, соответствующий этой частоте (с текущим K_pwm).
 *    - Оператор смотрит на дисплей частотника, видит реальную freqReal.
 *    - Вводит freqReal в интерфейс регулятора.
 *    - Калибратор вычисляет новый K_pwm = freqReal / freqCalc.
 *
 * 2) Автокалибровка (по Modbus):
 *    - Регулятор последовательно задаёт несколько значений ШИМ.
 *    - Для каждого читает реальную частоту с частотника по Modbus.
 *    - Строит линейную аппроксимацию freqReal = K_pwm × freqCalc + B_pwm.
 *    - Вычисляет и сохраняет K_pwm (и опционально B_pwm).
 */

#ifndef PWM_CALIBRATOR_H
#define PWM_CALIBRATOR_H

#include <stdint.h>

class PwmCalibrator {
public:
    /** Режим калибровки */
    enum Mode {
        MODE_IDLE,      ///< калибровка неактивна
        MODE_MANUAL,    ///< ручная: оператор вводит freqReal
        MODE_AUTO       ///< авто: регулятор опрашивает частотник по Modbus
    };

private:
    float  _K_pwm;           // поправочный коэффициент (по умолчанию 1.0)
    float  _B_pwm;           // смещение, Гц (по умолчанию 0.0)
    Mode   _mode;
    bool   _confirmed;       // оператор подтвердил измеренную частоту

    // Для авто-режима
    static const int AUTO_STEPS = 5;    // количество точек
    float  _autoFreqCalc[AUTO_STEPS];   // заданные частоты
    float  _autoFreqReal[AUTO_STEPS];   // измеренные частоты
    int    _autoStep;                   // текущий шаг (0..AUTO_STEPS-1)
    bool   _autoDone;                   // все шаги завершены

public:
    PwmCalibrator()
        : _K_pwm(1.0f), _B_pwm(0.0f),
          _mode(MODE_IDLE), _confirmed(false),
          _autoStep(0), _autoDone(false)
    {
        for (int i = 0; i < AUTO_STEPS; i++) {
            _autoFreqCalc[i] = 0.0f;
            _autoFreqReal[i] = 0.0f;
        }
    }

    // ========================================================================
    // Ручная калибровка
    // ========================================================================

    /** Начать ручную калибровку */
    void startManual() {
        _mode = MODE_MANUAL;
        _confirmed = false;
    }

    /**
     * @brief Установить реально измеренную частоту (ручной режим).
     *
     * @param freqCalc Расчётная частота (та, что была задана оператором).
     * @param freqReal Реальная частота (с дисплея частотника).
     */
    void setFrequencies(float freqCalc, float freqReal) {
        if (_mode != MODE_MANUAL) return;
        if (freqCalc == 0.0f) return;
        _K_pwm = freqReal / freqCalc;
        _confirmed = true;
    }

    /** Подтвердить и выйти из ручного режима */
    void confirmManual() {
        if (_mode == MODE_MANUAL && _confirmed) {
            _mode = MODE_IDLE;
        }
    }

    // ========================================================================
    // Автокалибровка (Modbus)
    // ========================================================================

    /** Начать автокалибровку */
    void startAuto() {
        _mode = MODE_AUTO;
        _autoStep = 0;
        _autoDone = false;
        for (int i = 0; i < AUTO_STEPS; i++) {
            _autoFreqCalc[i] = 0.0f;
            _autoFreqReal[i] = 0.0f;
        }
    }

    /**
     * @brief Получить следующую тестовую частоту для авто-режима.
     *
     * Вызывается в цикле: регулятор выставляет эту частоту,
     * ждёт устаканивания, читает реальную по Modbus,
     * и вызывает addAutoSample().
     *
     * @return Тестовая частота, Гц. Или -1, если все шаги пройдены.
     */
    float getNextAutoFreq() {
        if (_mode != MODE_AUTO || _autoDone) return -1.0f;

        // Равномерно распределённые точки от 10% до 100% диапазона
        // Диапазон берётся извне (minFreq..maxFreq), здесь просто пропорция
        float ratio = (float)(_autoStep + 1) / (float)AUTO_STEPS;
        return ratio;   // вызывающий умножит на (maxFreq - minFreq) + minFreq
    }

    /**
     * @brief Добавить измерение в авто-режиме.
     *
     * @param freqCalc Заданная частота, Гц.
     * @param freqReal Измеренная (по Modbus) частота, Гц.
     * @return true, если это был последний шаг и калибровка завершена.
     */
    bool addAutoSample(float freqCalc, float freqReal) {
        if (_mode != MODE_AUTO || _autoDone) return false;
        if (_autoStep >= AUTO_STEPS) return false;

        _autoFreqCalc[_autoStep] = freqCalc;
        _autoFreqReal[_autoStep] = freqReal;
        _autoStep++;

        if (_autoStep >= AUTO_STEPS) {
            computeLinearFit();
            _autoDone = true;
            _mode = MODE_IDLE;
            return true;
        }
        return false;
    }

    /** Прогресс автокалибровки (0..100) */
    int autoProgress() const {
        if (_mode != MODE_AUTO) return 0;
        return (_autoStep * 100) / AUTO_STEPS;
    }

    // ========================================================================
    // Общие методы
    // ========================================================================

    /** Отменить любую калибровку */
    void abort() {
        _mode = MODE_IDLE;
        _confirmed = false;
        _autoStep = 0;
        _autoDone = false;
    }

    /** Режим калибровки */
    Mode getMode() const { return _mode; }

    /** Активен ли любой режим калибровки */
    bool isActive() const { return _mode != MODE_IDLE; }

    /** Введена ли реальная частота в ручном режиме (ожидает confirm) */
    bool isConfirmed() const { return _confirmed; }

    /** Получить текущий поправочный коэффициент */
    float getK() const { return _K_pwm; }

    /** Получить текущее смещение */
    float getB() const { return _B_pwm; }

    /** Установить K_pwm вручную (например, из EEPROM) */
    void setK(float k) {
        if (k > 0.0f && k < 10.0f) {
            _K_pwm = k;
        }
    }

    /** Установить B_pwm вручную */
    void setB(float b) {
        _B_pwm = b;
    }

    /**
     * @brief Применить калибровку к расчётной частоте.
     * @param freqCalc Расчётная частота, Гц.
     * @return Исправленная частота, Гц.
     */
    float apply(float freqCalc) const {
        return freqCalc * _K_pwm + _B_pwm;
    }

private:
    /**
     * @brief Вычислить K_pwm и B_pwm методом наименьших квадратов
     *        по накопленным точкам.
     *
     * Модель: freqReal = K × freqCalc + B
     */
    void computeLinearFit() {
        float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
        int n = AUTO_STEPS;

        for (int i = 0; i < n; i++) {
            float x = _autoFreqCalc[i];
            float y = _autoFreqReal[i];
            sumX  += x;
            sumY  += y;
            sumXY += x * y;
            sumX2 += x * x;
        }

        float denom = n * sumX2 - sumX * sumX;
        if (denom == 0.0f) return;

        _K_pwm = (n * sumXY - sumX * sumY) / denom;
        _B_pwm = (sumY - _K_pwm * sumX) / n;

        // Защита от некорректных значений
        if (_K_pwm <= 0.0f || _K_pwm > 10.0f) {
            _K_pwm = 1.0f;
            _B_pwm = 0.0f;
        }
    }
};

#endif // PWM_CALIBRATOR_H