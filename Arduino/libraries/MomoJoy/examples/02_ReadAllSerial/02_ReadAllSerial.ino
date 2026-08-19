/*
 * MomoJoy - 02_ReadAllSerial
 * ---------------------------------------------------------------------------
 * ESP32-S3 N16R8 + ShanWan Q34U with the mode switch on "D" (HID & APP).
 * Reads every value the controller sends and prints it to Serial (UART0 ->
 * CH343P USB-serial chip).
 *
 * Serial Monitor: 115200 8N1
 *
 * Pairing: slide the controller switch to D, hold HOME ~3 s until the LED
 * blinks fast, then power the board. See docs/en/PAIRING.md.
 *
 * NOTE: keep .ino files ASCII-only. Non-ASCII characters break the Arduino
 * automatic prototype generator (ctags) and produce confusing errors such as
 * "ambiguating new declaration of 'void ...'".
 */
#include <Arduino.h>
#include <MomoJoy.h>

using namespace momojoy;

// Forward declarations. The Arduino builder only auto-generates prototypes for
// functions that are not already declared, so declaring them here keeps the
// sketch immune to that step (it is the usual source of "ambiguating new
// declaration" errors on some toolchain versions).
void setup();
void loop();
void onConnected(const char* name, const char* address);
void onDisconnected();
void onButtons(uint32_t pressed, uint32_t released);

void onConnected(const char* name, const char* address) {
  Serial.printf("\n>>> Controller connected: \"%s\" [%s]\n", name, address);
  MomoJoy.dumpDescriptorFields(Serial);
}

void onDisconnected() {
  Serial.println(">>> Controller disconnected, scanning again...");
}

void onButtons(uint32_t pressed, uint32_t released) {
  struct ButtonName {
    uint32_t bit;
    const char* name;
  };
  static const ButtonName kNames[] = {
      {MOMO_BTN_A, "A"},
      {MOMO_BTN_B, "B"},
      {MOMO_BTN_X, "X"},
      {MOMO_BTN_Y, "Y"},
      {MOMO_BTN_L1, "L1"},
      {MOMO_BTN_R1, "R1"},
      {MOMO_BTN_L2, "L2"},
      {MOMO_BTN_R2, "R2"},
      {MOMO_BTN_SELECT, "SELECT/View"},
      {MOMO_BTN_START, "START/Menu"},
      {MOMO_BTN_HOME, "HOME/Power"},
      {MOMO_BTN_L3, "L3"},
      {MOMO_BTN_R3, "R3"},
      {MOMO_BTN_M1, "M1"},
      {MOMO_BTN_M2, "M2"},
      {MOMO_BTN_C, "C"},
      {MOMO_BTN_Z, "Z"},
      {MOMO_BTN_CAPTURE, "CAPTURE"},
  };
  for (const ButtonName& n : kNames) {
    if (pressed & n.bit) Serial.printf("pressed : %s\n", n.name);
    if (released & n.bit) Serial.printf("released: %s\n", n.name);
  }
}

void setup() {
  // Serial goes to UART0 (GPIO43 TX / GPIO44 RX) -> CH343P.
  // PlatformIO sets -DARDUINO_USB_CDC_ON_BOOT=0 for this.
  // Arduino IDE: Tools > USB CDC On Boot > Disabled.
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=============================================");
  Serial.println(" MomoJoy - ShanWan Q34U (Mode D) BLE HID host");
  Serial.println(" ESP32-S3 N16R8 / Arduino core");
  Serial.println("=============================================");

  MomoJoyOptions opt;
  opt.localName = "MOMO";
  opt.nameFilter = nullptr;   // set to "Q34" if several pads are nearby
  opt.deadzone = 24;
  opt.autoReconnect = true;
  opt.clearBondsOnBoot = false;   // true = forget pairings on every boot
  opt.verbose = true;

  MomoJoy.onConnect(onConnected);
  MomoJoy.onDisconnect(onDisconnected);
  MomoJoy.onButton(onButtons);

  if (!MomoJoy.begin(opt)) {
    Serial.println("[ERROR] BLE init failed");
  }
}

void loop() {
  MomoJoy.update();

  if (MomoJoy.isConnected()) {
    // Print only when something changes, so the console stays readable.
    MomoJoy.printStateChanges(Serial);
  }

  delay(10);
}
