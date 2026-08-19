# Changelog

บันทึกการเปลี่ยนแปลงที่สำคัญทั้งหมดของ MomoJoy

รูปแบบอ้างอิงตาม [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
และเลขเวอร์ชันเป็นไปตาม [Semantic Versioning](https://semver.org/spec/v2.0.0.html)

## Unreleased

ยังไม่มีรายการเปลี่ยนแปลง

## 1.0.0 — 2026-08-19

release แรก

### Added

- BLE HID host เขียนขึ้นใหม่ทั้งหมดบน NimBLE-Arduino (ไม่พึ่ง Bluepad32)
- parse HID Report Descriptor ตอน runtime จอย BLE HID ตัวไหนก็ decode ได้ถูกต้อง
  โดยไม่ต้อง hard-code byte layout ของยี่ห้อใดยี่ห้อหนึ่ง
- button profile สำหรับ ShanWan Q34U ที่เลื่อนสวิตช์ไปตำแหน่ง `D` (HID & APP / Android BLE)
- ช่วงค่าเข้ากันได้กับ Bluepad32 (stick −512..511, trigger 0..1023) ย้ายโค้ดเดิมได้ง่าย
- รองรับปุ่ม Home/Back ที่จอยส่งมาแยกเป็น report ของ Consumer page
- edge detection (`justPressed()` / `justReleased()`), radial dead zone และระดับ battery
- ตัวอย่าง 4 ตัว: `01_RawDump` (calibration), `02_ReadAllSerial`, `03_MomoRobot`, `04_Minimal`
- unit test 16 เคสสำหรับ core ที่เป็น C++ ล้วน (`pio test -e native`)
- แยก repo เป็น `Arduino/` และ `PlatformIO/` โดยใช้ source ของ library ชุดเดียวกันผ่าน
  `lib_extra_dirs`
- เอกสารสองภาษา (ไทย + ศัพท์เทคนิค EN) ในไฟล์เดียว อยู่ใต้ `docs/`
- GitHub Actions CI: native test, PlatformIO build, Arduino IDE build และ ASCII-only check

### Fixed

- NimBLE-Arduino 1.4.3 คืนค่าเป็น `NimBLEAttValue` จาก `readValue()` ไม่ใช่ `std::string`
  และ `setConnectTimeout()` รับ `uint8_t` — แก้ทั้งสองจุดหลังตรวจกับ header ตัวจริงแล้ว
- sketch ทุกตัวเป็น ASCII ล้วนและประกาศ prototype ของตัวเอง เพราะอักขระที่ไม่ใช่ ASCII ทำให้
  automatic prototype generator ของ Arduino สร้าง `int` แทน `void` แล้วขึ้น error ชวนงงว่า
  `ambiguating new declaration` — ตอนนี้ CI บังคับให้ source เป็น ASCII แล้ว

### Verified

ทดสอบกับ toolchain จริงดังนี้:

- `pio run` ผ่านครบทั้ง 4 environment ด้วย `espressif32@6.9.0`
  (arduino-esp32 2.0.17, xtensa-esp32s3-elf-g++ 8.4.0) และ `NimBLE-Arduino 1.4.3`
- `arduino-cli compile` ผ่านครบทั้ง 4 ตัวอย่าง ด้วย core และ library ชุดเดียวกัน
- `pio test -e native` ผ่าน 16/16 และชุดเดียวกันผ่านภายใต้ ASan + UBSan ด้วย
