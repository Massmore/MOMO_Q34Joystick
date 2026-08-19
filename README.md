# MomoJoy

**BLE HID gamepad host for the ESP32-S3, on the plain Arduino core.**

Built for the **ShanWan Q34U with its mode switch on `D`** (HID & APP / Android BLE),
but it works with other BLE HID gamepads too: MomoJoy reads and parses the
controller's **HID Report Map at runtime** instead of hard-coding one vendor's byte
layout.

Written from scratch — no Bluepad32. The only dependency is `NimBLE-Arduino`.

> ภาษาไทย → [README.th.md](README.th.md)

---

## Status

| Check | Result |
|---|---|
| `pio test -e native` — 16 unit tests for the HID parser / scaling / mapping | ✅ 16/16 |
| Same tests under AddressSanitizer + UndefinedBehaviorSanitizer | ✅ pass |
| `pio run` — all 4 environments, ESP32-S3, arduino-esp32 2.0.17 + NimBLE 1.4.3 | ✅ build + link + `.bin` |
| `arduino-cli compile` — all 4 examples, same core and library | ✅ build + link |
| Paired with real Q34U hardware | ✅ verified by the author |

Firmware size: about **530 KB flash / 32 KB RAM** for the full "print everything" build.

---

## Repository layout

```
.
├── Arduino/
│   ├── README.md               Arduino IDE quick start
│   └── libraries/MomoJoy/      <- THE library. Copy this folder into
│       ├── library.properties     Documents/Arduino/libraries/
│       ├── keywords.txt
│       ├── src/                   MomoJoy.h/.cpp, MomoJoySerial.cpp, core/
│       └── examples/              01_RawDump .. 04_Minimal
├── PlatformIO/
│   ├── README.md               PlatformIO quick start
│   ├── platformio.ini          envs: readall / rawdump / robot / minimal / native
│   ├── src/app_*.cpp           thin entry points that include the examples
│   └── test/test_core/         unit tests
├── docs/
│   ├── en/                     INSTALL, PAIRING, CALIBRATION, API, HARDWARE, MIGRATION
│   └── th/                     the same documents in Thai
├── tools/                      dev scripts that work without PlatformIO
└── .github/workflows/ci.yml    builds both platforms on every push
```

**There is only one copy of the library source.** `PlatformIO/platformio.ini` points at
`../Arduino/libraries` with `lib_extra_dirs`, so both toolchains compile the exact same
files and the two trees can never drift apart.

---

## Quick start

### PlatformIO (recommended)

```bash
cd PlatformIO
pio run -e rawdump -t upload -t monitor    # run this first with a new controller
pio run -e readall -t upload -t monitor    # main firmware
pio test -e native                         # unit tests, no board needed
```

### Arduino IDE

1. Copy `Arduino/libraries/MomoJoy` into `Documents/Arduino/libraries/`
2. Library Manager → install **NimBLE-Arduino 1.4.x**
3. Board: **ESP32S3 Dev Module**, and set **USB CDC On Boot → Disabled**
4. File → Examples → MomoJoy → `02_ReadAllSerial`

Full instructions: [docs/en/INSTALL.md](docs/en/INSTALL.md)

---

## Minimal sketch

```cpp
#include <MomoJoy.h>
using namespace momojoy;

void setup() {
  Serial.begin(115200);
  MomoJoy.begin();
}

void loop() {
  MomoJoy.update();                    // call every iteration; never blocks
  if (!MomoJoy.isConnected()) return;

  if (MomoJoy.justPressed(MOMO_BTN_A)) Serial.println("A!");
  int speed = -MomoJoy.ly();           // -512 .. 511
  int turbo = MomoJoy.r2();            //    0 .. 1023
}
```

## Value ranges

Deliberately identical to Bluepad32, so an existing sketch only needs its accessors renamed
(see [docs/en/MIGRATION_FROM_BLUEPAD32.md](docs/en/MIGRATION_FROM_BLUEPAD32.md)).

| Accessor | Range |
|---|---|
| `lx() ly() rx() ry()` | −512 … 511, 0 = centre, radial dead zone applied |
| `l2() r2()` | 0 … 1023 (analog triggers) |
| `dpad()` | bit mask: `MOMO_DPAD_UP / DOWN / LEFT / RIGHT` |
| `battery()` | 0 … 100 %, or `0xFF` when the pad does not report it |

Buttons: `MOMO_BTN_` `A B X Y L1 R1 L2 R2 SELECT START HOME L3 R3 M1 M2 C Z CAPTURE`

Full API: [docs/en/API.md](docs/en/API.md)

---

## How it works

```
scan  (filter: HID service 0x1812, or appearance = gamepad)
  └─ connect + bond (Just Works, LE Secure Connections, stored in NVS)
       ├─ write Protocol Mode = Report        (0x2A4E <- 1)
       ├─ read  Report Map                    (0x2A4B) -> MomoHidParser -> field table
       ├─ subscribe to every input report     (0x2A4D, type from descriptor 0x2908)
       └─ notify -> FreeRTOS queue -> MomoMapper::decode() -> MomoGamepadState
```

Nothing assumes "byte 3 is the Y axis". Every offset comes from the controller's own report
descriptor, so axes, triggers and the D-pad are correct on any BLE HID gamepad. Only the
*button numbering* is vendor-specific, and that lives in one table you can edit
([docs/en/CALIBRATION.md](docs/en/CALIBRATION.md)).

---

## Limitations

- One controller at a time (by design — keeps RAM use small)
- BLE only. The ESP32-S3 has no Bluetooth Classic, so PS3/PS4/Xbox BR/EDR pads will not work
- No rumble or LED control yet (HID output reports are not implemented)
- Pinned to `arduino-esp32` 2.0.x + `NimBLE-Arduino` 1.4.x; 3.x / 2.x have not been tested

---

## Contributing

`.ino` and library sources must stay **ASCII-only** — non-ASCII characters break the Arduino
automatic prototype generator and produce misleading `ambiguating new declaration` errors.
CI enforces this. Write documentation in `docs/`, not in comments.

Before opening a PR:

```bash
./tools/run_native_tests.sh     # unit tests + sanitizers, no PlatformIO needed
./tools/check_esp_layer.sh      # syntax-check the NimBLE layer against stub headers
cd PlatformIO && pio run -e readall && pio test -e native
```

## License

MIT — see [LICENSE](LICENSE).
