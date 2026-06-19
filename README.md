# SyncRegulator

**A speed synchronization and tension regulation library for Arduino/AVR/STM32/ESP.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Overview

SyncRegulator provides all the building blocks for closed‑loop speed synchronization between two rotating shafts using incremental encoders:

- **Encoder pulse measurement** – interrupt‑based counting with configurable update interval.
- **Moving average filter** – smooths speed and error signals.
- **Impulse normalization** – brings encoder pulses to a common scale.
- **PD‑regulator with dead zone** – calculates frequency correction with hysteresis.
- **Auto‑calibration** – automatically determines the normalization coefficient.
- **Abstract `SyncController` interface** – allows plugging in custom regulation laws (PID, fuzzy, etc.).

The library is hardware‑independent: all platform‑specific code is isolated in HAL files. It is designed to work seamlessly with any character LCD UI (e.g., SimpleLCDUI) and button handling (e.g., OneButton).

## Installation

### Arduino IDE
1. Download the repository as a ZIP file.
2. In Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library** and select the downloaded file.
3. Restart the IDE.

### PlatformIO
Add the following to your `platformio.ini`:
```ini
lib_deps =
    https://github.com/ReinhardtDizel/SyncRegulator.git
```
Or copy the library folder into the lib/ directory of your project.

Quick Start
```cpp
#include <SyncRegulator.h>

// Create encoder readers (pin, coeff*1e6, offset*1000, intervalMs)
EncoderReader encK(2, 2457, 0, 500);
EncoderReader encL(3, 11197, 0, 500);

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
See the examples folder for a complete working example.

Dependencies
The library itself has no external dependencies. It uses standard Arduino functions (digitalPinToInterrupt, attachInterrupt, millis) which can be replaced by a HAL layer for other platforms.

License
This library is released under the MIT License. See LICENSE for details.

Author
Reinhardt Michael

GitHub: ReinhardtDizel

Email: dizel882@gmail.com