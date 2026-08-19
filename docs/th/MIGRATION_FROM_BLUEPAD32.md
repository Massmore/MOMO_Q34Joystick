# ย้ายจากโค้ด Bluepad32 เดิมมาที่ MomoJoy

> English → [../en/MIGRATION_FROM_BLUEPAD32.md](../en/MIGRATION_FROM_BLUEPAD32.md)

ช่วงค่าเหมือนกันทุกอย่าง (แกน −512..511, ไก 0..1023) จึงเปลี่ยนแค่ชื่อฟังก์ชัน

## โครงหลัก

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
+  opt.clearBondsOnBoot = true;      // เทียบเท่า forgetBluetoothKeys()
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

## ตารางเทียบ

| Bluepad32 | MomoJoy |
|---|---|
| `ctl->a()` | `MomoJoy.pressed(MOMO_BTN_A)` |
| `ctl->b()` | `MomoJoy.pressed(MOMO_BTN_B)` |
| `ctl->x()` | `MomoJoy.pressed(MOMO_BTN_X)` |
| `ctl->y()` | `MomoJoy.pressed(MOMO_BTN_Y)` |
| `ctl->l1()` / `r1()` | `MomoJoy.pressed(MOMO_BTN_L1 / MOMO_BTN_R1)` |
| `ctl->thumbL()` / `thumbR()` | `MomoJoy.pressed(MOMO_BTN_L3 / MOMO_BTN_R3)` |
| `ctl->brake()` / `throttle()` | `MomoJoy.l2()` / `MomoJoy.r2()` |
| `ctl->axisX() axisY()` | `MomoJoy.lx()` `MomoJoy.ly()` |
| `ctl->axisRX() axisRY()` | `MomoJoy.rx()` `MomoJoy.ry()` |
| `ctl->dpad() == 0x01` (บน) | `MomoJoy.dpadUp()` |
| `ctl->dpad() == 0x02` (ล่าง) | `MomoJoy.dpadDown()` |
| `ctl->dpad() == 0x04` (ขวา) | `MomoJoy.dpadRight()` |
| `ctl->dpad() == 0x08` (ซ้าย) | `MomoJoy.dpadLeft()` |
| `ctl->miscButtons() & 0x01` | `MomoJoy.pressed(MOMO_BTN_HOME)` |
| `ctl->miscButtons() & 0x02` | `MomoJoy.pressed(MOMO_BTN_SELECT)` |
| `ctl->miscButtons() & 0x04` | `MomoJoy.pressed(MOMO_BTN_START)` |
| `ctl->miscButtons() & 0x08` | `MomoJoy.pressed(MOMO_BTN_CAPTURE)` |
| `BP32.forgetBluetoothKeys()` | `MomoJoy.forgetBonds()` หรือ `opt.clearBondsOnBoot = true` |
| `ctl->battery()` (0..255) | `MomoJoy.battery()` (0..100 %) |

## ข้อดีที่ได้เพิ่ม

- **edge detection ในตัว** — ไม่ต้องเก็บ `lastState` เอง:
  `MomoJoy.justPressed(MOMO_BTN_A)` แทน `if (a() && !lastA)`
- **ไม่ต้องใช้ board platform พิเศษ** — Bluepad32 ต้องใช้ `platform = ...bluepad32...`
  ซึ่งผูกกับ Arduino core เวอร์ชันที่เขาแพตช์ไว้ MomoJoy ใช้ Arduino core มาตรฐาน
- **แก้เองได้ทั้งหมด** — mapping ปุ่มอยู่ในตารางเดียวใน `MomoMapper.cpp`

## ข้อจำกัดที่ต้องรู้

| | Bluepad32 | MomoJoy |
|---|---|---|
| จอยพร้อมกัน | สูงสุด 4 | 1 (ตั้งใจให้เบา) |
| Bluetooth Classic (PS3/PS4/Xbox แบบ BR/EDR) | รองรับ | ❌ BLE อย่างเดียว (ESP32-S3 ไม่มี BT Classic) |
| สั่น / ไฟ LED บนจอย | รองรับบางรุ่น | ยังไม่รองรับ (HID Output Report ยังไม่ได้ทำ) |
| จอย BLE HID ทั่วไป | รองรับ | รองรับ (parse descriptor เอง) |
