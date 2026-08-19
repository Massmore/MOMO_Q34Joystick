# MomoJoy — PlatformIO (VS Code)

> English → [README.md](README.md)

## 1. ติดตั้ง

1. [VS Code](https://code.visualstudio.com/)
2. VS Code → Extensions → ค้นหา **PlatformIO IDE** → Install
3. **ไดรเวอร์ CH343P** — ดู [docs/th/INSTALL.md](../docs/th/INSTALL.md#ไดรเวอร์-ch343p)
4. เปิด **โฟลเดอร์ `PlatformIO/` นี้** ใน VS Code (ไม่ใช่ราก repo)
   ครั้งแรก PlatformIO จะโหลด `espressif32` กับ `NimBLE-Arduino` ให้เอง (ประมาณ 5–10 นาที)

## 2. คำสั่งของแต่ละ environment

| คำสั่ง | ทำอะไร |
|---|---|
| `pio run -e rawdump -t upload -t monitor` | โหมด calibrate พิมพ์ HID report map และไบต์ดิบ |
| `pio run -e readall -t upload -t monitor` | ตัวหลัก พิมพ์ปุ่ม แกน ไก แบต ครบ |
| `pio run -e robot -t upload` | ตัวอย่างคุมหุ่น 2 มอเตอร์ |
| `pio run -e minimal -t upload` | โครงสั้นที่สุด |
| `pio test -e native` | unit test 16 เคสบน PC ไม่ต้องมีบอร์ด |

`readall` เป็น env เริ่มต้น พิมพ์ `pio run` เฉย ๆ ก็คือ build ตัวนี้

## 3. โค้ดอยู่ตรงไหน

```
PlatformIO/
├── platformio.ini
├── src/app_*.cpp        entry point บาง ๆ หนึ่งไฟล์ต่อหนึ่ง env
└── test/test_core/      unit test ของ core ที่เป็น C++ ล้วน
```

ตัวไลบรารี **ไม่ได้ copy ซ้ำ** มาไว้ที่นี่ ใน `platformio.ini` มีบรรทัด

```ini
lib_extra_dirs = ${PROJECT_DIR}/../Arduino/libraries
```

PlatformIO จึงคอมไพล์ `../Arduino/libraries/MomoJoy` ตรง ๆ ซึ่งเป็นไฟล์ชุดเดียวกับที่
Arduino IDE ใช้ ส่วน `src/app_*.cpp` แต่ละไฟล์มีแค่สองบรรทัด include ตัวอย่างที่ต้องการ:

```cpp
#include "../../Arduino/libraries/MomoJoy/examples/02_ReadAllSerial/02_ReadAllSerial.ino"
```

แก้ไลบรารีหรือตัวอย่างที่ `../Arduino/libraries/MomoJoy/` ที่เดียว ทั้งสอง toolchain เห็นทันที

## 4. ค่าบอร์ด (ตั้งไว้ให้แล้วใน platformio.ini)

```ini
board = esp32-s3-devkitc-1
board_build.arduino.memory_type = qio_opi   ; N16R8: flash QIO + PSRAM OPI
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
build_flags = -DARDUINO_USB_CDC_ON_BOOT=0   ; Serial -> UART0 -> CH343P
```

ตัวสำคัญคือ `ARDUINO_USB_CDC_ON_BOOT=0` ถ้าไม่มี `Serial` จะไปออกพอร์ต USB ในตัวของ
ESP32-S3 แล้วจะไม่เห็นอะไรเลยที่พอร์ต CH343P — เสียบสายที่พอร์ต **UART**

`monitor_rts = 0` / `monitor_dtr = 0` กันไม่ให้บอร์ดรีเซ็ตตอนเปิด monitor
ถ้าอยากให้รีเซ็ตอัตโนมัติ ให้ลบสองบรรทัดนี้

## 5. ผลการ build ที่ยืนยันแล้ว

```
RAM:   [=         ]   9.9% (used 32368 bytes from 327680 bytes)
Flash: [=         ]   8.1% (used 529153 bytes from 6553600 bytes)
Building .pio/build/readall/firmware.bin
========================= [SUCCESS] Took 27.53 seconds =========================
```

## 6. แก้ปัญหา

| อาการ | วิธีแก้ |
|---|---|
| ครั้งแรก build แล้วขึ้น `HTTPClientError` | ไม่มีเน็ต หรือ proxy บล็อก registry ของ PlatformIO |
| Serial monitor ว่าง | เสียบสายผิดพอร์ต (ไปเสียบ USB-OTG แทน UART) |
| บอร์ดบูตวน | `board_build.arduino.memory_type` ต้องเป็น `qio_opi` สำหรับ N16R8 |
| `pio test -e native` หา header ไม่เจอ | ต้องรันในโฟลเดอร์ `PlatformIO/` ไม่ใช่รากของ repo |
| อัปโหลดไม่ผ่านที่ 921600 | ลด `upload_speed` เหลือ 460800 |

เพิ่มเติม: [docs/th/PAIRING.md](../docs/th/PAIRING.md) · [docs/th/CALIBRATION.md](../docs/th/CALIBRATION.md) · [docs/th/API.md](../docs/th/API.md)
