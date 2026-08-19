# Hardware — ESP32-S3 N16R8 + CH343P

## ทำไมต้องมี `-DARDUINO_USB_CDC_ON_BOOT=0`

ESP32-S3 มี USB ในตัว บอร์ด DevKit ส่วนใหญ่จึงมี **USB สอง port**:

```text
[USB-C "UART"] --- CH343P --- UART0 (GPIO43 TX / GPIO44 RX) --- ESP32-S3
[USB-C "OTG"]  --------------- native USB peripheral --------- ESP32-S3
```

ค่าเริ่มต้นของ arduino-esp32 core map `Serial` ไปที่ USB CDC (คือ port OTG) ผลคือถ้าเสียบ
port UART จะ **ไม่เห็นอะไรเลย** MomoJoy จึงตั้ง `ARDUINO_USB_CDC_ON_BOOT=0` ไว้ใน
`platformio.ini` เพื่อบังคับให้ `Serial` เป็น `HardwareSerial(0)` ที่ออกทาง CH343P

ค่าเทียบเท่าใน Arduino IDE: **Tools → USB CDC On Boot → Disabled**

ถ้าอยากใช้ port OTG แทน ให้ลบ flag นี้ออก (แล้ว `Serial` จะกลับไปเป็น USB CDC)

## แยก log ออก UART อีกเส้น

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

## Memory และ partition

N16R8 = flash 16 MB (QIO) + PSRAM 8 MB (OPI)

```ini
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
```

> [!CAUTION]
> ถ้าตั้ง `memory_type` ผิด (เช่นใส่ `qio_qspi` กับบอร์ดที่เป็น OPI PSRAM) บอร์ดจะ boot-loop
> อาการคือขึ้น `Guru Meditation Error` หรือ reset ซ้ำ ๆ ตั้งแต่ยังไม่ถึง `setup()`

ตัว MomoJoy เองใช้ RAM น้อยมาก (buffer สำหรับ report map 512 byte + queue ขนาด 16 × 26 byte)
จึงไม่จำเป็นต้องมี PSRAM แต่เปิดไว้ก็ไม่เสียหาย

ค่าที่วัดได้จาก firmware `readall`: **flash 529 KB, RAM 32 KB**

## BLE กับ Wi-Fi ใช้ antenna ร่วมกัน

ถ้าเปิด Wi-Fi ไปด้วยแล้วจอยเริ่มกระตุก ให้ลอง:

- ลด traffic ของ Wi-Fi ระหว่างที่กำลังขับหุ่น
- หรือเรียก `esp_wifi_set_ps(WIFI_PS_MIN_MODEM)`
- หรือแยกงานกัน: ให้ ESP32-S3 ตัวหนึ่งรับค่าจากจอย แล้วส่งต่อออก UART / ESP-NOW ไปอีกบอร์ด

## การต่อมอเตอร์ (ตัวอย่าง `03_MomoRobot`)

| ESP32-S3 | ต่อไปที่ |
| --- | --- |
| GPIO5 | motor driver ซ้าย — PWM |
| GPIO4 | motor driver ซ้าย — DIR |
| GPIO6 | motor driver ขวา — PWM |
| GPIO7 | motor driver ขวา — DIR |
| GND | GND ร่วมกับ driver **เสมอ** |

> [!WARNING]
> อย่าจ่ายไฟมอเตอร์ผ่านขา 5 V ของบอร์ด แรงดันที่ตกจะทำให้ BLE หลุด
> ให้ใช้แหล่งจ่ายแยกและต่อ GND ร่วมกัน

PWM ตั้งไว้ที่ 20 kHz / 10 bit ดังนั้นช่วง duty 0..1023 จึงตรงกับสเกลที่ `r2()` คืนมาพอดี

## อ่านต่อ

[INSTALL.md](INSTALL.md) · [PAIRING.md](PAIRING.md) · [API.md](API.md)
