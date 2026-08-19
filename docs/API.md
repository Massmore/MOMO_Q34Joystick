# MomoJoy API Reference

```cpp
#include <MomoJoy.h>
using namespace momojoy;
```

`MomoJoy` เป็น global object ตัวเดียว รองรับจอย 1 ตัวต่อบอร์ด

## สารบัญ

- [Lifecycle](#lifecycle)
- [สถานะการเชื่อมต่อ (connection state)](#สถานะการเชื่อมต่อ-connection-state)
- [การอ่านค่า](#การอ่านค่า)
- [Callback](#callback)
- [การปรับแต่ง (tuning)](#การปรับแต่ง-tuning)
- [Debug / introspection](#debug--introspection)
- [ชั้น core (ใช้แยกได้ / unit test ได้)](#ชั้น-core-ใช้แยกได้--unit-test-ได้)

## Lifecycle

```cpp
struct MomoJoyOptions {
  const char* localName        = "MOMO";   // ชื่อ BLE ของ ESP32 ตอน pairing
  const char* nameFilter       = nullptr;  // connect เฉพาะจอยที่ชื่อมีคำนี้ (nullptr = ตัวไหนก็ได้)
  const char* addressFilter    = nullptr;  // connect เฉพาะ MAC นี้ "AA:BB:CC:DD:EE:FF"
  bool        autoReconnect    = true;     // หลุดแล้ว rescan ต่อเอง
  bool        clearBondsOnBoot = false;    // true = ลบ bond ทั้งหมดทุกครั้งที่ boot
  uint16_t    deadzone         = 24;       // 0..512
  uint16_t    connectTimeoutSec= 10;
  const MomoProfile* profile   = &kProfileAndroidGamepad;
  bool        verbose          = true;     // log ออก Serial
};

bool MomoJoy.begin(const MomoJoyOptions& = MomoJoyOptions());
void MomoJoy.end();
void MomoJoy.update();      // ต้องเรียกทุกรอบใน loop() และไม่ block
```

## สถานะการเชื่อมต่อ (connection state)

```cpp
bool        MomoJoy.isConnected();
bool        MomoJoy.isScanning();
const char* MomoJoy.peerName();      // ชื่อจอยที่ connect อยู่
const char* MomoJoy.peerAddress();   // MAC address ของจอย
void        MomoJoy.disconnect();
void        MomoJoy.forgetBonds();   // ลบ bond ทั้งหมดออกจาก NVS
```

## การอ่านค่า

```cpp
int16_t  MomoJoy.lx(), ly(), rx(), ry();   // -512 .. 511
uint16_t MomoJoy.l2(), r2();               // 0 .. 1023
uint8_t  MomoJoy.dpad();                   // bit mask MOMO_DPAD_*
bool     MomoJoy.dpadUp(), dpadDown(), dpadLeft(), dpadRight();
uint8_t  MomoJoy.battery();                // 0..100 %, 0xFF = ไม่ทราบ

bool MomoJoy.pressed(uint32_t mask);       // กดค้างอยู่ตอนนี้
bool MomoJoy.justPressed(uint32_t mask);   // เพิ่งกดในรอบ update() นี้
bool MomoJoy.justReleased(uint32_t mask);

const MomoGamepadState& MomoJoy.state();   // ทั้ง struct รวม .seq ที่นับจำนวน report
```

> [!NOTE]
> `justPressed()` / `justReleased()` มีผลเฉพาะรอบ `update()` ปัจจุบัน ถ้า `loop()` เรียก
> `update()` รอบละครั้ง ก็จะทำงานตรงไปตรงมาอย่างที่คาด

### Button bit

```text
MOMO_BTN_A  B  X  Y  L1  R1  L2  R2
MOMO_BTN_SELECT  START  HOME  L3  R3
MOMO_BTN_M1  M2  C  Z  CAPTURE
```

รวมหลายปุ่มได้ด้วย `|` เช่น `MomoJoy.pressed(MOMO_BTN_L1 | MOMO_BTN_R1)` จะเป็น true
เมื่อกดปุ่มใดปุ่มหนึ่งใน 2 ปุ่มนี้

### D-pad bit

```text
MOMO_DPAD_UP  DOWN  LEFT  RIGHT
```

## Callback

```cpp
MomoJoy.onConnect([](const char* name, const char* addr) { ... });
MomoJoy.onDisconnect([]() { ... });                        // สั่งหยุดมอเตอร์ตรงนี้
MomoJoy.onButton([](uint32_t pressed, uint32_t released) { ... });
MomoJoy.onRawReport([](uint8_t rid, const uint8_t* d, size_t n) { ... });
```

callback ทั้งหมดถูกเรียกจาก `update()` คือทำงานบน task ของ `loop()` ไม่ใช่ BLE task
ดังนั้นเรียก `Serial`, `digitalWrite()` และ `delay()` ข้างในได้อย่างปลอดภัย

> [!TIP]
> callback เป็น plain function pointer — ใช้ lambda ที่ไม่ capture ได้

## การปรับแต่ง (tuning)

```cpp
void MomoJoy.setDeadzone(uint16_t dz);
void MomoJoy.setProfile(const MomoProfile* p);
```

## Debug / introspection

```cpp
void MomoJoy.dumpReportMap(Serial);         // HID report map ดิบ
void MomoJoy.dumpDescriptorFields(Serial);  // ตาราง field ที่ parse ได้
void MomoJoy.printState(Serial);            // พิมพ์สถานะปัจจุบัน 1 บรรทัด
void MomoJoy.printStateChanges(Serial);     // พิมพ์เฉพาะตอนค่าเปลี่ยน

const MomoHidParser& MomoJoy.descriptor();
const uint8_t*       MomoJoy.reportMap();
size_t               MomoJoy.reportMapLen();
```

## ชั้น core (ใช้แยกได้ / unit test ได้)

```cpp
#include <core/MomoHidParser.h>

MomoHidParser p;
p.parse(reportMapBytes, len);
const HidField* x = p.find(kPageGenericDesktop, kUsageX);
int32_t raw = hidExtract(payload, payloadLen, *x);
int16_t val = scaleAxis(raw, x->logicalMin, x->logicalMax);
```

| ฟังก์ชัน | ทำอะไร |
| --- | --- |
| `MomoHidParser::parse()` | decode HID report descriptor → ตาราง `HidField` |
| `MomoHidParser::find(page, usage, rid)` | หา field จาก usage |
| `hidExtract(data, len, field)` | ดึง bit ออกมา (LSB-first + sign extend) |
| `scaleAxis(raw, min, max)` | → −512..511 |
| `scaleTrigger(raw, min, max)` | → 0..1023 |
| `hatToDpad(v, min, max)` | hat switch → D-pad bit |
| `applyDeadzone(x, y, dz)` | radial dead zone |
| `MomoMapper::decode()` | report → `MomoGamepadState` |

ทั้งหมดเป็น C++ ล้วน: ไม่มี dynamic allocation ไม่พึ่ง Arduino และไม่ใช้ exception

## อ่านต่อ

[CALIBRATION.md](CALIBRATION.md) · [MIGRATION_FROM_BLUEPAD32.md](MIGRATION_FROM_BLUEPAD32.md)
