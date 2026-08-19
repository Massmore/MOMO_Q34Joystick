/*
 * MomoJoy - 04_Minimal
 * The shortest useful sketch. Start here when building your own project.
 */
#include <Arduino.h>
#include <MomoJoy.h>

using namespace momojoy;

void setup();
void loop();

void setup() {
  Serial.begin(115200);
  MomoJoy.begin();   // defaults: connect to the first BLE HID gamepad found
}

void loop() {
  MomoJoy.update();
  if (!MomoJoy.isConnected()) return;

  if (MomoJoy.justPressed(MOMO_BTN_A)) Serial.println("A pressed");
  if (MomoJoy.justReleased(MOMO_BTN_A)) Serial.println("A released");
  if (MomoJoy.dpadUp()) Serial.println("D-Pad UP");

  Serial.printf("LX=%d LY=%d R2=%u\n", MomoJoy.lx(), MomoJoy.ly(), MomoJoy.r2());
  delay(100);
}
