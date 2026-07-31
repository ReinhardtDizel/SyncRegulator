/**
 * @file PdCalibrator.h
 * @brief Автоматический подбор опережения Pd.
 *
 * Принцип:
 *   1. Оператор запускает калибровку (открывается отдельное окно).
 *   2. Автоматика продолжает работать.
 *   3. В окне калибровки отображаются скорости, ошибка, и временный
 *      параметр регулировки натяжения (аналог T, но только для калибровки).
 *   4. Оператор кнопками ± меняет этот параметр, визуально оценивая
 *      натяжение ткани.
 *   5. Когда натяжение идеально, оператор нажимает «Запомнить».
 *   6. Текущая фильтрованная ошибка становится новым Pd_base.
 *   7. Временный параметр обнуляется, оператор выходит из окна калибровки.
 *
 * Основной параметр T на главном экране при этом не затрагивается.
 */

#ifndef PD_CALIBRATOR_H
#define PD_CALIBRATOR_H

#include <stdint.h>

class PdCalibrator {
public:
    enum Mode {
        MODE_IDLE,      ///< калибровка неактивна
        MODE_ACTIVE,    ///< калибровка активна, оператор подбирает натяжение
    };

private:
    Mode     _mode;
    int32_t* _pdBase;           // указатель на Pd_base в PDParams
    int32_t  _originalPdBase;   // сохранённое исходное Pd_base
    int32_t  _calibT;           // временный параметр регулировки (аналог T)
    int32_t  _capturedPd;       // захваченное значение нового Pd_base
    bool     _captured;         // оператор нажал «Запомнить»

public:
    PdCalibrator()
        : _mode(MODE_IDLE)
        , _pdBase(nullptr)
        , _originalPdBase(0)
        , _calibT(0)
        , _capturedPd(0)
        , _captured(false)
    {}

    /**
     * @brief Привязать калибратор к Pd_base.
     * @param pdBase Указатель на Pd_base в PDParams.
     */
    void bind(int32_t* pdBase) {
        _pdBase = pdBase;
    }

    // ========================================================================
    // Управление калибровкой
    // ========================================================================

    /** Начать калибровку */
    void start() {
        if (!_pdBase) return;
        _mode = MODE_ACTIVE;
        _originalPdBase = *_pdBase;
        _calibT = 0;
        _capturedPd = 0;
        _captured = false;
    }

    /**
     * @brief Изменить временный параметр регулировки.
     *
     * Вызывается по кнопкам ± в окне калибровки.
     * Влияет на Pd_total внутри регулятора: Pd_total = Pd_base + T + calibT.
     *
     * @param delta Изменение (±1).
     * @param minVal Минимальное значение calibT.
     * @param maxVal Максимальное значение calibT.
     */
    void adjustCalibT(int32_t delta, int32_t minVal, int32_t maxVal) {
        if (_mode != MODE_ACTIVE || _captured) return;
        _calibT += delta;
        if (_calibT < minVal) _calibT = minVal;
        if (_calibT > maxVal) _calibT = maxVal;
    }

    /**
     * @brief Текущее значение Pd_total для передачи в регулятор.
     *
     * Во время калибровки: Pd_total = Pd_base + T + calibT
     * (T — штатный параметр с главного экрана, calibT — временный).
     *
     * @param tValue Текущее значение основного T.
     * @return Pd_total для регулятора.
     */
    int32_t getPdTotal(int32_t tValue) const {
        if (_mode == MODE_ACTIVE) {
            return *_pdBase + tValue + _calibT;
        }
        return *_pdBase + tValue;
    }

    /**
     * @brief Захватить текущую ошибку как новый Pd_base.
     *
     * Вызывается по нажатию «Запомнить».
     *
     * @param filteredError Текущая фильтрованная ошибка из регулятора.
     */
    void capture(int32_t filteredError) {
        if (_mode != MODE_ACTIVE) return;
        _capturedPd = filteredError;
        _captured = true;
    }

    /** Подтвердить: Pd_base = capturedPd, calibT = 0, выход из режима */
    void confirm() {
        if (_mode != MODE_ACTIVE || !_captured) return;
        if (_pdBase) *_pdBase = _capturedPd;
        _calibT = 0;
        _captured = false;
        _mode = MODE_IDLE;
    }

    /** Отменить калибровку и восстановить исходное Pd_base */
    void abort() {
        if (_mode != MODE_ACTIVE) return;
        if (_pdBase) *_pdBase = _originalPdBase;
        _calibT = 0;
        _capturedPd = 0;
        _captured = false;
        _mode = MODE_IDLE;
    }

    // ========================================================================
    // Геттеры
    // ========================================================================

    Mode getMode() const { return _mode; }
    bool isActive() const { return _mode != MODE_IDLE; }
    bool isCaptured() const { return _captured; }

    /** Временный параметр регулировки (для отображения в UI) */
    int32_t getCalibT() const { return _calibT; }

    /** Захваченное значение нового Pd_base (для отображения перед confirm) */
    int32_t getCapturedPd() const { return _capturedPd; }

    /** Исходное Pd_base (для информации) */
    int32_t getOriginalPdBase() const { return _originalPdBase; }
};

#endif // PD_CALIBRATOR_H