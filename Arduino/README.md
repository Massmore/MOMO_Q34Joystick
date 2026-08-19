# MomoJoy — Arduino IDE

> ภาษาไทย → [README.th.md](README.th.md)

## 1. Install

### a. The library

Copy the whole folder

```
Arduino/libraries/MomoJoy
```

into your Arduino libraries folder:

| OS | Path |
|---|---|
| Windows | `Documents\Arduino\libraries\MomoJoy` |
| macOS | `~/Documents/Arduino/libraries/MomoJoy` |
| Linux | `~/Arduino/libraries/MomoJoy` |

Then restart the Arduino IDE. `MomoJoy` appears under *File → Examples*.

### b. Dependencies

| What | How |
|---|---|
| **ESP32 board package** | Boards Manager → `esp32 by Espressif Systems` (tested with **2.0.17**) |
| **NimBLE-Arduino** | Library Manager → `NimBLE-Arduino` (tested with **1.4.3**) |
| **CH343P USB driver** | See [docs/en/INSTALL.md](../docs/en/INSTALL.md#ch343p-usb-serial-driver) |

## 2. Board settings — these matter

Select **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module**, then:

| Setting | Value | Why |
|---|---|---|
| **USB CDC On Boot** | **Disabled** | Otherwise `Serial` goes to the native USB-OTG port and you see nothing on the CH343P port |
| Flash Size | 16MB (128Mb) | N16R8 |
| PSRAM | OPI PSRAM | N16R8 — wrong value here makes the board boot-loop |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) | |
| Upload Speed | 921600 | |
| Core Debug Level | None or Error | |

Plug the USB cable into the **UART** port of the board (the one wired to the CH343P chip).

## 3. Run

1. *File → Examples → MomoJoy → **01_RawDump*** — upload it, open Serial Monitor at
   **115200**, slide the Q34U switch to **D**, hold **HOME** until its LED blinks fast.
   You should see the HID report map and the raw bytes of every button press.
2. *File → Examples → MomoJoy → **02_ReadAllSerial*** — the normal firmware: prints every
   button, axis, trigger and the battery level.
3. *03_MomoRobot* — a two-motor robot example.
4. *04_Minimal* — the smallest skeleton to copy into your own sketch.

## 4. Expected first output

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

## 5. Troubleshooting

| Symptom | Fix |
|---|---|
| Serial Monitor stays empty | **USB CDC On Boot** is not *Disabled*, or the cable is in the USB-OTG port |
| Board reboots in a loop before `setup()` | PSRAM setting is wrong — it must be **OPI PSRAM** on N16R8 |
| `ambiguating new declaration of 'void ...'` | You added non-ASCII characters to the `.ino`. Keep sketches ASCII-only, or declare the function prototype yourself |
| `NimBLEDevice.h: No such file` | NimBLE-Arduino is not installed |
| Scans but never connects | Stale pairing — set `opt.clearBondsOnBoot = true` once, and reset the controller |

More: [docs/en/PAIRING.md](../docs/en/PAIRING.md) · [docs/en/CALIBRATION.md](../docs/en/CALIBRATION.md) · [docs/en/API.md](../docs/en/API.md)
