# SyncRegulator

**A speed synchronization and tension regulation library for embedded systems (Arduino/AVR/STM32/ESP).**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Overview

SyncRegulator provides all the building blocks for closed‑loop speed synchronization between two rotating shafts using incremental encoders:

- **Encoder pulse measurement** – interrupt‑based counting with configurable update interval.
- **Moving average filter** – smooths speed and error signals.
- **Impulse normalization** – brings encoder pulses to a common scale.
- **PD‑regulator with dead zone** – calculates frequency correction with hysteresis.
- **Auto‑calibration** – automatically determines the normalization coefficient.
- **Abstract `SyncController` interface** – allows plugging in custom regulation laws (PID, fuzzy, etc.).

The library is **hardware‑independent**: all platform‑specific code is isolated in a HAL interface. You only need to implement a small set of functions for your microcontroller.

## Hardware Abstraction

SyncRegulator does not depend on any specific platform. To use it, you must implement the `SyncHAL` interface for your target hardware:

```cpp
class MyHAL : public SyncHAL {
    void pinModeInputPullup(uint8_t pin) override;
    void attachInterruptExt(uint8_t pin, void (*isr)(), int mode) override;
    unsigned long millis() override;
    void delayMs(uint32_t ms) override;
    void disableInterrupts() override;
    void enableInterrupts() override;
};
```

See the examples/ folder for ready‑to‑use implementations for Arduino and AVR.

Installation

Arduino IDE

1. Download the repository as a ZIP file.
2. In Arduino IDE, go to Sketch → Include Library → Add .ZIP Library and select the downloaded file.
3. Restart the IDE.

PlatformIO

Add the following to your platformio.ini:

```ini
lib_deps =
    https://github.com/ReinhardtDizel/SyncRegulator.git
```

Or copy the library folder into the lib/ directory of your project.

Quick Start

Here is a minimal example using the library with an Arduino‑compatible board.
First, implement the HAL (you can copy the SyncHAL_Arduino from the examples/ folder):

```cpp
#include <SyncRegulator.h>

// 1. Your custom HAL (here for Arduino)
class ArduinoHAL : public SyncHAL {
public:
    void pinModeInputPullup(uint8_t pin) override {
        pinMode(pin, INPUT_PULLUP);
    }
    void attachInterruptExt(uint8_t pin, void (*isr)(), int mode) override {
        attachInterrupt(digitalPinToInterrupt(pin), isr, mode);
    }
    unsigned long millis() override {
        return ::millis();
    }
    void delayMs(uint32_t ms) override {
        ::delay(ms);
    }
    void disableInterrupts() override {
        noInterrupts();
    }
    void enableInterrupts() override {
        interrupts();
    }
};

// 2. Create HAL instance and encoder readers
ArduinoHAL hal;
EncoderReader encK(hal, 2, 2457, 0, 500);  // pin, coeff*1e6, offset*1000, intervalMs
EncoderReader encL(hal, 3, 11197, 0, 500);

Normalizer normalizer(219);      // K_norm * 1000
PDRegulator regulator;
PDParams params = { 50, 5, 2, 4, 50 };   // Kp, Pd, dead_in, dead_out, max_dF

void setup() {
    encK.begin();
    encL.begin();
}

void loop() {
    if (encK.update() && encL.update()) {
        int32_t py_norm = normalizer.normalize(encK.getRawImpulses());
        int32_t pk_raw  = encL.getRawImpulses();
        int16_t deltaF = regulator.compute(py_norm, pk_raw, &params);
        // Apply deltaF to your frequency converter...
    }
}
```

For more complete examples, see the examples/ folder.

Dependencies

The library itself has no external dependencies. It uses only standard C++ and the small HAL interface.
Ready‑to‑use HAL implementations for Arduino and pure AVR are provided as examples in the examples/ folder.

License

This library is released under the MIT License. See LICENSE for details.

Author

Reinhardt Michael
GitHub: ReinhardtDizel
Email: dizel882@gmail.com
