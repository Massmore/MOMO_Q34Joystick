# MomoJoy — Arduino IDE

คู่มือเริ่มต้นใช้งาน MomoJoy บน Arduino IDE 2.x
(ถ้าใช้ VS Code + PlatformIO ให้ดู [../PlatformIO/README.md](../PlatformIO/README.md) แทน)

## 1. ติดตั้ง

### 1.1 ตัว library

คัดลอกทั้งโฟลเดอร์

```text
Arduino/libraries/MomoJoy
```

ไปไว้ในโฟลเดอร์ libraries ของ Arduino:

| OS | Path |
| --- | --- |
| Windows | `Documents\Arduino\libraries\MomoJoy` |
| macOS | `~/Documents/Arduino/libraries/MomoJoy` |
| Linux | `~/Arduino/libraries/MomoJoy` |

จากนั้นปิด-เปิด Arduino IDE ใหม่ จะเห็น `MomoJoy` โผล่ใน *File → Examples*

### 1.2 Dependency

| อะไร | ติดตั้งอย่างไร |
| --- | --- |
| **ESP32 board package** | Boards Manager → `esp32 by Espressif Systems` (ทดสอบกับ **2.0.17**) |
| **NimBLE-Arduino** | Library Manager → `NimBLE-Arduino` (ทดสอบกับ **1.4.3**) |
| **CH343P USB driver** | ดู [docs/INSTALL.md](../docs/INSTALL.md#ch343p-driver) |

## 2. ตั้งค่าบอร์ด — ตรงนี้สำคัญมาก

เลือก **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module** แล้วตั้งค่าดังนี้:

| Setting | ค่า | ทำไม |
| --- | --- | --- |
| **USB CDC On Boot** | **Disabled** | ถ้าไม่ปิด `Serial` จะออกทาง native USB-OTG port แล้วจะไม่เห็นอะไรเลยที่ port ของ CH343P |
| Flash Size | 16MB (128Mb) | สำหรับ N16R8 |
| PSRAM | OPI PSRAM | สำหรับ N16R8 — ตั้งผิดบอร์ดจะ boot-loop |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) | |
| Upload Speed | 921600 | |
| Core Debug Level | None หรือ Error | |

เสียบสาย USB ที่ port **UART** ของบอร์ด (port ที่ต่อกับชิป CH343P)

## 3. ลองใช้งาน

1. *File → Examples → MomoJoy → **01_RawDump*** — upload แล้วเปิด Serial Monitor ที่
   **115200** เลื่อนสวิตช์จอย Q34U ไปที่ **D** และกด **HOME** ค้างจนไฟ LED กระพริบเร็ว
   จะเห็น HID report map และ byte ดิบทุกครั้งที่กดปุ่ม
2. *File → Examples → MomoJoy → **02_ReadAllSerial*** — firmware ตัวหลัก พิมพ์ปุ่ม axis
   trigger และระดับ battery ออกมาครบ
3. *03_MomoRobot* — ตัวอย่างคุมหุ่น 2 มอเตอร์
4. *04_Minimal* — โครงที่สั้นที่สุด เอาไปต่อยอดใน sketch ของคุณเอง

## 4. ผลลัพธ์ที่ควรได้ครั้งแรก

```text
=============================================
 MomoJoy - ShanWan Q34U (Mode D) BLE HID host
 ESP32-S3 N16R8 / Arduino core
=============================================
[MomoJoy] BLE HID host started
[MomoJoy] scanning for a BLE HID gamepad...
[MomoJoy] connecting to ShanWan Q34u (xx:xx:xx:xx:xx:xx)
[MomoJoy] report map: 108 bytes, 22 input fields, 1 report id(s)
[MomoJoy] subscribed to 1 input report(s)
[MomoJoy] ready
```

## 5. แก้ปัญหา (troubleshooting)

| อาการ | วิธีแก้ |
| --- | --- |
| Serial Monitor ว่างเปล่า | ยังไม่ได้ตั้ง **USB CDC On Boot** เป็น *Disabled* หรือเสียบสายผิด port (ไปเสียบ USB-OTG แทน UART) |
| บอร์ด reboot วนตั้งแต่ยังไม่ถึง `setup()` | ตั้ง PSRAM ผิด ต้องเป็น **OPI PSRAM** สำหรับ N16R8 |
| ขึ้น `ambiguating new declaration of 'void ...'` | มีอักขระที่ไม่ใช่ ASCII (เช่นภาษาไทย) อยู่ในไฟล์ `.ino` ให้เขียน `.ino` เป็น ASCII ล้วน หรือประกาศ prototype ของฟังก์ชันเอง |
| `NimBLEDevice.h: No such file` | ยังไม่ได้ติดตั้ง NimBLE-Arduino |
| scan เจอแต่ connect ไม่ติด | bond เดิมค้างอยู่ ตั้ง `opt.clearBondsOnBoot = true` สักครั้ง แล้ว reset จอย |

## อ่านต่อ

[docs/PAIRING.md](../docs/PAIRING.md) ·
[docs/CALIBRATION.md](../docs/CALIBRATION.md) ·
[docs/API.md](../docs/API.md)
