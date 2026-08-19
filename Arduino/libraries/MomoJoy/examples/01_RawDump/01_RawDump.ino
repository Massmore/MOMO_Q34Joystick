/*
 * MomoJoy - 01_RawDump  (calibration tool)
 * ---------------------------------------------------------------------------
 * Run this FIRST with a new controller. It prints:
 *   - the raw HID Report Map
 *   - every input field the descriptor parser extracted
 *   - the raw bytes of every input report, with changed bytes in [brackets]
 *
 * Press one button at a time and watch which byte/bit changes, then adjust
 * kAndroidMap in MomoMapper.cpp. See docs/en/CALIBRATION.md.
 *
 * NOTE: keep .ino files ASCII-only. Non-ASCII characters can break the Arduino
 * automatic prototype generator.
 */
#include <Arduino.h>
#include <MomoJoy.h>

using namespace momojoy;

void setup();
void loop();
void onConnected(const char* name, const char* address);
void onRaw(uint8_t reportId, const uint8_t* data, size_t len);

void onConnected(const char* name, const char* address) {
  Serial.printf("\n>>> connected: \"%s\" [%s]\n", name, address);
  MomoJoy.dumpReportMap(Serial);
  MomoJoy.dumpDescriptorFields(Serial);
  Serial.println("Press buttons / move sticks and watch the bytes change:");
}

void onRaw(uint8_t reportId, const uint8_t* data, size_t len) {
  static uint8_t last[24] = {0};
  static size_t lastLen = 0;

  bool changed = (len != lastLen);
  for (size_t i = 0; i < len && !changed; ++i) changed = (data[i] != last[i]);
  if (!changed) return;

  Serial.printf("rid=%u len=%2u |", reportId, static_cast<unsigned>(len));
  for (size_t i = 0; i < len; ++i) {
    const bool diff = (i >= lastLen) || (data[i] != last[i]);
    Serial.printf(diff ? " [%02X]" : "  %02X ", data[i]);
  }
  Serial.print("  | ");
  for (size_t i = 0; i < len; ++i) {
    for (int b = 7; b >= 0; --b) Serial.print((data[i] >> b) & 1);
    Serial.print(' ');
  }
  Serial.println();

  memcpy(last, data, len > sizeof(last) ? sizeof(last) : len);
  lastLen = len;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n== MomoJoy RAW DUMP ==");

  MomoJoyOptions opt;
  opt.verbose = true;
  opt.clearBondsOnBoot = true;   // start from a clean pairing while calibrating

  MomoJoy.onConnect(onConnected);
  MomoJoy.onRawReport(onRaw);
  MomoJoy.begin(opt);
}

void loop() {
  MomoJoy.update();
  delay(5);
}
