/**
 * @file EncoderReader.cpp
 * @brief Статические члены EncoderReader + привязка ISR для AVR.
 */

#include "EncoderReader.h"
#include <avr/interrupt.h>

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

// Привязка к аппаратным векторам AVR
ISR(INT0_vect) { if (EncoderReader::_instances[0]) EncoderReader::_instances[0]->capture_isr(); }
ISR(INT1_vect) { if (EncoderReader::_instances[1]) EncoderReader::_instances[1]->capture_isr(); }
