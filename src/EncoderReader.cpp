/**
 * @file EncoderReader.cpp
 * @brief Статические члены EncoderReader.
 */

#include "EncoderReader.h"

EncoderReader* EncoderReader::_instances[EncoderReader::MAX_ENCODERS] = { nullptr };
uint8_t EncoderReader::_nextIndex = 0;

const EncoderReader::IsrFunc EncoderReader::_dispatchers[EncoderReader::MAX_ENCODERS] = {
    EncoderReader::isr0,
    EncoderReader::isr1,
    EncoderReader::isr2,
    EncoderReader::isr3,
    EncoderReader::isr4,
    EncoderReader::isr5,
    EncoderReader::isr6,
    EncoderReader::isr7
};