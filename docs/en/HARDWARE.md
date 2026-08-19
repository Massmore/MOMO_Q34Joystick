# Hardware — ESP32-S3 N16R8 + CH343P

> ภาษาไทย → [../th/HARDWARE.md](../th/HARDWARE.md)

## Why `-DARDUINO_USB_CDC_ON_BOOT=0` matters

The ESP32-S3 has USB built in, so most dev boards expose **two USB ports**:

```
[USB-C "UART"] --- CH343P --- UART0 (GPIO43 TX / GPIO44 RX) --- ESP32-S3
[USB-C "OTG"]  --------------- native USB peripheral --------- ESP32-S3
```

By default the Arduino-ESP32 core maps `Serial` to USB CDC (the OTG port). Plug into the UART
port with that default and **you see nothing**. MomoJoy therefore sets
`ARDUINO_USB_CDC_ON_BOOT=0` in `platformio.ini`, which makes `Serial` a `HardwareSerial(0)`
going out through the CH343P.

Arduino IDE equivalent: **Tools → USB CDC On Boot → Disabled**.

To use the OTG port instead, remove the flag (then `Serial` becomes USB CDC).

## Logging on a separate UART

```cpp
HardwareSerial Log(1);

void setup() {
  Log.begin(115200, SERIAL_8N1, /*RX=*/18, /*TX=*/17);
  MomoJoy.begin();
}

void loop() {
  MomoJoy.update();
  MomoJoy.printStateChanges(Log);
}
```

## Memory and partitions

N16R8 = 16 MB flash (QIO) + 8 MB PSRAM (OPI)

```ini
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
```

> Getting `memory_type` wrong (for example `qio_qspi` on an OPI-PSRAM board) makes the board
> boot-loop: `Guru Meditation Error` or endless resets before `setup()` ever runs.

MomoJoy itself uses very little RAM (a 512-byte report-map buffer plus a 16 × 26 byte queue),
so PSRAM is not required — but leaving it enabled does no harm.

Measured for the `readall` firmware: **529 KB flash, 32 KB RAM**.

## BLE and Wi-Fi share one antenna

If you enable Wi-Fi as well and the controller starts stuttering:

- reduce Wi-Fi traffic while driving
- or call `esp_wifi_set_ps(WIFI_PS_MIN_MODEM)`
- or split the work: one ESP32-S3 handles the controller and forwards values over
  UART / ESP-NOW to a second board

## Motor wiring (the `03_MomoRobot` example)

| ESP32-S3 | Goes to |
|---|---|
| GPIO5 | Left motor driver PWM |
| GPIO4 | Left motor driver DIR |
| GPIO6 | Right motor driver PWM |
| GPIO7 | Right motor driver DIR |
| GND | **Always** common with the driver's GND |

⚠️ Do not power motors from the board's 5 V rail. The voltage dip will drop the BLE link. Use
a separate supply and tie the grounds together.

PWM is configured at 20 kHz / 10 bits, so the duty range 0..1023 lines up exactly with the
scale returned by `r2()`.
