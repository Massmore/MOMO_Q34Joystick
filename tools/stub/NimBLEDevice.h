// Stub of the NimBLE-Arduino **1.4.3** public API surface that MomoJoy uses.
// Signatures were copied from the real headers so `tools/check_esp_layer.sh`
// catches API mistakes. Syntax-check only - never built into firmware.
#pragma once

#include <stdint.h>
#include <time.h>

#include <functional>
#include <string>
#include <vector>

#define ESP_PWR_LVL_P9 7
#define BLE_HS_IO_NO_INPUT_OUTPUT 3

typedef int esp_power_level_t;

struct ble_gap_upd_params {
  uint16_t itvl_min;
  uint16_t itvl_max;
  uint16_t latency;
  uint16_t supervision_timeout;
};

struct ble_gap_sec_state {
  unsigned encrypted : 1;
  unsigned authenticated : 1;
  unsigned bonded : 1;
};

struct ble_gap_conn_desc {
  uint16_t conn_handle;
  ble_gap_sec_state sec_state;
};

class NimBLEUUID {
 public:
  NimBLEUUID() {}
  NimBLEUUID(uint16_t) {}
  NimBLEUUID(uint32_t) {}
  NimBLEUUID(const std::string&) {}
  bool equals(const NimBLEUUID&) const { return false; }
};

class NimBLEAddress {
 public:
  NimBLEAddress() {}
  NimBLEAddress(const std::string&) {}
  std::string toString() const { return ""; }
};

// 1.4.x returns this from readValue(), NOT std::string.
class NimBLEAttValue {
 public:
  uint16_t length() const { return 0; }
  uint16_t size() const { return 0; }
  const uint8_t* data() const { return nullptr; }
  const char* c_str() const { return ""; }
  uint8_t operator[](int) const { return 0; }
  operator std::string() const { return ""; }
};

class NimBLERemoteCharacteristic;

typedef std::function<void(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool)> notify_callback;

class NimBLERemoteDescriptor {
 public:
  uint16_t getHandle() { return 0; }
  NimBLEUUID getUUID() { return NimBLEUUID(); }
  NimBLEAttValue readValue() { return NimBLEAttValue(); }
};

class NimBLERemoteService;

class NimBLERemoteCharacteristic {
 public:
  NimBLEUUID getUUID() { return NimBLEUUID(); }
  uint16_t getHandle() { return 0; }
  bool canNotify() { return true; }
  bool canRead() { return true; }
  NimBLEAttValue readValue(time_t* = nullptr) { return NimBLEAttValue(); }
  bool writeValue(const uint8_t*, size_t, bool = false) { return true; }
  bool subscribe(bool = true, notify_callback = nullptr, bool = false) { return true; }
  NimBLERemoteDescriptor* getDescriptor(const NimBLEUUID&) { return nullptr; }
};

class NimBLERemoteService {
 public:
  NimBLERemoteCharacteristic* getCharacteristic(const NimBLEUUID&) { return nullptr; }
  std::vector<NimBLERemoteCharacteristic*>* getCharacteristics(bool = false) { return nullptr; }
  NimBLEUUID getUUID() { return NimBLEUUID(); }
};

class NimBLEClient;

class NimBLEClientCallbacks {
 public:
  virtual ~NimBLEClientCallbacks() {}
  virtual void onConnect(NimBLEClient*) {}
  virtual void onDisconnect(NimBLEClient*) {}
  virtual bool onConnParamsUpdateRequest(NimBLEClient*, const ble_gap_upd_params*) { return true; }
  virtual uint32_t onPassKeyRequest() { return 0; }
  virtual void onAuthenticationComplete(ble_gap_conn_desc*) {}
  virtual bool onConfirmPIN(uint32_t) { return true; }
};

class NimBLEAdvertisedDevice;

class NimBLEClient {
 public:
  bool connect(NimBLEAdvertisedDevice*, bool = true) { return true; }
  bool connect(const NimBLEAddress&, bool = true) { return true; }
  int disconnect(uint8_t = 0) { return 0; }
  bool isConnected() { return true; }
  bool secureConnection() { return true; }
  void setClientCallbacks(NimBLEClientCallbacks*, bool = true) {}
  void setConnectTimeout(uint8_t) {}
  void setConnectionParams(uint16_t, uint16_t, uint16_t, uint16_t,
                           uint16_t = 16, uint16_t = 16) {}
  void updateConnParams(uint16_t, uint16_t, uint16_t, uint16_t) {}
  NimBLERemoteService* getService(const NimBLEUUID&) { return nullptr; }
  uint16_t getConnId() { return 0; }
};

class NimBLEAdvertisedDevice {
 public:
  NimBLEAddress getAddress() { return NimBLEAddress(); }
  uint16_t getAppearance() { return 0; }
  std::string getName() { return ""; }
  bool isAdvertisingService(const NimBLEUUID&) { return false; }
  bool haveName() { return false; }
};

class NimBLEAdvertisedDeviceCallbacks {
 public:
  virtual ~NimBLEAdvertisedDeviceCallbacks() {}
  virtual void onResult(NimBLEAdvertisedDevice*) = 0;
};

class NimBLEScanResults {};

class NimBLEScan {
 public:
  bool start(uint32_t, void (*)(NimBLEScanResults), bool = false) { return true; }
  void setAdvertisedDeviceCallbacks(NimBLEAdvertisedDeviceCallbacks*, bool = false) {}
  void setActiveScan(bool) {}
  void setInterval(uint16_t) {}
  void setWindow(uint16_t) {}
  bool stop() { return true; }
  void clearResults() {}
};

class NimBLEDevice {
 public:
  static void init(const std::string&) {}
  static void deinit(bool = false) {}
  static void setPower(esp_power_level_t, int = 0) {}
  static void setSecurityAuth(bool, bool, bool) {}
  static void setSecurityIOCap(uint8_t) {}
  static void deleteAllBonds() {}
  static int getNumBonds() { return 0; }
  static NimBLEScan* getScan() { static NimBLEScan s; return &s; }
  static NimBLEClient* createClient(NimBLEAddress = NimBLEAddress()) {
    static NimBLEClient c;
    return &c;
  }
  static bool deleteClient(NimBLEClient*) { return true; }
};
