# Migrating from Bluepad32

> ภาษาไทย → [../th/MIGRATION_FROM_BLUEPAD32.md](../th/MIGRATION_FROM_BLUEPAD32.md)

Value ranges are identical (sticks −512..511, triggers 0..1023), so only the accessor names
change.

## Skeleton

```diff
-#include <Bluepad32.h>
-ControllerPtr myControllers[BP32_MAX_GAMEPADS];
+#include <MomoJoy.h>
+using namespace momojoy;

 void setup() {
   Serial.begin(115200);
-  BP32.setup(&onConnectedController, &onDisconnectedController);
-  BP32.forgetBluetoothKeys();
+  MomoJoyOptions opt;
+  opt.clearBondsOnBoot = true;      // equivalent to forgetBluetoothKeys()
+  MomoJoy.begin(opt);
 }

 void loop() {
-  BP32.update();
-  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
-    if (myControllers[i] && myControllers[i]->isConnected()) { ... }
-  }
+  MomoJoy.update();
+  if (MomoJoy.isConnected()) { ... }
 }
```

## Accessor table

| Bluepad32 | MomoJoy |
|---|---|
| `ctl->a()` | `MomoJoy.pressed(MOMO_BTN_A)` |
| `ctl->b()` | `MomoJoy.pressed(MOMO_BTN_B)` |
| `ctl->x()` | `MomoJoy.pressed(MOMO_BTN_X)` |
| `ctl->y()` | `MomoJoy.pressed(MOMO_BTN_Y)` |
| `ctl->l1()` / `r1()` | `MomoJoy.pressed(MOMO_BTN_L1 / MOMO_BTN_R1)` |
| `ctl->thumbL()` / `thumbR()` | `MomoJoy.pressed(MOMO_BTN_L3 / MOMO_BTN_R3)` |
| `ctl->brake()` / `throttle()` | `MomoJoy.l2()` / `MomoJoy.r2()` |
| `ctl->axisX()` `axisY()` | `MomoJoy.lx()` `MomoJoy.ly()` |
| `ctl->axisRX()` `axisRY()` | `MomoJoy.rx()` `MomoJoy.ry()` |
| `ctl->dpad() == 0x01` (up) | `MomoJoy.dpadUp()` |
| `ctl->dpad() == 0x02` (down) | `MomoJoy.dpadDown()` |
| `ctl->dpad() == 0x04` (right) | `MomoJoy.dpadRight()` |
| `ctl->dpad() == 0x08` (left) | `MomoJoy.dpadLeft()` |
| `ctl->miscButtons() & 0x01` | `MomoJoy.pressed(MOMO_BTN_HOME)` |
| `ctl->miscButtons() & 0x02` | `MomoJoy.pressed(MOMO_BTN_SELECT)` |
| `ctl->miscButtons() & 0x04` | `MomoJoy.pressed(MOMO_BTN_START)` |
| `ctl->miscButtons() & 0x08` | `MomoJoy.pressed(MOMO_BTN_CAPTURE)` |
| `BP32.forgetBluetoothKeys()` | `MomoJoy.forgetBonds()` or `opt.clearBondsOnBoot = true` |
| `ctl->battery()` (0..255) | `MomoJoy.battery()` (0..100 %) |

## What you gain

- **Built-in edge detection** — no more tracking `lastState` yourself:
  `MomoJoy.justPressed(MOMO_BTN_A)` instead of `if (a() && !lastA)`
- **No special board platform** — Bluepad32 needs its own patched `platform = ...bluepad32...`
  pinned to a particular Arduino core. MomoJoy uses the stock `espressif32` platform
- **You can fix it yourself** — the button mapping is one table in `MomoMapper.cpp`, and the
  descriptor parser has unit tests you can extend

## What you lose

| | Bluepad32 | MomoJoy |
|---|---|---|
| Simultaneous controllers | up to 4 | 1 |
| Bluetooth Classic (PS3/PS4/Xbox BR/EDR) | yes | ❌ BLE only (the ESP32-S3 has no BT Classic) |
| Rumble / controller LEDs | some models | not implemented (no HID output reports yet) |
| Generic BLE HID gamepads | yes | yes (descriptor parsed at runtime) |
