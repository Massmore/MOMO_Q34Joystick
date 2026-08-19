# MomoJoy API

> English → [../en/API.md](../en/API.md)

```cpp
#include <MomoJoy.h>
using namespace momojoy;
```

อ็อบเจกต์ global ชื่อ `MomoJoy` (ตัวเดียว รองรับจอย 1 ตัวต่อบอร์ด)

---

## เริ่มต้น

```cpp
struct MomoJoyOptions {
  const char* localName        = "MOMO";   // ชื่อ BLE ของ ESP32 (ใช้ตอนจับคู่)
  const char* nameFilter       = nullptr;  // ต่อเฉพาะจอยที่ชื่อมีคำนี้ (nullptr = ตัวไหนก็ได้)
  const char* addressFilter    = nullptr;  // ต่อเฉพาะ MAC นี้ "AA:BB:CC:DD:EE:FF"
  bool        autoReconnect    = true;     // หลุดแล้วสแกนต่อเอง
  bool        clearBondsOnBoot = false;    // true = ลืมการจับคู่ทุกครั้งที่บูต
  uint16_t    deadzone         = 24;       // 0..512
  uint16_t    connectTimeoutSec= 10;
  const MomoProfile* profile   = &kProfileAndroidGamepad;
  bool        verbose          = true;     // log ออก Serial
};

bool MomoJoy.begin(const MomoJoyOptions& = MomoJoyOptions());
void MomoJoy.end();
void MomoJoy.update();      // ต้องเรียกทุกรอบใน loop() — ไม่บล็อก
```

## สถานะการเชื่อมต่อ

```cpp
bool        MomoJoy.isConnected();
bool        MomoJoy.isScanning();
const char* MomoJoy.peerName();      // ชื่อจอยที่ต่ออยู่
const char* MomoJoy.peerAddress();   // MAC
void        MomoJoy.disconnect();
void        MomoJoy.forgetBonds();   // ลบการจับคู่ทั้งหมดใน NVS
```

## อ่านค่า

```cpp
int16_t  MomoJoy.lx(), ly(), rx(), ry();   // -512 .. 511
uint16_t MomoJoy.l2(), r2();               // 0 .. 1023
uint8_t  MomoJoy.dpad();                   // บิต MOMO_DPAD_*
bool     MomoJoy.dpadUp(), dpadDown(), dpadLeft(), dpadRight();
uint8_t  MomoJoy.battery();                // 0..100 %, 0xFF = ไม่รู้

bool MomoJoy.pressed(uint32_t mask);       // กดอยู่ตอนนี้
bool MomoJoy.justPressed(uint32_t mask);   // เพิ่งกดในรอบ update() นี้
bool MomoJoy.justReleased(uint32_t mask);

const MomoGamepadState& MomoJoy.state();   // ทั้งก้อน (มี .seq นับจำนวน report)
```

> `justPressed()` / `justReleased()` มีผลเฉพาะรอบ `update()` ปัจจุบัน
> ถ้า `loop()` เรียก `update()` รอบเดียวต่อรอบ ก็ใช้ได้ตรงไปตรงมา

### บิตปุ่ม

```
MOMO_BTN_A  B  X  Y  L1  R1  L2  R2
MOMO_BTN_SELECT  START  HOME  L3  R3
MOMO_BTN_M1  M2  C  Z  CAPTURE
```

รวมหลายปุ่มได้: `MomoJoy.pressed(MOMO_BTN_L1 | MOMO_BTN_R1)` = กดอย่างน้อยหนึ่งใน 2 ปุ่ม

### บิต D-Pad

```
MOMO_DPAD_UP  DOWN  LEFT  RIGHT
```

## Callback

```cpp
MomoJoy.onConnect([](const char* name, const char* addr) { ... });
MomoJoy.onDisconnect([]() { ... });                       // ⚠️ สั่งหยุดมอเตอร์ตรงนี้
MomoJoy.onButton([](uint32_t pressed, uint32_t released) { ... });
MomoJoy.onRawReport([](uint8_t rid, const uint8_t* d, size_t n) { ... });
```

Callback ทั้งหมดถูกเรียกจาก `update()` (คือใน task ของ `loop()`) ไม่ใช่จาก BLE task
→ เรียก `Serial`, `digitalWrite`, `delay` ได้ตามปกติ

> callback ต้องเป็น function pointer (lambda ที่ไม่ capture ก็ได้)

## ปรับแต่ง

```cpp
void MomoJoy.setDeadzone(uint16_t dz);
void MomoJoy.setProfile(const MomoProfile* p);
```

## Debug / introspection

```cpp
void MomoJoy.dumpReportMap(Serial);         // HID Report Map ดิบ
void MomoJoy.dumpDescriptorFields(Serial);  // ตาราง field ที่ parse ได้
void MomoJoy.printState(Serial);            // พิมพ์สถานะปัจจุบัน 1 บรรทัด
void MomoJoy.printStateChanges(Serial);     // พิมพ์เฉพาะตอนค่าเปลี่ยน

const MomoHidParser& MomoJoy.descriptor();
const uint8_t*       MomoJoy.reportMap();
size_t               MomoJoy.reportMapLen();
```

---

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
|---|---|
| `MomoHidParser::parse()` | ถอด HID Report Descriptor → ตาราง `HidField` |
| `MomoHidParser::find(page, usage, rid)` | หา field ตาม usage |
| `hidExtract(data, len, field)` | ดึงบิตออกมา (LSB-first + sign extend) |
| `scaleAxis(raw, min, max)` | → −512..511 |
| `scaleTrigger(raw, min, max)` | → 0..1023 |
| `hatToDpad(v, min, max)` | hat switch → บิต D-Pad |
| `applyDeadzone(x, y, dz)` | deadzone แบบวงกลม |
| `MomoMapper::decode()` | report → `MomoGamepadState` |

ทั้งหมดเป็น C++ ล้วน ไม่มี dynamic allocation ไม่พึ่ง Arduino
