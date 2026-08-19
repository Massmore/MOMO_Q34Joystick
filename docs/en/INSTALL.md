# Installation

> ภาษาไทย → [../th/INSTALL.md](../th/INSTALL.md)

## 1. What you need

| Item | Detail |
|---|---|
| Board | ESP32-**S3** N16R8 (16 MB QIO flash + 8 MB OPI PSRAM) — any BLE-capable ESP32 works |
| Controller | ShanWan **Q34U** (four-position slide switch: X / S / P / **D**) |
| Cable | USB-C into the board's **UART** port (the one wired to the **CH343P** chip) |

> ⚠️ Most ESP32-S3 dev boards have two USB ports: the **UART** port (through CH343P) and the
> **USB-OTG** port (straight into the chip). MomoJoy routes `Serial` to **UART0 → CH343P**,
> so plug the cable into the UART port.

## 2. CH343P USB-serial driver

You have to install this yourself; it does not ship with the OS (except on recent Linux).

| OS | How |
|---|---|
| Windows | Install [CH343SER.EXE from WCH](https://www.wch-ic.com/downloads/CH343SER_EXE.html) |
| macOS | macOS 13+ usually detects it. If not, use [ch34xser_macos](https://github.com/WCHSoftGroup/ch34xser_macos) |
| Linux | Kernel ≥ 5.13 ships the `ch343` driver. Otherwise use [ch343ser_linux](https://github.com/WCHSoftGroup/ch343ser_linux). Also run `sudo usermod -aG dialout $USER` and log out/in |

Check that it enumerated:

- Windows: Device Manager → Ports (COM & LPT) → `USB-Enhanced-SERIAL CH343`
- macOS: `ls /dev/cu.wchusbserial*`
- Linux: `ls /dev/ttyACM*` or `dmesg | grep ch343`

## 3. Option A — PlatformIO (recommended)

1. Install [VS Code](https://code.visualstudio.com/)
2. VS Code → Extensions → search `PlatformIO IDE` → Install
3. Open the **`PlatformIO/`** folder of this repository (not the repository root)
4. The first build downloads `espressif32@6.9.0` and `NimBLE-Arduino 1.4.x` (5–10 minutes)
5. Verify the setup without plugging in a board:

```bash
cd PlatformIO
pio test -e native      # expect 16/16 PASSED
```

6. Flash it:

```bash
pio run -e rawdump -t upload -t monitor
```

## 4. Option B — Arduino IDE

1. Install Arduino IDE 2.x
2. Boards Manager → install **esp32 by Espressif Systems** (tested with 2.0.17)
3. Library Manager → install **NimBLE-Arduino** (tested with 1.4.3)
4. Copy `Arduino/libraries/MomoJoy` into your libraries folder:

   | OS | Path |
   |---|---|
   | Windows | `Documents\Arduino\libraries\MomoJoy` |
   | macOS | `~/Documents/Arduino/libraries/MomoJoy` |
   | Linux | `~/Arduino/libraries/MomoJoy` |

5. Select **ESP32S3 Dev Module** and set:

   | Setting | Value |
   |---|---|
   | **USB CDC On Boot** | **Disabled** ← without this `Serial` does not reach the CH343P |
   | Flash Size | 16MB (128Mb) |
   | PSRAM | OPI PSRAM |
   | Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
   | Upload Speed | 921600 |

6. File → Examples → MomoJoy → `01_RawDump`

## 5. Tested versions

| Component | Version |
|---|---|
| platform espressif32 (PlatformIO) | 6.9.0 |
| arduino-esp32 core | 2.0.17 |
| NimBLE-Arduino | 1.4.3 |
| xtensa-esp32s3-elf-g++ | 8.4.0 (esp-2021r2-patch5) |

Other versions will probably work but are untested — in particular arduino-esp32 3.x and
NimBLE 2.x, where the NimBLE API changed noticeably.

## 6. Verify

```bash
# core unit tests, no board required
cd PlatformIO && pio test -e native

# the same tests without PlatformIO (plain g++ with sanitizers)
./tools/run_native_tests.sh

# syntax-check the NimBLE layer against stub headers
./tools/check_esp_layer.sh
```
