// Stub Arduino.h - ONLY used by tools/check_esp_layer.sh to syntax-check the
// ESP32 layer of MomoJoy on a PC. It is never compiled into firmware.
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#define ARDUINO_ARCH_ESP32 1
#define ESP_ARDUINO_VERSION_MAJOR 2

#define F(x) (x)
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0

typedef std::string String;

inline unsigned long millis() { return 0; }
inline void delay(unsigned long) {}
inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline void ledcSetup(int, int, int) {}
inline void ledcAttachPin(int, int) {}
inline void ledcWrite(int, uint32_t) {}

class Print {
 public:
  virtual ~Print() {}
  size_t print(const char*) { return 0; }
  size_t print(char) { return 0; }
  size_t print(int) { return 0; }
  size_t println() { return 0; }
  size_t println(const char*) { return 0; }
  size_t println(int) { return 0; }
  int printf(const char*, ...) { return 0; }
};

class Stream : public Print {
 public:
  void begin(unsigned long) {}
};

extern Stream Serial;
