# Pairing the ShanWan Q34U with an ESP32-S3

> ภาษาไทย → [../th/PAIRING.md](../th/PAIRING.md)

## The Q34U has four modes, selected by the slide switch

| Switch | Mode | Used with |
|---|---|---|
| `X` | X-Input | PC / 2.4 GHz dongle |
| `S` | Switch | Nintendo Switch |
| `P` | PS4 | PlayStation 4 |
| **`D`** | **HID & APP** | **Android / BLE HID — this is the one MomoJoy uses** |

> MomoJoy only works in position **D**. The other modes use Bluetooth Classic or 2.4 GHz,
> and the ESP32-**S3 has no Bluetooth Classic** — BLE only.

## Steps

1. Slide the switch to **D**
2. Hold **HOME** for about 3 seconds until the LED **blinks quickly** — pairing mode
3. Power up the ESP32-S3 with MomoJoy flashed. It finds the pad within 2–5 seconds
4. Serial shows:

```
[MomoJoy] scanning for a BLE HID gamepad...
[MomoJoy] connecting to ShanWan Q34u (xx:xx:xx:xx:xx:xx)
[MomoJoy] report map: 108 bytes, 22 input fields, 1 report id(s)
[MomoJoy] subscribed to 1 input report(s)
[MomoJoy] ready
```

5. The controller LED goes solid. The bond is stored in the ESP32's NVS, so it reconnects
   automatically on the next power-up.

## When several controllers are nearby

Lock onto one by name or MAC address:

```cpp
MomoJoyOptions opt;
opt.nameFilter = "Q34";                    // substring of the advertised name
// or, more precise:
opt.addressFilter = "AA:BB:CC:DD:EE:FF";   // the MAC printed on the first connection
MomoJoy.begin(opt);
```

## Common problems

| Symptom | Cause / fix |
|---|---|
| Found but never connects, or `pairing failed` | The pad still remembers an old phone. Clear both sides: set `opt.clearBondsOnBoot = true` (or call `MomoJoy.forgetBonds()`), then reset the controller by holding HOME ~8 s |
| `encryption failed` | Same as above — a stale bond |
| Connects but no values arrive | The switch is not on `D`, or the pad went to sleep. Press any button |
| `no HID service (0x1812)` | It connected to some other BLE device. Set `nameFilter` |
| Frequent disconnects | Usually signal, not software. Keep the antenna away from metal, and avoid heavy Wi-Fi traffic at the same time (see [HARDWARE.md](HARDWARE.md)) |
| A phone grabs the pad first | BLE HID connects to one host at a time. Turn off the phone's Bluetooth or "forget" the pad there |

## Factory reset the controller

Hold **HOME for about 8 seconds** until the LED goes out and starts blinking again. Check the
manual that came with your unit — sub-models differ.
