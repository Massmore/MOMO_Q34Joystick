# การจับคู่จอย ShanWan Q34U กับ ESP32-S3 (Mode D)

> English → [../en/PAIRING.md](../en/PAIRING.md)

## Q34U มี 4 โหมด — เลือกด้วยสวิตช์เลื่อนบนตัวจอย

| ตำแหน่งสวิตช์ | โหมด | ใช้กับ |
|---|---|---|
| `X` | X-Input | PC / ดองเกิล 2.4G |
| `S` | Switch | Nintendo Switch |
| `P` | PS4 | PlayStation 4 |
| **`D`** | **HID & APP** | **Android / BLE HID ← ตัวที่ MomoJoy ใช้** |

> MomoJoy ใช้ได้เฉพาะตำแหน่ง **D** เท่านั้น โหมดอื่นเป็น Bluetooth Classic หรือ 2.4 GHz
> ซึ่ง ESP32-**S3 ไม่มี Bluetooth Classic** (มีแต่ BLE)

## ขั้นตอน

1. เลื่อนสวิตช์ไปที่ **D**
2. กดปุ่ม **HOME** ค้างประมาณ 3 วินาที จนไฟ LED **กระพริบเร็ว** = โหมดรอจับคู่
3. เปิดบอร์ด ESP32-S3 ที่แฟลช MomoJoy ไว้แล้ว → บอร์ดจะสแกนเจอเองภายใน 2–5 วินาที
4. บน Serial จะขึ้น:

```
[MomoJoy] scanning for a BLE HID gamepad...
[MomoJoy] connecting to ShanWan Q34u (xx:xx:xx:xx:xx:xx)
[MomoJoy] report map: 108 bytes, 22 input fields, 1 report id(s)
[MomoJoy] subscribed to 1 input report(s)
[MomoJoy] ready
```

5. ไฟบนจอยจะค้างนิ่ง = จับคู่แล้ว การจับคู่ถูกเก็บใน NVS ของ ESP32 → ครั้งต่อไปเปิดมาต่อเองอัตโนมัติ

## ถ้ามีจอยหลายตัวรอบตัว

ล็อกด้วยชื่อหรือ MAC:

```cpp
MomoJoyOptions opt;
opt.nameFilter = "Q34u";                      // ชื่อที่จอย advertise
// หรือแม่นกว่า:
opt.addressFilter = "AA:BB:CC:DD:EE:FF";      // ดู MAC ได้จาก log ตอนต่อครั้งแรก
MomoJoy.begin(opt);
```

## ปัญหาที่เจอบ่อย

| อาการ | สาเหตุ / วิธีแก้ |
|---|---|
| สแกนเจอแต่ต่อไม่ติด / `pairing failed` | จอยยังจำการจับคู่กับมือถือเครื่องเก่าอยู่ → ลบ bond ทั้งสองฝั่ง: บน ESP32 ตั้ง `opt.clearBondsOnBoot = true` หรือเรียก `MomoJoy.forgetBonds()` แล้วรีเซ็ตจอย (กด HOME ค้าง 8 วินาที) |
| `encryption failed` | เหมือนข้างบน — bond ค้าง ลบแล้วจับคู่ใหม่ |
| ต่อติดแต่ไม่มีค่าออกมาเลย | จอยไม่ได้อยู่ตำแหน่ง `D` / หรืออยู่ในโหมดประหยัดไฟ ลองกดปุ่มสักครั้ง |
| `no HID service (0x1812)` | เจอ BLE ตัวอื่นที่ไม่ใช่จอย → ตั้ง `nameFilter` |
| หลุดบ่อย | ลอง `opt.deadzone` ไม่เกี่ยว — มักเป็นเรื่องระยะ/สัญญาณ ลองย้ายเสาอากาศให้พ้นโลหะ และอย่าใช้ WiFi หนัก ๆ พร้อมกัน |
| ต่อกับมือถือแล้ว ESP32 แย่งไม่ได้ | BLE HID ต่อได้ทีละเครื่อง ให้ปิด Bluetooth มือถือหรือสั่ง "forget" ก่อน |

## รีเซ็ตจอยกลับค่าโรงงาน

กด **HOME ค้าง ~8 วินาที** จนไฟดับแล้วกระพริบใหม่ (ดูคู่มือที่มากับกล่องอีกที รุ่นย่อยอาจต่างกัน)
