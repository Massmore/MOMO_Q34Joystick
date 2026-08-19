# Calibration — ปรับ mapping ให้ตรงกับจอยจริง

> English → [../en/CALIBRATION.md](../en/CALIBRATION.md)

แกนอนาล็อก / ไกอนาล็อก / D-Pad **ไม่ต้อง calibrate** เพราะ MomoJoy อ่านตำแหน่งบิตจาก
HID Report Descriptor ของจอยเองตอน runtime

สิ่งที่อาจต้องปรับคือ **หมายเลขปุ่ม** เพราะแต่ละยี่ห้อจัดลำดับ Button 1..15 ไม่เหมือนกัน

## 1. ดูว่าจอยส่งอะไรมาจริง

```bash
pio run -e rawdump -t upload -t monitor
```

จะได้ประมาณนี้:

```
---- HID Report Map (108 bytes) ----
0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x85, 0x01, 0x09, 0x30, 0x09, 0x31,
...
---- Parsed input fields ----
  rid  page  usage  bit-off  bits   logical range   name
    1  0x01  0x030        0     8       0..255      X  (Left stick X)
    1  0x01  0x031        8     8       0..255      Y  (Left stick Y)
    1  0x01  0x032       16     8       0..255      Z  (Right stick X)
    1  0x01  0x035       24     8       0..255      Rz (Right stick Y)
    1  0x02  0x0C5       32     8       0..255      Brake (L2)
    1  0x02  0x0C4       40     8       0..255      Accelerator (R2)
    1  0x01  0x039       48     4       0..7        Hat switch (D-Pad)
    1  0x09  0x001       56     1       0..1        Button 1
    ...
  report id 1 -> 9 byte payload
```

จากนั้นกด **ทีละปุ่ม** แล้วดูบรรทัดแบบนี้ (ไบต์ที่เปลี่ยนจะอยู่ในวงเล็บเหลี่ยม):

```
rid=1 len= 9 |  80   80   80   80   00   00   0F  [01]  00   | ...
```

`0x01` ที่ไบต์ 7 = bit 56 = **Button 1**

## 2. จดตารางว่าปุ่มจริงตัวไหน = Button เบอร์อะไร

| ปุ่มบนจอย | ไบต์ที่เปลี่ยน | บิต | = Button # |
|---|---|---|---|
| A | 7 | 0 | 1 |
| B | 7 | 1 | 2 |
| … | | | |

สูตร: `Button # = (byteIndex - firstButtonByte) * 8 + bitIndex + 1`
(ค่า `bit-off` ของ Button 1 อยู่ในตาราง "Parsed input fields" อยู่แล้ว)

## 3. แก้ตาราง mapping

เปิด `Arduino/libraries/MomoJoy/src/core/MomoMapper.cpp` แล้วแก้แค่ตารางนี้:

```cpp
static const MomoButtonMap kAndroidMap[] = {
    {1,  MOMO_BTN_A},        // <- เลข 1 คือ HID Button #
    {2,  MOMO_BTN_B},
    {4,  MOMO_BTN_X},
    {5,  MOMO_BTN_Y},
    {7,  MOMO_BTN_L1},
    {8,  MOMO_BTN_R1},
    {11, MOMO_BTN_SELECT},
    {12, MOMO_BTN_START},
    {13, MOMO_BTN_HOME},
    {14, MOMO_BTN_L3},
    {15, MOMO_BTN_R3},
    // ...
};
```

หรือทำ profile ของตัวเองโดยไม่แตะไลบรารี:

```cpp
static const MomoButtonMap myMap[] = {
    {1, MOMO_BTN_B}, {2, MOMO_BTN_A}, /* ... */
};
static const MomoProfile myProfile = {"MyPad", myMap, 2, false, false};

MomoJoyOptions opt;
opt.profile = &myProfile;
MomoJoy.begin(opt);
```

## 4. ถ้าแกน Y กลับด้าน

```cpp
static const MomoProfile myProfile = {"MyPad", kMyMap, N,
                                      /*invertLeftY=*/true,
                                      /*invertRightY=*/true};
```

## 5. ถ้าก้านอนาล็อกไหลเองตอนปล่อย

เพิ่ม deadzone (หน่วยเดียวกับค่าที่อ่านได้ 0..512):

```cpp
MomoJoy.setDeadzone(48);     // ค่าเริ่มต้น 24
```

## 6. ยืนยันด้วย unit test

ถ้าแก้ตรรกะใน `core/` ให้รัน:

```bash
pio test -e native          # หรือ ./tools/run_native_tests.sh
```
