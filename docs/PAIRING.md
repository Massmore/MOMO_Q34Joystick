# การจับคู่ (Pairing) จอย ShanWan Q34U กับ ESP32-S3

## Q34U มี 4 โหมด เลือกด้วยสวิตช์เลื่อนบนตัวจอย

| ตำแหน่งสวิตช์ | โหมด | ใช้กับ |
| --- | --- | --- |
| `X` | X-Input | PC / dongle 2.4 GHz |
| `S` | Switch | Nintendo Switch |
| `P` | PS4 | PlayStation 4 |
| **`D`** | **HID & APP** | **Android / BLE HID ← ตัวที่ MomoJoy ใช้** |

> [!IMPORTANT]
> MomoJoy ใช้ได้เฉพาะตำแหน่ง **D** เท่านั้น โหมดอื่นเป็น Bluetooth Classic หรือ 2.4 GHz
> ซึ่ง ESP32-**S3 ไม่มี Bluetooth Classic** (มีแต่ BLE)

## ขั้นตอนจับคู่

1. เลื่อนสวิตช์ไปที่ **D**
2. กดปุ่ม **HOME** ค้างประมาณ 3 วินาที จนไฟ LED **กระพริบเร็ว** = เข้าโหมดรอจับคู่
3. เปิดบอร์ด ESP32-S3 ที่ flash MomoJoy ไว้แล้ว บอร์ดจะ scan เจอเองภายใน 2–5 วินาที
4. บน Serial จะขึ้นข้อความนี้:

   ```text
   [MomoJoy] scanning for a BLE HID gamepad...
   [MomoJoy] connecting to ShanWan Q34u (xx:xx:xx:xx:xx:xx)
   [MomoJoy] report map: 108 bytes, 22 input fields, 1 report id(s)
   [MomoJoy] subscribed to 1 input report(s)
   [MomoJoy] ready
   ```

5. ไฟบนจอยจะค้างนิ่ง = จับคู่สำเร็จ bond ถูกเก็บใน NVS ของ ESP32
   ครั้งต่อไปเปิดมาจะ reconnect ให้อัตโนมัติ

## ถ้ามีจอยหลายตัวอยู่รอบ ๆ

ล็อกจอยที่ต้องการด้วยชื่อหรือ MAC address:

```cpp
MomoJoyOptions opt;
opt.nameFilter = "Q34";                    // substring ของชื่อที่จอย advertise
// หรือแม่นยำกว่า:
opt.addressFilter = "AA:BB:CC:DD:EE:FF";   // ดู MAC ได้จาก log ตอน connect ครั้งแรก
MomoJoy.begin(opt);
```

## ปัญหาที่เจอบ่อย

| อาการ | สาเหตุ / วิธีแก้ |
| --- | --- |
| scan เจอแต่ connect ไม่ติด หรือขึ้น `pairing failed` | จอยยังจำ bond กับมือถือเครื่องเก่าอยู่ ต้องล้างทั้งสองฝั่ง: ฝั่ง ESP32 ตั้ง `opt.clearBondsOnBoot = true` (หรือเรียก `MomoJoy.forgetBonds()`) แล้ว reset จอยด้วยการกด HOME ค้าง ~8 วินาที |
| `encryption failed` | สาเหตุเดียวกับข้างบน — bond ค้าง ให้ล้างแล้วจับคู่ใหม่ |
| connect ติดแต่ไม่มีค่าออกมาเลย | สวิตช์ไม่ได้อยู่ตำแหน่ง `D` หรือจอยเข้าโหมดประหยัดไฟ ลองกดปุ่มสักครั้ง |
| `no HID service (0x1812)` | ไป connect กับอุปกรณ์ BLE ตัวอื่นที่ไม่ใช่จอย ให้ตั้ง `nameFilter` |
| หลุดบ่อย | มักเป็นเรื่องสัญญาณ ไม่ใช่ software ให้ย้าย antenna ให้พ้นโลหะ และเลี่ยงการใช้ Wi-Fi หนัก ๆ พร้อมกัน (ดู [HARDWARE.md](HARDWARE.md)) |
| มือถือแย่งจอยไปก่อน | BLE HID ต่อได้ทีละ host เดียว ให้ปิด Bluetooth ของมือถือ หรือสั่ง "forget" จอยในมือถือก่อน |

## รีเซ็ตจอยกลับค่าโรงงาน

กด **HOME ค้างประมาณ 8 วินาที** จนไฟดับแล้วกระพริบใหม่
(ควรเช็คคู่มือที่มากับกล่องอีกที เพราะรุ่นย่อยอาจต่างกัน)

## อ่านต่อ

[CALIBRATION.md](CALIBRATION.md) · [HARDWARE.md](HARDWARE.md) · [API.md](API.md)
