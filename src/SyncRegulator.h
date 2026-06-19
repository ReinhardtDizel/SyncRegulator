/**
 * @file SyncRegulator.h
 * @brief Главный заголовок библиотеки SyncRegulator.
 *
 * Библиотека для синхронизации скорости и регулирования натяжения.
 * Включает измерение импульсов энкодеров, фильтрацию, нормализацию,
 * ПД-регулятор с мёртвой зоной и автокалибровку.
 *
 * Подключите этот файл, чтобы получить доступ ко всем компонентам:
 * @code
 * #include <SyncRegulator.h>
 * @endcode
 */
#ifndef SYNCREGULATOR_H
#define SYNCREGULATOR_H

#include "MovingAverage.h"
#include "Normalizer.h"
#include "EncoderReader.h"
#include "SyncController.h"
#include "PDRegulator.h"
#include "AutoCalibrator.h"

#endif // SYNCREGULATOR_H