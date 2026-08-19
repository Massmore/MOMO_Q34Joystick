# Changelog

All notable changes to MomoJoy are documented here.

## 1.0.0 — 2026-08-19

First release.

### Added
- BLE HID host written from scratch on NimBLE-Arduino (no Bluepad32 dependency).
- Runtime HID Report Descriptor parsing, so any BLE HID gamepad is decoded correctly
  without hard-coding a vendor byte layout.
- Button profile for the ShanWan Q34U with the mode switch on `D` (HID & APP / Android BLE).
- Bluepad32-compatible value ranges (sticks -512..511, triggers 0..1023) for easy migration.
- Support for Home/Back keys delivered in a separate Consumer-page report.
- Edge detection (`justPressed` / `justReleased`), radial dead zone, battery level.
- Four examples: RawDump (calibration), ReadAllSerial, MomoRobot, Minimal.
- 16 unit tests for the pure-C++ core (`pio test -e native`).
- Repository split into `Arduino/` and `PlatformIO/` trees sharing ONE copy of the library
  through `lib_extra_dirs`.
- Bilingual documentation (English and Thai) under `docs/en` and `docs/th`.
- GitHub Actions CI: native tests, PlatformIO build, Arduino IDE build, ASCII-only check.

### Verified against real toolchains
- `pio run` succeeds for all four environments with `espressif32@6.9.0`
  (arduino-esp32 2.0.17, xtensa-esp32s3-elf-g++ 8.4.0) and `NimBLE-Arduino 1.4.3`.
- `arduino-cli compile` succeeds for all four examples with the same core and library.
- `pio test -e native` passes 16/16; the same tests also pass under ASan + UBSan.

### Fixed during development
- NimBLE-Arduino 1.4.3 returns `NimBLEAttValue` from `readValue()`, not `std::string`;
  `setConnectTimeout()` takes `uint8_t`. Both corrected after checking the real headers.
- Sketches are now ASCII-only and declare their own prototypes. Non-ASCII characters made the
  Arduino automatic prototype generator emit `int` instead of `void`, producing misleading
  `ambiguating new declaration` errors. CI now enforces ASCII-only sources.
