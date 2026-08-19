# MomoJoy

**BLE HID gamepad host สำหรับ ESP32-S3 บน Arduino core มาตรฐาน**

MomoJoy ทำมาเพื่อจอย **ShanWan Q34U ที่เลื่อนสวิตช์ไปตำแหน่ง `D`** (HID & APP / Android BLE)
แต่ใช้กับจอย BLE HID ยี่ห้ออื่นได้ด้วย เพราะ MomoJoy **อ่านและ parse HID Report Map ของจอยตอน
runtime** ไม่ได้ hard-code byte layout ของยี่ห้อใดยี่ห้อหนึ่ง

เขียนขึ้นใหม่ทั้งหมด ไม่ใช้ Bluepad32 — dependency เดียวคือ `NimBLE-Arduino`

> เอกสารทั้ง repo เป็นแบบสองภาษาในไฟล์เดียว: คำอธิบายเป็นภาษาไทย ส่วนศัพท์เทคนิค ชื่อ API
> เมนู และ log ทั้งหมดคงไว้เป็นภาษาอังกฤษตามของจริง

## สารบัญ

- [สถานะการทดสอบ](#สถานะการทดสอบ)
- [โครงสร้าง repo](#โครงสร้าง-repo)
- [เริ่มใช้งาน](#เริ่มใช้งาน)
- [โค้ดขั้นต่ำ](#โค้ดขั้นต่ำ)
- [ช่วงค่าที่อ่านได้](#ช่วงค่าที่อ่านได้)
- [หลักการทำงาน](#หลักการทำงาน)
- [ข้อจำกัด](#ข้อจำกัด)
- [เอกสารทั้งหมด](#เอกสารทั้งหมด)
- [การส่งโค้ดกลับ (contributing)](#การส่งโค้ดกลับ-contributing)
- [License](#license)

## สถานะการทดสอบ

| รายการ | ผล |
| --- | --- |
| `pio test -e native` — unit test 16 เคส (HID parser / scaling / mapping) | ✅ 16/16 |
| ชุดเทสต์เดียวกันภายใต้ AddressSanitizer + UndefinedBehaviorSanitizer | ✅ ผ่าน |
| `pio run` — ครบทั้ง 4 environment บน ESP32-S3, arduino-esp32 2.0.17 + NimBLE 1.4.3 | ✅ build + link + ได้ `.bin` |
| `arduino-cli compile` — ตัวอย่างครบทั้ง 4 ตัว ด้วย core และ library ชุดเดียวกัน | ✅ build + link |
| ต่อกับจอย Q34U ตัวจริง | ✅ ผู้เขียนทดสอบผ่านแล้ว |

ขนาด firmware: ประมาณ **530 KB flash / 32 KB RAM** สำหรับ build ที่พิมพ์ค่าทุกอย่าง

## โครงสร้าง repo

```text
.
├── Arduino/
│   ├── README.md               คู่มือเริ่มต้นสำหรับ Arduino IDE
│   └── libraries/MomoJoy/      <- ตัว library คัดลอกทั้งโฟลเดอร์นี้ไปไว้ที่
│       ├── library.properties     Documents/Arduino/libraries/
│       ├── keywords.txt
│       ├── src/                   MomoJoy.h/.cpp, MomoJoySerial.cpp, core/
│       └── examples/              01_RawDump .. 04_Minimal
├── PlatformIO/
│   ├── README.md               คู่มือเริ่มต้นสำหรับ VS Code + PlatformIO
│   ├── platformio.ini          environment: readall / rawdump / robot / minimal / native
│   ├── src/app_*.cpp           entry point บาง ๆ ที่ include ไฟล์ตัวอย่าง
│   └── test/test_core/         unit test
├── docs/                       INSTALL, PAIRING, CALIBRATION, API, HARDWARE, MIGRATION
├── tools/                      สคริปต์ทดสอบที่ไม่ต้องมี PlatformIO
└── .github/workflows/ci.yml    build ทั้งสองแพลตฟอร์มทุกครั้งที่ push
```

**source ของ library มีชุดเดียว** — `PlatformIO/platformio.ini` ชี้ไปที่ `../Arduino/libraries`
ผ่าน `lib_extra_dirs` ทั้งสอง toolchain จึง compile ไฟล์เดียวกันเป๊ะ ไม่มีทางหลุดจากกัน

## เริ่มใช้งาน

### PlatformIO (แนะนำ)

```bash
cd PlatformIO
pio run -e rawdump -t upload -t monitor    # รันอันนี้ก่อนเสมอเมื่อได้จอยตัวใหม่
pio run -e readall -t upload -t monitor    # firmware ตัวหลัก
pio test -e native                         # unit test ไม่ต้องมีบอร์ด
```

### Arduino IDE

1. คัดลอก `Arduino/libraries/MomoJoy` ไปไว้ที่ `Documents/Arduino/libraries/`
2. Library Manager → ติดตั้ง **NimBLE-Arduino 1.4.x**
3. เลือกบอร์ด **ESP32S3 Dev Module** แล้วตั้ง **USB CDC On Boot → Disabled**
4. File → Examples → MomoJoy → `02_ReadAllSerial`

รายละเอียดครบ: [docs/INSTALL.md](docs/INSTALL.md)

## โค้ดขั้นต่ำ

```cpp
#include <MomoJoy.h>
using namespace momojoy;

void setup() {
  Serial.begin(115200);
  MomoJoy.begin();
}

void loop() {
  MomoJoy.update();                    // ต้องเรียกทุกรอบ และไม่ block
  if (!MomoJoy.isConnected()) return;

  if (MomoJoy.justPressed(MOMO_BTN_A)) Serial.println("A!");
  int speed = -MomoJoy.ly();           // -512 .. 511
  int turbo = MomoJoy.r2();            //    0 .. 1023
}
```

## ช่วงค่าที่อ่านได้

ตั้งใจให้ตรงกับ Bluepad32 เป๊ะ ๆ โค้ดเดิมจึงแค่เปลี่ยนชื่อ accessor ก็ใช้ได้
(ดู [docs/MIGRATION_FROM_BLUEPAD32.md](docs/MIGRATION_FROM_BLUEPAD32.md))

| Accessor | ช่วงค่า |
| --- | --- |
| `lx() ly() rx() ry()` | −512 … 511, 0 = กึ่งกลาง, มี radial dead zone |
| `l2() r2()` | 0 … 1023 (analog trigger) |
| `dpad()` | bit mask: `MOMO_DPAD_UP / DOWN / LEFT / RIGHT` |
| `battery()` | 0 … 100 % หรือ `0xFF` เมื่อจอยไม่ได้ส่งค่ามา |

ปุ่มทั้งหมด: `MOMO_BTN_` `A B X Y L1 R1 L2 R2 SELECT START HOME L3 R3 M1 M2 C Z CAPTURE`

API ครบทุกตัว: [docs/API.md](docs/API.md)

## หลักการทำงาน

```text
scan (filter: HID service 0x1812 หรือ appearance = gamepad)
  └─ connect + bond (Just Works, LE Secure Connections, เก็บลง NVS)
       ├─ write Protocol Mode = Report        (0x2A4E <- 1)
       ├─ read  Report Map                    (0x2A4B) -> MomoHidParser -> ตาราง field
       ├─ subscribe ทุก input report          (0x2A4D, อ่านชนิดจาก descriptor 0x2908)
       └─ notify -> FreeRTOS queue -> MomoMapper::decode() -> MomoGamepadState
```

ไม่มีจุดไหนที่เดาว่า "byte ที่ 3 คือแกน Y" — ทุก offset มาจาก report descriptor ของจอยเอง
axis, trigger และ D-pad จึงถูกต้องกับจอย BLE HID ตัวไหนก็ได้ มีแค่ **หมายเลขปุ่ม (button
numbering)** ที่ต่างกันตามยี่ห้อ ซึ่งอยู่ในตารางเดียวที่แก้ได้ง่าย
(ดู [docs/CALIBRATION.md](docs/CALIBRATION.md))

## ข้อจำกัด

- ต่อจอยได้ทีละ 1 ตัว (ตั้งใจออกแบบไว้แบบนี้ เพื่อให้ใช้ RAM น้อย)
- BLE เท่านั้น — ESP32-S3 ไม่มี Bluetooth Classic จอย PS3/PS4/Xbox แบบ BR/EDR จึงใช้ไม่ได้
- ยังไม่รองรับ rumble และการคุม LED บนจอย (ยังไม่ได้ทำ HID output report)
- ล็อกไว้ที่ `arduino-esp32` 2.0.x + `NimBLE-Arduino` 1.4.x — ยังไม่ได้ทดสอบกับ 3.x / 2.x

## เอกสารทั้งหมด

| เอกสาร | เนื้อหา |
| --- | --- |
| [docs/INSTALL.md](docs/INSTALL.md) | ติดตั้ง toolchain, driver CH343P, ตั้งค่าบอร์ด |
| [docs/PAIRING.md](docs/PAIRING.md) | จับคู่จอย Q34U โหมด `D` และแก้ปัญหาการเชื่อมต่อ |
| [docs/CALIBRATION.md](docs/CALIBRATION.md) | ปรับ button mapping ให้ตรงกับจอยของคุณ |
| [docs/API.md](docs/API.md) | API reference ครบทุกฟังก์ชัน |
| [docs/HARDWARE.md](docs/HARDWARE.md) | ESP32-S3 N16R8, CH343P, partition, การต่อมอเตอร์ |
| [docs/MIGRATION_FROM_BLUEPAD32.md](docs/MIGRATION_FROM_BLUEPAD32.md) | ย้ายโค้ดเดิมจาก Bluepad32 |

## การส่งโค้ดกลับ (contributing)

ไฟล์ `.ino` และ source ของ library ต้องเป็น **ASCII ล้วน** — อักขระที่ไม่ใช่ ASCII ทำให้
automatic prototype generator ของ Arduino พัง แล้วขึ้น error ชวนงงว่า
`ambiguating new declaration` (CI ตรวจให้อัตโนมัติ) ให้เขียนคำอธิบายภาษาไทยไว้ใน `docs/`
แทนการใส่ใน comment ของโค้ด

ก่อนเปิด PR:

```bash
./tools/run_native_tests.sh     # unit test + sanitizer ไม่ต้องมี PlatformIO
./tools/check_esp_layer.sh      # syntax check ชั้น NimBLE ด้วย stub header
cd PlatformIO && pio run -e readall && pio test -e native
```

## License

MIT — ดู [LICENSE](LICENSE)
