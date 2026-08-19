# MomoJoy API

> ภาษาไทย → [../th/API.md](../th/API.md)

```cpp
#include <MomoJoy.h>
using namespace momojoy;
```

`MomoJoy` is a global object. One controller per board.

---

## Lifecycle

```cpp
struct MomoJoyOptions {
  const char* localName        = "MOMO";   // BLE name of the ESP32 during pairing
  const char* nameFilter       = nullptr;  // only connect if the name contains this
  const char* addressFilter    = nullptr;  // only connect to "AA:BB:CC:DD:EE:FF"
  bool        autoReconnect    = true;     // rescan after a disconnect
  bool        clearBondsOnBoot = false;    // true = forget all pairings on boot
  uint16_t    deadzone         = 24;       // 0..512
  uint16_t    connectTimeoutSec= 10;
  const MomoProfile* profile   = &kProfileAndroidGamepad;
  bool        verbose          = true;     // log to Serial
};

bool MomoJoy.begin(const MomoJoyOptions& = MomoJoyOptions());
void MomoJoy.end();
void MomoJoy.update();      // call every loop() iteration; never blocks
```

## Connection state

```cpp
bool        MomoJoy.isConnected();
bool        MomoJoy.isScanning();
const char* MomoJoy.peerName();      // name of the connected controller
const char* MomoJoy.peerAddress();   // its MAC address
void        MomoJoy.disconnect();
void        MomoJoy.forgetBonds();   // erase all bonds from NVS
```

## Reading values

```cpp
int16_t  MomoJoy.lx(), ly(), rx(), ry();   // -512 .. 511
uint16_t MomoJoy.l2(), r2();               // 0 .. 1023
uint8_t  MomoJoy.dpad();                   // MOMO_DPAD_* bit mask
bool     MomoJoy.dpadUp(), dpadDown(), dpadLeft(), dpadRight();
uint8_t  MomoJoy.battery();                // 0..100 %, 0xFF = unknown

bool MomoJoy.pressed(uint32_t mask);       // held right now
bool MomoJoy.justPressed(uint32_t mask);   // pressed during this update()
bool MomoJoy.justReleased(uint32_t mask);

const MomoGamepadState& MomoJoy.state();   // whole struct, incl. .seq report counter
```

> `justPressed()` / `justReleased()` are valid only for the current `update()` call. With
> one `update()` per `loop()` iteration this works exactly as you would expect.

### Button bits

```
MOMO_BTN_A  B  X  Y  L1  R1  L2  R2
MOMO_BTN_SELECT  START  HOME  L3  R3
MOMO_BTN_M1  M2  C  Z  CAPTURE
```

Combine them: `MomoJoy.pressed(MOMO_BTN_L1 | MOMO_BTN_R1)` is true if either is held.

### D-pad bits

```
MOMO_DPAD_UP  DOWN  LEFT  RIGHT
```

## Callbacks

```cpp
MomoJoy.onConnect([](const char* name, const char* addr) { ... });
MomoJoy.onDisconnect([]() { ... });                        // stop your motors here
MomoJoy.onButton([](uint32_t pressed, uint32_t released) { ... });
MomoJoy.onRawReport([](uint8_t rid, const uint8_t* d, size_t n) { ... });
```

All callbacks run from `update()`, i.e. on the `loop()` task — not on the BLE task. `Serial`,
`digitalWrite()` and `delay()` are all safe inside them.

> Callbacks are plain function pointers; a non-capturing lambda works.

## Tuning

```cpp
void MomoJoy.setDeadzone(uint16_t dz);
void MomoJoy.setProfile(const MomoProfile* p);
```

## Debug / introspection

```cpp
void MomoJoy.dumpReportMap(Serial);         // raw HID report map
void MomoJoy.dumpDescriptorFields(Serial);  // the parsed field table
void MomoJoy.printState(Serial);            // one line with the current state
void MomoJoy.printStateChanges(Serial);     // prints only when something changes

const MomoHidParser& MomoJoy.descriptor();
const uint8_t*       MomoJoy.reportMap();
size_t               MomoJoy.reportMapLen();
```

---

## The core layer (usable standalone, unit-testable)

```cpp
#include <core/MomoHidParser.h>

MomoHidParser p;
p.parse(reportMapBytes, len);
const HidField* x = p.find(kPageGenericDesktop, kUsageX);
int32_t raw = hidExtract(payload, payloadLen, *x);
int16_t val = scaleAxis(raw, x->logicalMin, x->logicalMax);
```

| Function | Purpose |
|---|---|
| `MomoHidParser::parse()` | Decode a HID report descriptor into a `HidField` table |
| `MomoHidParser::find(page, usage, rid)` | Look up a field by usage |
| `hidExtract(data, len, field)` | Pull the bits out (LSB-first, sign-extended) |
| `scaleAxis(raw, min, max)` | → −512..511 |
| `scaleTrigger(raw, min, max)` | → 0..1023 |
| `hatToDpad(v, min, max)` | Hat switch → D-pad bits |
| `applyDeadzone(x, y, dz)` | Radial dead zone |
| `MomoMapper::decode()` | Report → `MomoGamepadState` |

All pure C++: no dynamic allocation, no Arduino dependency, no exceptions.
