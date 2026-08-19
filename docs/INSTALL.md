# การติดตั้ง (Installation)

ติดตั้ง toolchain, driver และตั้งค่าบอร์ดให้พร้อมใช้ MomoJoy

## สารบัญ

- [1. ของที่ต้องมี](#1-ของที่ต้องมี)
- [2. CH343P USB-serial driver](#2-ch343p-usb-serial-driver)
- [3. ทางเลือกที่ 1 — PlatformIO (แนะนำ)](#3-ทางเลือกที่-1--platformio-แนะนำ)
- [4. ทางเลือกที่ 2 — Arduino IDE](#4-ทางเลือกที่-2--arduino-ide)
- [5. เวอร์ชันที่ทดสอบแล้ว](#5-เวอร์ชันที่ทดสอบแล้ว)
- [6. ตรวจสอบว่าติดตั้งถูกต้อง](#6-ตรวจสอบว่าติดตั้งถูกต้อง)

## 1. ของที่ต้องมี

| ของ | รายละเอียด |
| --- | --- |
| บอร์ด | ESP32-**S3** N16R8 (flash 16 MB QIO + PSRAM 8 MB OPI) — ใช้ ESP32 ตัวไหนก็ได้ที่มี BLE |
| จอย | ShanWan **Q34U** (สวิตช์เลื่อน 4 ตำแหน่ง X / S / P / **D**) |
| สาย | USB-C เสียบที่ port **UART** ของบอร์ด (port ที่ต่อกับชิป **CH343P**) |

> [!WARNING]
> บอร์ด ESP32-S3 ส่วนใหญ่มี USB สอง port คือ port **UART** (ผ่าน CH343P) กับ port
> **USB-OTG** (ต่อตรงเข้าชิป) MomoJoy ตั้งให้ `Serial` ออกทาง **UART0 → CH343P**
> จึงต้องเสียบสายที่ port UART

<a id="ch343p-driver"></a>

## 2. CH343P USB-serial driver

driver ตัวนี้ต้องติดตั้งเอง ระบบปฏิบัติการไม่ได้มีมาให้ (ยกเว้น Linux รุ่นใหม่ ๆ)

| OS | วิธีติดตั้ง |
| --- | --- |
| Windows | ติดตั้ง [CH343SER.EXE จาก WCH](https://www.wch-ic.com/downloads/CH343SER_EXE.html) |
| macOS | macOS 13 ขึ้นไปมักตรวจเจอเอง ถ้าไม่เจอให้ใช้ [ch34xser_macos](https://github.com/WCHSoftGroup/ch34xser_macos) |
| Linux | kernel ≥ 5.13 มี driver `ch343` มาให้แล้ว ถ้าไม่มีให้ใช้ [ch343ser_linux](https://github.com/WCHSoftGroup/ch343ser_linux) และอย่าลืมรัน `sudo usermod -aG dialout $USER` แล้ว log out/in |

ตรวจว่าระบบมองเห็นแล้วหรือยัง:

- **Windows** — Device Manager → Ports (COM & LPT) → `USB-Enhanced-SERIAL CH343`
- **macOS** — `ls /dev/cu.wchusbserial*`
- **Linux** — `ls /dev/ttyACM*` หรือ `dmesg | grep ch343`

## 3. ทางเลือกที่ 1 — PlatformIO (แนะนำ)

1. ติดตั้ง [VS Code](https://code.visualstudio.com/)
2. VS Code → Extensions → ค้นหา `PlatformIO IDE` → Install
3. เปิดโฟลเดอร์ **`PlatformIO/`** ของ repo นี้ (ไม่ใช่ root ของ repo)
4. การ build ครั้งแรกจะ download `espressif32@6.9.0` และ `NimBLE-Arduino 1.4.x` (5–10 นาที)
5. ทดสอบว่าทุกอย่างพร้อม โดยยังไม่ต้องเสียบบอร์ด:

   ```bash
   cd PlatformIO
   pio test -e native      # ควรได้ 16/16 PASSED
   ```

6. upload ลงบอร์ดจริง:

   ```bash
   pio run -e rawdump -t upload -t monitor
   ```

## 4. ทางเลือกที่ 2 — Arduino IDE

1. ติดตั้ง Arduino IDE 2.x
2. Boards Manager → ติดตั้ง **esp32 by Espressif Systems** (ทดสอบกับ 2.0.17)
3. Library Manager → ติดตั้ง **NimBLE-Arduino** (ทดสอบกับ 1.4.3)
4. คัดลอกโฟลเดอร์ `Arduino/libraries/MomoJoy` ไปที่โฟลเดอร์ libraries ของคุณ:

   | OS | Path |
   | --- | --- |
   | Windows | `Documents\Arduino\libraries\MomoJoy` |
   | macOS | `~/Documents/Arduino/libraries/MomoJoy` |
   | Linux | `~/Arduino/libraries/MomoJoy` |

5. เลือกบอร์ด **ESP32S3 Dev Module** แล้วตั้งค่า:

   | Setting | ค่า |
   | --- | --- |
   | **USB CDC On Boot** | **Disabled** ← ถ้าไม่ปิด `Serial` จะไม่ออกทาง CH343P |
   | Flash Size | 16MB (128Mb) |
   | PSRAM | OPI PSRAM |
   | Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
   | Upload Speed | 921600 |

6. File → Examples → MomoJoy → `01_RawDump`

## 5. เวอร์ชันที่ทดสอบแล้ว

| ส่วนประกอบ | เวอร์ชัน |
| --- | --- |
| platform espressif32 (PlatformIO) | 6.9.0 |
| arduino-esp32 core | 2.0.17 |
| NimBLE-Arduino | 1.4.3 |
| xtensa-esp32s3-elf-g++ | 8.4.0 (esp-2021r2-patch5) |

เวอร์ชันอื่นน่าจะใช้ได้แต่ยังไม่ได้ทดสอบ โดยเฉพาะ arduino-esp32 3.x และ NimBLE 2.x
ซึ่ง API ของ NimBLE เปลี่ยนไปพอสมควร

## 6. ตรวจสอบว่าติดตั้งถูกต้อง

```bash
# unit test ของ core — ไม่ต้องมีบอร์ด
cd PlatformIO && pio test -e native

# ชุดเดียวกันแบบไม่ง้อ PlatformIO (ใช้ g++ + sanitizer)
./tools/run_native_tests.sh

# syntax check ชั้น NimBLE ด้วย stub header
./tools/check_esp_layer.sh
```

## อ่านต่อ

[PAIRING.md](PAIRING.md) · [HARDWARE.md](HARDWARE.md) · [API.md](API.md)
