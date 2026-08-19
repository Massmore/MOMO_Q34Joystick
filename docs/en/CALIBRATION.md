# Calibration — matching the button map to your controller

> ภาษาไทย → [../th/CALIBRATION.md](../th/CALIBRATION.md)

Analog sticks, analog triggers and the D-pad need **no calibration**: MomoJoy reads their bit
positions straight from the controller's HID report descriptor at runtime.

What may need adjusting is the **button numbering**, because vendors order Button 1..15
differently.

## 1. See what the controller actually sends

```bash
cd PlatformIO
pio run -e rawdump -t upload -t monitor
```

Output looks like this:

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

Then press **one button at a time** and watch the raw line — changed bytes are in brackets:

```
rid=1 len= 9 |  80   80   80   80   00   00   0F  [01]  00   | ...
```

`0x01` in byte 7 = bit 56 = **Button 1**.

## 2. Write down which physical button is which HID button

| Button on the pad | Byte that changed | Bit | = Button # |
|---|---|---|---|
| A | 7 | 0 | 1 |
| B | 7 | 1 | 2 |
| … | | | |

Formula: `Button # = (byteIndex - firstButtonByte) * 8 + bitIndex + 1`
(the `bit-off` of Button 1 is already printed in the "Parsed input fields" table).

## 3. Edit the mapping table

Open `Arduino/libraries/MomoJoy/src/core/MomoMapper.cpp` and change only this table:

```cpp
static const MomoButtonMap kAndroidMap[] = {
    {1,  MOMO_BTN_A},        // 1 is the HID button number
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

Or define your own profile without touching the library:

```cpp
static const MomoButtonMap myMap[] = {
    {1, MOMO_BTN_B}, {2, MOMO_BTN_A}, /* ... */
};
static const MomoProfile myProfile = {"MyPad", myMap, 2, false, false};

MomoJoyOptions opt;
opt.profile = &myProfile;
MomoJoy.begin(opt);
```

## 4. If an axis is inverted

```cpp
static const MomoProfile myProfile = {"MyPad", kMyMap, N,
                                      /*invertLeftY=*/true,
                                      /*invertRightY=*/true};
```

## 5. If a stick drifts when released

Widen the dead zone (same units as the output, 0..512):

```cpp
MomoJoy.setDeadzone(48);     // default is 24
```

## 6. Re-run the tests

If you changed anything under `src/core/`:

```bash
cd PlatformIO && pio test -e native      # or ./tools/run_native_tests.sh
```
