/**
 * @file SyncController.h
 * @brief Абстрактный интерфейс регулятора синхронизации.
 *
 * Определяет общий метод compute(), который должен быть реализован
 * в любом конкретном регуляторе (ПД, ПИД и т.д.).
 */
#ifndef SYNCCONTROLLER_H
#define SYNCCONTROLLER_H

#include <stdint.h>

/**
 * @brief Базовый класс для всех регуляторов синхронизации.
 *
 * Наследники (PDRegulator, PIDRegulator, ...) обязаны
 * переопределить compute(), принимая нормализованные импульсы
 * кирпласта, сырые импульсы каландра и указатель на параметры.
 */
class SyncController {
public:
    virtual ~SyncController() {}

    /**
     * @brief Рассчитать управляющее воздействие (deltaF).
     *
     * @param py_norm   Нормализованные импульсы кирпласта.
     * @param pk_raw    Сырые импульсы каландра.
     * @param params    Указатель на структуру параметров регулятора.
     *                  Конкретный тип зависит от реализации.
     * @return deltaF в единицах 0.01 Гц.
     */
    virtual int16_t compute(int32_t py_norm, int32_t pk_raw,
                            const void* params) = 0;
};

#endif // SYNCCONTROLLER_H