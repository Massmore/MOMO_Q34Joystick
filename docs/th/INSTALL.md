# การติดตั้ง (ภาษาไทย)

> English → [../en/INSTALL.md](../en/INSTALL.md)

## 1. ของที่ต้องมี

| ของ | รายละเอียด |
|---|---|
| บอร์ด | ESP32-**S3** N16R8 (Flash 16 MB QIO + PSRAM 8 MB OPI) — ต้องเป็นชิปที่มี **BLE** |
| จอย | ShanWan **Q34U** (สวิตช์เลื่อน 4 ตำแหน่ง X / S / P / **D**) |
| สาย | USB-C เสียบที่พอร์ต **UART** ของบอร์ด (พอร์ตที่ต่อชิป **CH343P**) |

> ⚠️ บอร์ด ESP32-S3 ส่วนใหญ่มีพอร์ต USB สองช่อง: พอร์ต **UART** (ผ่าน CH343P) กับพอร์ต
> **USB-OTG** (ต่อตรงเข้าชิป) ไลบรารีนี้ตั้งให้ `Serial` ออกทาง **UART0 → CH343P**
> ให้เสียบสายที่พอร์ต UART

## 2. ไดรเวอร์ CH343P

ต้องลงเอง ระบบปฏิบัติการไม่ได้มีมาให้ (ยกเว้น Linux ใหม่ ๆ)

| ระบบ | ทำอย่างไร |
|---|---|
| Windows | ติดตั้ง [CH343SER.EXE จาก WCH](https://www.wch-ic.com/downloads/CH343SER_EXE.html) |
| macOS | macOS 13 ขึ้นไปมักเจอเอง ถ้าไม่เจอใช้ [ch34xser_macos](https://github.com/WCHSoftGroup/ch34xser_macos) |
| Linux | เคอร์เนล ≥ 5.13 มีไดรเวอร์ `ch343` มาให้แล้ว ถ้าไม่มีใช้ [ch343ser_linux](https://github.com/WCHSoftGroup/ch343ser_linux) และอย่าลืม `sudo usermod -aG dialout $USER` แล้ว log out/in |

ตรวจว่าเจอหรือยัง:

- Windows: Device Manager → Ports (COM & LPT) → `USB-Enhanced-SERIAL CH343`
- macOS: `ls /dev/cu.wchusbserial*`
- Linux: `ls /dev/ttyACM*` หรือ `dmesg | grep ch343`

## 3. ทางที่ 1 — PlatformIO (แนะนำ)

1. ติดตั้ง [VS Code](https://code.visualstudio.com/)
2. VS Code → Extensions → ค้นหา `PlatformIO IDE` → Install
3. เปิดโฟลเดอร์ **`PlatformIO/`** ของ repo นี้ (ไม่ใช่รากของ repo)
4. ครั้งแรกจะโหลด platform `espressif32@6.9.0` กับ `NimBLE-Arduino 1.4.x` ให้เอง (5–10 นาที)
5. ทดสอบว่าทุกอย่างพร้อม โดยยังไม่ต้องเสียบบอร์ด:

```bash
cd PlatformIO
pio test -e native      # ควรได้ 16/16 PASSED
```

6. อัปโหลดจริง:

```bash
pio run -e rawdump -t upload -t monitor
```

## 4. ทางที่ 2 — Arduino IDE

1. ติดตั้ง Arduino IDE 2.x
2. Boards Manager → ติดตั้ง **esp32 by Espressif Systems** (ทดสอบกับ 2.0.17)
3. Library Manager → ติดตั้ง **NimBLE-Arduino** (ทดสอบกับ 1.4.3)
4. คัดลอกโฟลเดอร์ `Arduino/libraries/MomoJoy` ไปที่โฟลเดอร์ libraries ของคุณ:

   | ระบบ | ที่อยู่ |
   |---|---|
   | Windows | `Documents\Arduino\libraries\MomoJoy` |
   | macOS | `~/Documents/Arduino/libraries/MomoJoy` |
   | Linux | `~/Arduino/libraries/MomoJoy` |

5. เลือกบอร์ด **ESP32S3 Dev Module** แล้วตั้งค่า:

   | ช่อง | ค่า |
   |---|---|
   | **USB CDC On Boot** | **Disabled** ← ถ้าไม่ปิด Serial จะไม่ออก CH343P |
   | Flash Size | 16MB (128Mb) |
   | PSRAM | OPI PSRAM |
   | Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
   | Upload Speed | 921600 |

6. File → Examples → MomoJoy → `01_RawDump`

## 5. เวอร์ชันที่ทดสอบแล้ว

| ส่วนประกอบ | เวอร์ชัน |
|---|---|
| platform espressif32 (PlatformIO) | 6.9.0 |
| arduino-esp32 core | 2.0.17 |
| NimBLE-Arduino | 1.4.3 |
| xtensa-esp32s3-elf-g++ | 8.4.0 (esp-2021r2-patch5) |

เวอร์ชันอื่นน่าจะใช้ได้ แต่ยังไม่ได้ทดสอบ โดยเฉพาะ arduino-esp32 3.x และ NimBLE 2.x
ซึ่ง API ของ NimBLE เปลี่ยนไปพอสมควร

## 6. ตรวจว่าถูกต้อง

```bash
# unit test ของ core — ไม่ต้องมีบอร์ด
cd PlatformIO && pio test -e native

# หรือแบบไม่ง้อ PlatformIO เลย (ใช้ g++ + sanitizer)
./tools/run_native_tests.sh

# syntax check ชั้น NimBLE ด้วย stub header
./tools/check_esp_layer.sh
```
