/**
 * @file SyncRegulator.h
 * @brief Главный заголовок библиотеки SyncRegulator.
 */
#ifndef SYNCREGULATOR_H
#define SYNCREGULATOR_H

// Подключаем нужный HAL
#if defined(ARDUINO)
    #include "hal/SyncHAL_Arduino.h"
#endif

#include "MovingAverage.h"
#include "Normalizer.h"
#include "EncoderReader.h"
#include "SyncController.h"
#include "PDRegulator.h"
#include "AutoCalibrator.h"

#endif