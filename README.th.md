# MomoJoy

**BLE HID gamepad host สำหรับ ESP32-S3 บน Arduino core ธรรมดา**

ทำมาเพื่อจอย **ShanWan Q34U ที่เลื่อนสวิตช์ไปตำแหน่ง `D`** (HID & APP / Android BLE)
แต่ใช้กับจอย BLE HID ตัวอื่นได้ด้วย เพราะ MomoJoy **อ่านและถอด HID Report Map ของจอยตอน runtime**
ไม่ได้ hard-code ไบต์ของยี่ห้อใดยี่ห้อหนึ่ง

เขียนขึ้นใหม่ทั้งหมด ไม่ใช้ Bluepad32 — dependency เดียวคือ `NimBLE-Arduino`

> English → [README.md](README.md)

---

## สถานะการทดสอบ

| รายการ | ผล |
|---|---|
| `pio test -e native` — unit test 16 เคส (HID parser / scaling / mapping) | ✅ 16/16 |
| เทสต์ชุดเดียวกันรันผ่าน AddressSanitizer + UndefinedBehaviorSanitizer | ✅ ผ่าน |
| `pio run` — ครบทั้ง 4 env บน ESP32-S3, arduino-esp32 2.0.17 + NimBLE 1.4.3 | ✅ build + link + ได้ `.bin` |
| `arduino-cli compile` — ตัวอย่างครบทั้ง 4 ตัว ด้วย core และไลบรารีชุดเดียวกัน | ✅ build + link |
| ต่อกับจอย Q34U จริง | ✅ ผู้ใช้ทดสอบผ่านแล้ว |

ขนาดเฟิร์มแวร์: ประมาณ **530 KB flash / 32 KB RAM** สำหรับตัวที่พิมพ์ค่าทุกอย่าง

---

## โครงสร้าง repo

```
.
├── Arduino/
│   ├── README.th.md            คู่มือเริ่มต้นสำหรับ Arduino IDE
│   └── libraries/MomoJoy/      <- ตัวไลบรารี คัดลอกทั้งโฟลเดอร์นี้ไปไว้ที่
│       ├── library.properties     Documents/Arduino/libraries/
│       ├── keywords.txt
│       ├── src/                   MomoJoy.h/.cpp, MomoJoySerial.cpp, core/
│       └── examples/              01_RawDump .. 04_Minimal
├── PlatformIO/
│   ├── README.th.md            คู่มือเริ่มต้นสำหรับ VS Code + PlatformIO
│   ├── platformio.ini          env: readall / rawdump / robot / minimal / native
│   ├── src/app_*.cpp           entry point บาง ๆ ที่ include ไฟล์ตัวอย่าง
│   └── test/test_core/         unit test
├── docs/
│   ├── th/                     INSTALL, PAIRING, CALIBRATION, API, HARDWARE, MIGRATION
│   └── en/                     เอกสารชุดเดียวกันภาษาอังกฤษ
├── tools/                      สคริปต์ทดสอบที่ไม่ต้องมี PlatformIO
└── .github/workflows/ci.yml    build ทั้งสองแพลตฟอร์มทุกครั้งที่ push
```

**ซอร์สไลบรารีมีชุดเดียว** — `PlatformIO/platformio.ini` ชี้ไปที่ `../Arduino/libraries`
ผ่าน `lib_extra_dirs` ทั้งสอง toolchain จึงคอมไพล์ไฟล์เดียวกันเป๊ะ ไม่มีทางหลุดจากกัน

---

## เริ่มใช้งานเร็ว ๆ

### PlatformIO (แนะนำ)

```bash
cd PlatformIO
pio run -e rawdump -t upload -t monitor    # ทำอันนี้ก่อนเมื่อได้จอยใหม่
pio run -e readall -t upload -t monitor    # เฟิร์มแวร์ตัวหลัก
pio test -e native                         # unit test ไม่ต้องมีบอร์ด
```

### Arduino IDE

1. คัดลอก `Arduino/libraries/MomoJoy` ไปไว้ที่ `Documents/Arduino/libraries/`
2. Library Manager → ติดตั้ง **NimBLE-Arduino 1.4.x**
3. เลือกบอร์ด **ESP32S3 Dev Module** แล้วตั้ง **USB CDC On Boot → Disabled**
4. File → Examples → MomoJoy → `02_ReadAllSerial`

รายละเอียดครบ: [docs/th/INSTALL.md](docs/th/INSTALL.md)

---

## โค้ดขั้นต่ำ

```cpp
#include <MomoJoy.h>
using namespace momojoy;

void setup() {
  Serial.begin(115200);
  MomoJoy.begin();
}

void loop() {
  MomoJoy.update();                    // ต้องเรียกทุกรอบ ไม่บล็อก
  if (!MomoJoy.isConnected()) return;

  if (MomoJoy.justPressed(MOMO_BTN_A)) Serial.println("A!");
  int speed = -MomoJoy.ly();           // -512 .. 511
  int turbo = MomoJoy.r2();            //    0 .. 1023
}
```

## ช่วงค่าที่อ่านได้

ตั้งใจให้ตรงกับ Bluepad32 เป๊ะ ๆ โค้ดเดิมจึงแค่เปลี่ยนชื่อฟังก์ชันก็ใช้ได้
(ดู [docs/th/MIGRATION_FROM_BLUEPAD32.md](docs/th/MIGRATION_FROM_BLUEPAD32.md))

| ฟังก์ชัน | ช่วงค่า |
|---|---|
| `lx() ly() rx() ry()` | −512 … 511, 0 = กลาง, มี deadzone แบบวงกลม |
| `l2() r2()` | 0 … 1023 (ไกอนาล็อก) |
| `dpad()` | บิต `MOMO_DPAD_UP / DOWN / LEFT / RIGHT` |
| `battery()` | 0 … 100 % หรือ `0xFF` ถ้าจอยไม่ส่งมา |

ปุ่ม: `MOMO_BTN_` `A B X Y L1 R1 L2 R2 SELECT START HOME L3 R3 M1 M2 C Z CAPTURE`

API ครบ: [docs/th/API.md](docs/th/API.md)

---

## หลักการทำงาน

```
สแกน (กรองด้วย HID service 0x1812 หรือ appearance = gamepad)
  └─ connect + จับคู่ (Just Works, LE Secure Connections, เก็บลง NVS)
       ├─ เขียน Protocol Mode = Report        (0x2A4E <- 1)
       ├─ อ่าน Report Map                     (0x2A4B) -> MomoHidParser -> ตาราง field
       ├─ subscribe ทุก input report          (0x2A4D, ดูชนิดจาก descriptor 0x2908)
       └─ notify -> คิว FreeRTOS -> MomoMapper::decode() -> MomoGamepadState
```

ไม่มีการเดาว่า "ไบต์ที่ 3 คือแกน Y" ทุก offset มาจาก report descriptor ของจอยเอง
แกนอนาล็อก ไกอนาล็อก และ D-Pad จึงถูกต้องกับจอย BLE HID ตัวไหนก็ได้
มีแค่ **หมายเลขปุ่ม** ที่ต่างกันตามยี่ห้อ ซึ่งอยู่ในตารางเดียวที่แก้ได้ง่าย
([docs/th/CALIBRATION.md](docs/th/CALIBRATION.md))

---

## ข้อจำกัด

- ต่อจอยได้ทีละ 1 ตัว (ตั้งใจ เพื่อให้กิน RAM น้อย)
- BLE อย่างเดียว — ESP32-S3 ไม่มี Bluetooth Classic จอย PS3/PS4/Xbox แบบ BR/EDR จึงใช้ไม่ได้
- ยังไม่รองรับสั่นและคุมไฟ LED บนจอย (ยังไม่ได้ทำ HID output report)
- ล็อกไว้ที่ `arduino-esp32` 2.0.x + `NimBLE-Arduino` 1.4.x ยังไม่ได้ทดสอบกับ 3.x / 2.x

---

## ถ้าจะแก้โค้ดส่งกลับ

ไฟล์ `.ino` และซอร์สไลบรารีต้องเป็น **ASCII ล้วน** — อักขระที่ไม่ใช่ ASCII ทำให้ตัวสร้าง
prototype อัตโนมัติของ Arduino พัง แล้วขึ้น error ชวนงงว่า `ambiguating new declaration`
(CI ตรวจให้) เขียนคำอธิบายภาษาไทยไว้ใน `docs/` แทนคอมเมนต์ในโค้ด

ก่อนส่ง PR:

```bash
./tools/run_native_tests.sh     # unit test + sanitizer ไม่ต้องมี PlatformIO
./tools/check_esp_layer.sh      # syntax check ชั้น NimBLE ด้วย stub header
cd PlatformIO && pio run -e readall && pio test -e native
```

## สัญญาอนุญาต

MIT — ดู [LICENSE](LICENSE)
