/**
 * @file SyncSystem.h
 * @brief Оркестратор системы синхронизации скорости.
 *
 * Связывает энкодеры, нормализатор и регулятор в единый рабочий цикл.
 * Вызывается периодически из main() — сам следит за интервалом.
 *
 * Зависимости:
 *   - EncoderReader.h
 *   - Normalizer.h
 *   - SyncController.h
 *   - AutoCalibrator.h
 *   - core/TimeProvider.h
 */

#ifndef SYNC_SYSTEM_H
#define SYNC_SYSTEM_H

#include <stdint.h>
#include "EncoderReader.h"
#include "Normalizer.h"
#include "SyncController.h"
#include "AutoCalibrator.h"
#include "core/TimeProvider.h"

class SyncSystem {
private:
    EncoderReader&  _encK;          // энкодер кирпласта
    EncoderReader&  _encL;          // энкодер каландра
    Normalizer&     _normalizer;    // нормализатор импульсов
    SyncController& _controller;    // регулятор (PDRegulator и т.п.)
    AutoCalibrator* _calibrator;    // автокалибровка (может быть nullptr)
    TimeProvider&   _time;
    const void*     _params;        // параметры регулятора (PDParams*)

    uint32_t _intervalMs;           // интервал измерения
    uint32_t _lastUpdate;           // время последнего обновления

    // Результаты последнего обновления
    float    _speedK;               // скорость кирпласта, м/мин
    float    _speedL;               // скорость каландра, м/мин
    int32_t  _pyNorm;               // нормализованные импульсы кирпласта
    int32_t  _pkRaw;                // сырые импульсы каландра
    int32_t  _rawError;             // сырая ошибка (py_norm - pk_raw, без Pd)
    int16_t  _deltaF;               // управляющее воздействие, 0.01 Гц
    bool     _updated;              // были ли новые данные в этом вызове

public:
    /**
     * @brief Конструктор.
     *
     * @param encK       Энкодер кирпласта (уже настроенный).
     * @param encL       Энкодер каландра (уже настроенный).
     * @param normalizer Нормализатор импульсов.
     * @param controller Регулятор (PDRegulator, PIDRegulator...).
     * @param params     Параметры регулятора (PDParams* и т.п.).
     * @param time       Источник времени.
     * @param intervalMs Интервал измерения (по умолчанию 500 мс).
     */
    SyncSystem(EncoderReader& encK,
               EncoderReader& encL,
               Normalizer& normalizer,
               SyncController& controller,
               const void* params,
               TimeProvider& time,
               uint32_t intervalMs = 500)
        : _encK(encK), _encL(encL),
          _normalizer(normalizer),
          _controller(controller),
          _calibrator(nullptr),
          _time(time),
          _params(params),
          _intervalMs(intervalMs),
          _lastUpdate(0),
          _speedK(0), _speedL(0),
          _pyNorm(0), _pkRaw(0), _rawError(0),
          _deltaF(0), _updated(false)
    {}

    /**
     * @brief Инициализация: сброс фильтров и синхронизация времени.
     * @param now Текущее время (millis).
     */
    void begin(uint32_t now) {
        _lastUpdate = now;
        _encK.resetFilter();
        _encL.resetFilter();
        _encK.resetCounter();
        _encL.resetCounter();
        _updated = false;
    }

    /**
     * @brief Главный метод. Вызывать в цикле как можно чаще.
     *
     * Сам отслеживает интервал. Когда наступает момент измерения:
     *   1. Снимает дельты с энкодеров
     *   2. Вычисляет скорости
     *   3. Нормализует импульсы
     *   4. Вызывает регулятор
     *   5. Если активна калибровка — добавляет пару
     *
     * @return true, если в этом вызове были новые данные.
     */
    bool update() {
        uint32_t now = _time.millis();

        if (now - _lastUpdate < _intervalMs) {
            _updated = false;
            return false;
        }

        _lastUpdate = now;

        // 1. Снять показания энкодеров
        _encK.capture();
        _encL.capture();

        // 2. Скорости
        _speedK = _encK.getSpeed();
        _speedL = _encL.getSpeed();

        // 3. Нормализация
        _pyNorm = _normalizer.normalize((int32_t)_encK.getDelta());
        _pkRaw  = (int32_t)_encL.getDelta();

        // 4. Сырая ошибка (для диагностики, без учёта Pd)
        _rawError = _pyNorm - _pkRaw;

        // 5. Регулятор
        _deltaF = _controller.compute(_pyNorm, _pkRaw, _params);

        // 6. Калибровка (если активна)
        if (_calibrator && _calibrator->isActive()) {
            _calibrator->addSample((int32_t)_encK.getDelta(),
                                   (int32_t)_encL.getDelta());
        }

        _updated = true;
        return true;
    }

    // --- Геттеры результатов ---

    /** Были ли новые данные в последнем вызове update() */
    bool hasUpdate() const { return _updated; }

    /** Скорость кирпласта, м/мин */
    float getSpeedK() const { return _speedK; }

    /** Скорость каландра, м/мин */
    float getSpeedL() const { return _speedL; }

    /** Нормализованные импульсы кирпласта */
    int32_t getPyNorm() const { return _pyNorm; }

    /** Сырые импульсы каландра */
    int32_t getPkRaw() const { return _pkRaw; }

    /**
     * @brief Сырая ошибка (py_norm - pk_raw).
     *
     * Не включает вычитание Pd и не фильтрована.
     * Для фильтрованной ошибки используйте PDRegulator::getFilteredError().
     */
    int32_t getRawError() const { return _rawError; }

    /**
     * @brief Управляющее воздействие в единицах 0.01 Гц.
     *
     * Новая частота ЧП = текущая_частота + deltaF / 100.
     * Например: deltaF = 25 → добавить 0.25 Гц.
     */
    int16_t getDeltaF() const { return _deltaF; }

    /** Текущий интервал измерения */
    uint32_t getInterval() const { return _intervalMs; }

    // --- Управление ---

    /** Изменить интервал измерения */
    void setInterval(uint32_t ms) {
        if (ms > 0) _intervalMs = ms;
    }

    /** Подключить автокалибровку */
    void setCalibrator(AutoCalibrator& cal) {
        _calibrator = &cal;
    }

    /** Отключить автокалибровку */
    void clearCalibrator() {
        _calibrator = nullptr;
    }

    /** Обновить параметры регулятора */
    void setParams(const void* params) {
        _params = params;
    }

    /** Сбросить регулятор и фильтры энкодеров */
    void reset() {
        _controller.reset();
        _encK.resetFilter();
        _encL.resetFilter();
        _lastUpdate = _time.millis();
        _updated = false;
    }

    /** Прямой доступ к нормализатору (для UI) */
    Normalizer& getNormalizer() { return _normalizer; }

    /** Прямой доступ к регулятору (для диагностики) */
    SyncController& getController() { return _controller; }
};

#endif // SYNC_SYSTEM_H