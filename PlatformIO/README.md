# MomoJoy — PlatformIO (VS Code)

> ภาษาไทย → [README.th.md](README.th.md)

## 1. Install

1. [VS Code](https://code.visualstudio.com/)
2. VS Code → Extensions → **PlatformIO IDE** → Install
3. **CH343P USB driver** — see [docs/en/INSTALL.md](../docs/en/INSTALL.md#ch343p-usb-serial-driver)
4. Open **this `PlatformIO/` folder** in VS Code (not the repository root).
   PlatformIO downloads `espressif32` and `NimBLE-Arduino` on the first build (5–10 min).

## 2. Environments

| Command | What it does |
|---|---|
| `pio run -e rawdump -t upload -t monitor` | Calibration: prints the HID report map and the raw report bytes |
| `pio run -e readall -t upload -t monitor` | Main firmware: every button, axis, trigger, battery |
| `pio run -e robot -t upload` | Two-motor robot example |
| `pio run -e minimal -t upload` | Smallest sketch |
| `pio test -e native` | 16 unit tests on your PC, no board required |

`readall` is the default environment, so a bare `pio run` builds that one.

## 3. Where the code lives

```
PlatformIO/
├── platformio.ini
├── src/app_*.cpp        thin entry points, one per environment
└── test/test_core/      unit tests for the pure-C++ core
```

The library itself is **not** duplicated here. `platformio.ini` contains

```ini
lib_extra_dirs = ${PROJECT_DIR}/../Arduino/libraries
```

so PlatformIO compiles `../Arduino/libraries/MomoJoy` directly — the same files the Arduino
IDE uses. Each `src/app_*.cpp` is a two-line file that includes the matching example:

```cpp
#include "../../Arduino/libraries/MomoJoy/examples/02_ReadAllSerial/02_ReadAllSerial.ino"
```

Edit the library or the examples in `../Arduino/libraries/MomoJoy/`; both toolchains pick the
change up immediately.

## 4. Board configuration (already set in platformio.ini)

```ini
board = esp32-s3-devkitc-1
board_build.arduino.memory_type = qio_opi   ; N16R8: QIO flash + OPI PSRAM
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
build_flags = -DARDUINO_USB_CDC_ON_BOOT=0   ; Serial -> UART0 -> CH343P
```

`ARDUINO_USB_CDC_ON_BOOT=0` is the important one: without it `Serial` goes to the ESP32-S3's
native USB port and nothing appears on the CH343P port. Plug the cable into the **UART** port.

`monitor_rts = 0` / `monitor_dtr = 0` stop the board resetting when you open the monitor.
Delete those two lines if you want an auto-reset instead.

## 5. Verified build output

```
RAM:   [=         ]   9.9% (used 32368 bytes from 327680 bytes)
Flash: [=         ]   8.1% (used 529153 bytes from 6553600 bytes)
Building .pio/build/readall/firmware.bin
========================= [SUCCESS] Took 27.53 seconds =========================
```

## 6. Troubleshooting

| Symptom | Fix |
|---|---|
| `HTTPClientError` on first build | No internet, or a proxy is blocking the PlatformIO registry |
| Serial monitor is empty | Cable is in the USB-OTG port instead of the UART port |
| Board boot-loops | `board_build.arduino.memory_type` must be `qio_opi` for N16R8 |
| `pio test -e native` cannot find headers | Run it from inside `PlatformIO/`, not from the repository root |
| Upload fails at 921600 baud | Lower `upload_speed` to 460800 |

More: [docs/en/PAIRING.md](../docs/en/PAIRING.md) · [docs/en/CALIBRATION.md](../docs/en/CALIBRATION.md) · [docs/en/API.md](../docs/en/API.md)
