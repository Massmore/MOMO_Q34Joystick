# ฮาร์ดแวร์ — ESP32-S3 N16R8 + CH343P

> English → [../en/HARDWARE.md](../en/HARDWARE.md)

## ทำไมต้อง `-DARDUINO_USB_CDC_ON_BOOT=0`

ESP32-S3 มี USB ในตัว บอร์ด DevKit ส่วนใหญ่จึงมี **พอร์ต USB สองช่อง**:

```
[USB-C "UART"] --- CH343P --- UART0 (GPIO43 TX / GPIO44 RX) --- ESP32-S3
[USB-C "OTG"]  ------------------- USB ในตัวชิป ---------------- ESP32-S3
```

Arduino-ESP32 ตั้งค่าเริ่มต้นให้ `Serial` = USB CDC (พอร์ต OTG) ผลคือเสียบพอร์ต UART แล้ว
**ไม่เห็นอะไรเลย** ไลบรารีนี้จึงตั้ง `ARDUINO_USB_CDC_ON_BOOT=0` ไว้ใน `platformio.ini`
เพื่อบังคับให้ `Serial` = `HardwareSerial(0)` ออกทาง CH343P

ถ้าอยากใช้พอร์ต OTG แทน: ลบ flag นั้นออก (แล้ว `Serial` จะกลายเป็น USB CDC)

## ถ้าอยากแยก log ออก UART1

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

## หน่วยความจำ / partition

N16R8 = Flash 16 MB (QIO) + PSRAM 8 MB (OPI)

```ini
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
```

> ถ้าตั้ง memory_type ผิด (เช่น `qio_qspi` กับบอร์ด OPI PSRAM) บอร์ดจะบูตวน
> อาการ: `Guru Meditation Error` หรือรีเซ็ตซ้ำ ๆ ตั้งแต่ก่อนถึง `setup()`

MomoJoy เองใช้ RAM น้อยมาก (buffer report map 512 B + คิว 16×26 B) ไม่ต้องใช้ PSRAM
แต่เปิด PSRAM ไว้ก็ไม่เสียหาย

## BLE vs WiFi

ESP32-S3 ใช้เสาอากาศร่วมกันระหว่าง WiFi กับ BLE ถ้าเปิด WiFi ไปด้วยแล้วจอยกระตุก:

- ลด traffic WiFi ระหว่างขับหุ่น
- หรือใช้ `esp_wifi_set_ps(WIFI_PS_MIN_MODEM)`
- หรือแยกงาน: ESP32-S3 ตัวหนึ่งรับจอย แล้วส่งค่าออก UART/ESP-NOW ไปอีกตัว

## การต่อมอเตอร์ (ตัวอย่าง `03_MomoRobot`)

| ESP32-S3 | ไปที่ |
|---|---|
| GPIO5 | ไดรเวอร์มอเตอร์ซ้าย PWM |
| GPIO4 | ไดรเวอร์มอเตอร์ซ้าย DIR |
| GPIO6 | ไดรเวอร์มอเตอร์ขวา PWM |
| GPIO7 | ไดรเวอร์มอเตอร์ขวา DIR |
| GND | GND ร่วมกับไดรเวอร์ **เสมอ** |

⚠️ อย่าจ่ายไฟมอเตอร์ผ่าน 5V ของบอร์ด — แรงดันตกทำให้ BLE หลุด ใช้แหล่งจ่ายแยกและต่อ GND ร่วม

PWM ตั้งไว้ 20 kHz / 10 บิต → ค่า duty 0..1023 ตรงกับสเกลของ `r2()` พอดี
