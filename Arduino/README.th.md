# MomoJoy — Arduino IDE

> English → [README.md](README.md)

## 1. ติดตั้ง

### ก. ตัวไลบรารี

คัดลอกโฟลเดอร์

```
Arduino/libraries/MomoJoy
```

ไปไว้ในโฟลเดอร์ libraries ของ Arduino:

| ระบบ | ที่อยู่ |
|---|---|
| Windows | `Documents\Arduino\libraries\MomoJoy` |
| macOS | `~/Documents/Arduino/libraries/MomoJoy` |
| Linux | `~/Arduino/libraries/MomoJoy` |

แล้วปิด-เปิด Arduino IDE ใหม่ จะเห็น `MomoJoy` ใน *File → Examples*

### ข. ของที่ต้องมีเพิ่ม

| อะไร | ติดตั้งยังไง |
|---|---|
| **บอร์ด ESP32** | Boards Manager → `esp32 by Espressif Systems` (ทดสอบกับ **2.0.17**) |
| **NimBLE-Arduino** | Library Manager → `NimBLE-Arduino` (ทดสอบกับ **1.4.3**) |
| **ไดรเวอร์ CH343P** | ดู [docs/th/INSTALL.md](../docs/th/INSTALL.md#ไดรเวอร์-ch343p) |

## 2. ตั้งค่าบอร์ด — ตรงนี้สำคัญมาก

เลือก **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module** แล้วตั้ง:

| ช่อง | ค่า | ทำไม |
|---|---|---|
| **USB CDC On Boot** | **Disabled** | ถ้าไม่ปิด `Serial` จะออกพอร์ต USB-OTG แล้วจะไม่เห็นอะไรเลยที่พอร์ต CH343P |
| Flash Size | 16MB (128Mb) | N16R8 |
| PSRAM | OPI PSRAM | N16R8 — ตั้งผิดบอร์ดจะบูตวน |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) | |
| Upload Speed | 921600 | |
| Core Debug Level | None หรือ Error | |

เสียบสาย USB ที่พอร์ต **UART** ของบอร์ด (พอร์ตที่ต่อกับชิป CH343P)

## 3. ลองใช้

1. *File → Examples → MomoJoy → **01_RawDump*** — อัปโหลด เปิด Serial Monitor ที่
   **115200** เลื่อนสวิตช์จอย Q34U ไปที่ **D** แล้วกด **HOME** ค้างจนไฟกระพริบเร็ว
   จะเห็น HID report map และไบต์ดิบทุกครั้งที่กดปุ่ม
2. *File → Examples → MomoJoy → **02_ReadAllSerial*** — ตัวหลัก พิมพ์ปุ่ม แกน ไก
   และระดับแบตออกมาครบ
3. *03_MomoRobot* — ตัวอย่างคุมหุ่น 2 มอเตอร์
4. *04_Minimal* — โครงสั้นที่สุดเอาไปต่อยอด

## 4. ผลลัพธ์ที่ควรได้ครั้งแรก

```
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

## 5. แก้ปัญหา

| อาการ | วิธีแก้ |
|---|---|
| Serial Monitor ว่างเปล่า | ยังไม่ได้ตั้ง **USB CDC On Boot** เป็น *Disabled* หรือเสียบสายผิดพอร์ต (ไปเสียบพอร์ต USB-OTG) |
| บอร์ดรีบูตวนตั้งแต่ยังไม่ถึง `setup()` | ตั้ง PSRAM ผิด ต้องเป็น **OPI PSRAM** สำหรับ N16R8 |
| ขึ้น `ambiguating new declaration of 'void ...'` | ใส่อักขระที่ไม่ใช่ ASCII (เช่นภาษาไทย) ลงใน `.ino` ให้เขียน `.ino` เป็น ASCII ล้วน หรือประกาศ prototype ของฟังก์ชันเอง |
| `NimBLEDevice.h: No such file` | ยังไม่ได้ติดตั้ง NimBLE-Arduino |
| สแกนเจอแต่ต่อไม่ติด | การจับคู่เดิมค้าง ตั้ง `opt.clearBondsOnBoot = true` สักครั้ง แล้วรีเซ็ตจอย |

เพิ่มเติม: [docs/th/PAIRING.md](../docs/th/PAIRING.md) · [docs/th/CALIBRATION.md](../docs/th/CALIBRATION.md) · [docs/th/API.md](../docs/th/API.md)
