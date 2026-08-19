/*
 * MomoJoy - 03_MomoRobot
 * ---------------------------------------------------------------------------
 * Drive a two-motor robot / RC car with the Q34U. Works with any PWM + DIR
 * motor driver (TB6612FNG, BTS7960, L298N, ...).
 *
 *   Left stick Y : forward / reverse
 *   Left stick X : steering
 *   R2           : turbo (analog)
 *   L1           : precision (slow) mode
 *   B            : emergency stop (toggle)
 *
 * Change the pin numbers below to match your board.
 *
 * NOTE: keep .ino files ASCII-only.
 */
#include <Arduino.h>
#include <MomoJoy.h>

using namespace momojoy;

// ---- Motor pins (edit to match your wiring) -------------------------------
static const int PIN_L_PWM = 5;
static const int PIN_L_DIR = 4;
static const int PIN_R_PWM = 6;
static const int PIN_R_DIR = 7;

static const int CH_L = 0;
static const int CH_R = 1;
static const int PWM_FREQ = 20000;   // 20 kHz - above hearing range
static const int PWM_BITS = 10;      // 0..1023, same scale as MomoJoy.r2()

static bool emergencyStop = false;

void setup();
void loop();
void pwmSetup(int pin, int ch);
void pwmWrite(int pin, int ch, uint32_t duty);
void driveMotor(int pwmPin, int ch, int dirPin, int speed);
void stopAll();
void onButtons(uint32_t pressed, uint32_t released);
void onDisconnected();

// ---- Arduino-ESP32 2.x / 3.x compatibility --------------------------------
void pwmSetup(int pin, int ch) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  (void)ch;
  ledcAttach(pin, PWM_FREQ, PWM_BITS);
#else
  ledcSetup(ch, PWM_FREQ, PWM_BITS);
  ledcAttachPin(pin, ch);
#endif
}

void pwmWrite(int pin, int ch, uint32_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  (void)ch;
  ledcWrite(pin, duty);
#else
  (void)pin;
  ledcWrite(ch, duty);
#endif
}

void driveMotor(int pwmPin, int ch, int dirPin, int speed) {
  if (speed < -1023) speed = -1023;
  if (speed > 1023) speed = 1023;
  digitalWrite(dirPin, speed >= 0 ? HIGH : LOW);
  pwmWrite(pwmPin, ch, static_cast<uint32_t>(speed >= 0 ? speed : -speed));
}

void stopAll() {
  driveMotor(PIN_L_PWM, CH_L, PIN_L_DIR, 0);
  driveMotor(PIN_R_PWM, CH_R, PIN_R_DIR, 0);
}

void onButtons(uint32_t pressed, uint32_t released) {
  (void)released;
  if (pressed & MOMO_BTN_B) {
    emergencyStop = !emergencyStop;
    Serial.printf("[MOMO] emergency stop = %s\n", emergencyStop ? "ON" : "OFF");
    if (emergencyStop) stopAll();
  }
}

void onDisconnected() {
  stopAll();
  Serial.println("[MOMO] controller lost -> motors stopped");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_L_DIR, OUTPUT);
  pinMode(PIN_R_DIR, OUTPUT);
  pwmSetup(PIN_L_PWM, CH_L);
  pwmSetup(PIN_R_PWM, CH_R);
  stopAll();

  MomoJoyOptions opt;
  opt.deadzone = 40;         // a robot wants a wider dead zone than a game
  opt.autoReconnect = true;
  MomoJoy.onButton(onButtons);
  MomoJoy.onDisconnect(onDisconnected);
  MomoJoy.begin(opt);

  Serial.println("[MOMO] ready, waiting for the controller...");
}

void loop() {
  MomoJoy.update();

  if (!MomoJoy.isConnected() || emergencyStop) {
    stopAll();
    delay(20);
    return;
  }

  // -512..511 -> -1023..1023
  int throttle = -static_cast<int>(MomoJoy.ly()) * 2;   // push up = forward
  int steer = static_cast<int>(MomoJoy.lx()) * 2;

  float gain = 0.55f;                                   // normal 55 %
  if (MomoJoy.pressed(MOMO_BTN_L1)) gain = 0.25f;       // precision mode
  gain += (MomoJoy.r2() / 1023.0f) * 0.45f;             // R2 = turbo up to 100 %

  int left = static_cast<int>((throttle + steer) * gain);
  int right = static_cast<int>((throttle - steer) * gain);

  driveMotor(PIN_L_PWM, CH_L, PIN_L_DIR, left);
  driveMotor(PIN_R_PWM, CH_R, PIN_R_DIR, right);

  static uint32_t last = 0;
  if (millis() - last > 200) {
    last = millis();
    Serial.printf("L=%5d R=%5d  (ly=%d lx=%d r2=%u bat=%u%%)\n", left, right,
                  MomoJoy.ly(), MomoJoy.lx(), MomoJoy.r2(), MomoJoy.battery());
  }

  delay(10);
}
