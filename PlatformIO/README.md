# MomoJoy — PlatformIO (VS Code)

คู่มือเริ่มต้นใช้งาน MomoJoy บน VS Code + PlatformIO
(ถ้าใช้ Arduino IDE ให้ดู [../Arduino/README.md](../Arduino/README.md) แทน)

## 1. ติดตั้ง

1. ติดตั้ง [VS Code](https://code.visualstudio.com/)
2. VS Code → Extensions → ค้นหา **PlatformIO IDE** → Install
3. ติดตั้ง **CH343P USB driver** — ดู [docs/INSTALL.md](../docs/INSTALL.md#ch343p-driver)
4. เปิด **โฟลเดอร์ `PlatformIO/` นี้** ใน VS Code (ไม่ใช่ root ของ repo)
   ครั้งแรก PlatformIO จะ download `espressif32` และ `NimBLE-Arduino` ให้เอง (ประมาณ 5–10 นาที)

## 2. Environment ที่มีให้

| คำสั่ง | ทำอะไร |
| --- | --- |
| `pio run -e rawdump -t upload -t monitor` | โหมด calibrate: พิมพ์ HID report map และ byte ดิบของ report |
| `pio run -e readall -t upload -t monitor` | firmware ตัวหลัก: ปุ่ม axis trigger battery ครบ |
| `pio run -e robot -t upload` | ตัวอย่างคุมหุ่น 2 มอเตอร์ |
| `pio run -e minimal -t upload` | sketch ที่สั้นที่สุด |
| `pio test -e native` | unit test 16 เคสบน PC ไม่ต้องมีบอร์ด |

`readall` เป็น environment เริ่มต้น พิมพ์ `pio run` เฉย ๆ จึงเท่ากับ build ตัวนี้

## 3. โค้ดอยู่ตรงไหน

```text
PlatformIO/
├── platformio.ini
├── src/app_*.cpp        entry point บาง ๆ หนึ่งไฟล์ต่อหนึ่ง environment
└── test/test_core/      unit test ของ core ที่เป็น C++ ล้วน
```

ตัว library **ไม่ได้ copy ซ้ำ** มาไว้ที่นี่ เพราะใน `platformio.ini` มีบรรทัด

```ini
lib_extra_dirs = ${PROJECT_DIR}/../Arduino/libraries
```

PlatformIO จึง compile `../Arduino/libraries/MomoJoy` ตรง ๆ ซึ่งเป็นไฟล์ชุดเดียวกับที่
Arduino IDE ใช้ ส่วน `src/app_*.cpp` แต่ละไฟล์มีแค่สองบรรทัด คือ include ตัวอย่างที่ต้องการ:

```cpp
#include "../../Arduino/libraries/MomoJoy/examples/02_ReadAllSerial/02_ReadAllSerial.ino"
```

แก้ library หรือไฟล์ตัวอย่างที่ `../Arduino/libraries/MomoJoy/` ที่เดียว ทั้งสอง toolchain
เห็นการเปลี่ยนแปลงทันที

## 4. ค่าบอร์ด (ตั้งไว้ให้แล้วใน platformio.ini)

```ini
board = esp32-s3-devkitc-1
board_build.arduino.memory_type = qio_opi   ; N16R8: QIO flash + OPI PSRAM
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
build_flags = -DARDUINO_USB_CDC_ON_BOOT=0   ; Serial -> UART0 -> CH343P
```

ตัวสำคัญที่สุดคือ `ARDUINO_USB_CDC_ON_BOOT=0` ถ้าไม่มีบรรทัดนี้ `Serial` จะไปออก native USB
port ของ ESP32-S3 แล้วจะไม่เห็นอะไรเลยที่ port ของ CH343P — และต้องเสียบสายที่ port **UART**

`monitor_rts = 0` / `monitor_dtr = 0` กันไม่ให้บอร์ด reset ตอนเปิด monitor
ถ้าอยากได้ auto-reset ให้ลบสองบรรทัดนี้ออก

## 5. ผลการ build ที่ยืนยันแล้ว

```text
RAM:   [=         ]   9.9% (used 32368 bytes from 327680 bytes)
Flash: [=         ]   8.1% (used 529153 bytes from 6553600 bytes)
Building .pio/build/readall/firmware.bin
========================= [SUCCESS] Took 27.53 seconds =========================
```

## 6. แก้ปัญหา (troubleshooting)

| อาการ | วิธีแก้ |
| --- | --- |
| build ครั้งแรกขึ้น `HTTPClientError` | ไม่มีเน็ต หรือ proxy block registry ของ PlatformIO |
| Serial monitor ว่างเปล่า | เสียบสายผิด port (ไปเสียบ USB-OTG แทน UART) |
| บอร์ด boot-loop | `board_build.arduino.memory_type` ต้องเป็น `qio_opi` สำหรับ N16R8 |
| `pio test -e native` หา header ไม่เจอ | ต้องรันในโฟลเดอร์ `PlatformIO/` ไม่ใช่ root ของ repo |
| upload ไม่ผ่านที่ 921600 baud | ลด `upload_speed` เหลือ 460800 |

## อ่านต่อ

[docs/PAIRING.md](../docs/PAIRING.md) ·
[docs/CALIBRATION.md](../docs/CALIBRATION.md) ·
[docs/API.md](../docs/API.md)
