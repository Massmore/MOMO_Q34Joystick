// MomoJoy - BLE HID host implementation (NimBLE-Arduino 1.4.x, Arduino-ESP32)
#include "MomoJoy.h"

#if defined(ARDUINO_ARCH_ESP32)

#include <NimBLEDevice.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <strings.h>   // strcasecmp

using namespace momojoy;

momojoy::MomoJoyClass MomoJoy;

namespace {

// ---- BLE UUIDs -------------------------------------------------------------
const uint16_t kSvcHid            = 0x1812;
const uint16_t kSvcBattery        = 0x180F;
const uint16_t kChrReportMap      = 0x2A4B;
const uint16_t kChrReport         = 0x2A4D;
const uint16_t kChrProtocolMode   = 0x2A4E;
const uint16_t kChrHidControl     = 0x2A4C;
const uint16_t kChrBatteryLevel   = 0x2A19;
const uint16_t kDscReportRef      = 0x2908;

const uint16_t kAppearanceGamepad = 0x03C4;
const uint16_t kAppearanceHidMask = 0x03C0;

// ---- Report queue ----------------------------------------------------------
struct RawReport {
  uint8_t id;
  uint8_t len;
  uint8_t data[24];
};

QueueHandle_t g_queue = nullptr;

// ---- BLE globals -----------------------------------------------------------
NimBLEClient*          g_client = nullptr;
NimBLEAddress          g_target;
bool                   g_haveTarget = false;
volatile uint8_t       g_batteryLevel = 0xFF;

// Map: characteristic handle -> HID report ID (input reports only).
struct ReportBinding {
  uint16_t handle;
  uint8_t  reportId;
};
ReportBinding g_bindings[8];
uint8_t       g_bindingCount = 0;

uint8_t reportIdForHandle(uint16_t handle) {
  for (uint8_t i = 0; i < g_bindingCount; ++i) {
    if (g_bindings[i].handle == handle) return g_bindings[i].reportId;
  }
  return 0;
}

void notifyCb(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t len, bool) {
  if (chr == nullptr) return;
  if (chr->getUUID().equals(NimBLEUUID(kChrBatteryLevel))) {
    if (len > 0) g_batteryLevel = data[0];
    return;
  }
  MomoJoy._pushReport(reportIdForHandle(chr->getHandle()), data, len);
}

// ---- Scan callbacks --------------------------------------------------------
class ScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
 public:
  const MomoJoyOptions* opts = nullptr;

  void onResult(NimBLEAdvertisedDevice* dev) override {
    if (dev == nullptr || opts == nullptr) return;

    const std::string name = dev->getName();
    const std::string addr = dev->getAddress().toString();

    if (opts->addressFilter != nullptr) {
      if (strcasecmp(opts->addressFilter, addr.c_str()) != 0) return;
    }
    if (opts->nameFilter != nullptr) {
      if (name.find(opts->nameFilter) == std::string::npos) return;
    } else {
      const bool advHid = dev->isAdvertisingService(NimBLEUUID(kSvcHid));
      const uint16_t app = dev->getAppearance();
      const bool hidLooking = (app == kAppearanceGamepad) ||
                              ((app & 0xFFC0) == kAppearanceHidMask);
      if (!advHid && !hidLooking) return;
    }

    g_target = dev->getAddress();
    g_haveTarget = true;
    NimBLEDevice::getScan()->stop();
    MomoJoy._onDeviceFound(name.c_str(), addr.c_str());
  }
};

ScanCallbacks g_scanCb;

// ---- Client callbacks ------------------------------------------------------
class ClientCallbacks : public NimBLEClientCallbacks {
 public:
  void onConnect(NimBLEClient* c) override {
    // Ask for a fast, stable connection: 15 ms interval, 3 s supervision.
    c->updateConnParams(12, 12, 0, 300);
    MomoJoy._onBleConnect();
  }

  void onDisconnect(NimBLEClient*) override {
    MomoJoy._onBleDisconnect();
  }

  bool onConnParamsUpdateRequest(NimBLEClient*, const ble_gap_upd_params* p) override {
    return p->itvl_min >= 6 && p->itvl_max <= 400;
  }

  uint32_t onPassKeyRequest() override { return 0; }
  bool onConfirmPIN(uint32_t) override { return true; }

  void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
    if (desc != nullptr && !desc->sec_state.encrypted) {
      // Almost always a stale bond on one of the two sides. Clearing the bond
      // on the ESP32 and re-pairing the controller fixes it.
      Serial.println(F("[MomoJoy] encryption failed - try MomoJoy.forgetBonds()"));
    }
  }
};

ClientCallbacks g_clientCb;

}  // namespace

// =============================================================================
namespace momojoy {

bool MomoJoyClass::begin(const MomoJoyOptions& options) {
  if (started_) return true;
  opts_ = options;

  mapper_.setProfile(opts_.profile);
  mapper_.setDeadzone(opts_.deadzone);

  if (g_queue == nullptr) {
    g_queue = xQueueCreate(16, sizeof(RawReport));
    if (g_queue == nullptr) return false;
  }

  NimBLEDevice::init(opts_.localName ? opts_.localName : "MOMO");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(/*bonding=*/true, /*mitm=*/false, /*sc=*/true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  if (opts_.clearBondsOnBoot) {
    NimBLEDevice::deleteAllBonds();
  }

  g_scanCb.opts = &opts_;
  started_ = true;

  if (opts_.verbose) {
    Serial.println(F("[MomoJoy] BLE HID host started"));
  }
  return startScan_();
}

void MomoJoyClass::end() {
  if (!started_) return;
  disconnect();
  NimBLEDevice::deinit(true);
  started_ = false;
}

bool MomoJoyClass::startScan_() {
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(&g_scanCb, false);
  scan->setInterval(45);
  scan->setWindow(15);
  scan->setActiveScan(true);
  scanning_ = scan->start(0, nullptr, false);
  lastScanKick_ = millis();
  if (opts_.verbose && scanning_) {
    Serial.println(F("[MomoJoy] scanning for a BLE HID gamepad..."));
  }
  return scanning_;
}

void MomoJoyClass::_onDeviceFound(const char* name, const char* address) {
  strncpy(peerName_, name ? name : "", sizeof(peerName_) - 1);
  peerName_[sizeof(peerName_) - 1] = 0;
  strncpy(peerAddr_, address ? address : "", sizeof(peerAddr_) - 1);
  peerAddr_[sizeof(peerAddr_) - 1] = 0;
  scanning_ = false;
  wantConnect_ = true;
}

void MomoJoyClass::_onBleConnect() {
  needDiscover_ = true;
}

void MomoJoyClass::_onBleDisconnect() {
  connected_ = false;
  lostLink_ = true;
}

void MomoJoyClass::_pushReport(uint8_t reportId, const uint8_t* data, size_t len) {
  if (g_queue == nullptr || data == nullptr) return;
  RawReport r;
  r.id  = reportId;
  r.len = static_cast<uint8_t>(len > sizeof(r.data) ? sizeof(r.data) : len);
  memcpy(r.data, data, r.len);
  // Called from the NimBLE host task (not an ISR): never block the stack.
  if (xQueueSend(g_queue, &r, 0) != pdTRUE) {
    RawReport drop;
    xQueueReceive(g_queue, &drop, 0);   // drop the oldest, keep the newest
    xQueueSend(g_queue, &r, 0);
  }
}

bool MomoJoyClass::connectToPeer_() {
  if (!g_haveTarget) return false;

  if (g_client == nullptr) {
    g_client = NimBLEDevice::createClient();
    g_client->setClientCallbacks(&g_clientCb, false);
    g_client->setConnectionParams(12, 12, 0, 300);
    g_client->setConnectTimeout(static_cast<uint8_t>(opts_.connectTimeoutSec));
  }

  if (opts_.verbose) {
    Serial.printf("[MomoJoy] connecting to %s (%s)\n", peerName_, peerAddr_);
  }

  if (!g_client->connect(g_target)) {
    if (opts_.verbose) Serial.println(F("[MomoJoy] connect failed"));
    return false;
  }
  // onConnect() normally sets this already; belt and braces in case the
  // callback ordering differs between NimBLE versions.
  needDiscover_ = true;
  return true;
}

bool MomoJoyClass::discoverHid_() {
  if (g_client == nullptr || !g_client->isConnected()) return false;

  // Bond / encrypt first: most controllers refuse to expose reports otherwise.
  if (!g_client->secureConnection()) {
    if (opts_.verbose) Serial.println(F("[MomoJoy] pairing failed"));
    return false;
  }

  NimBLERemoteService* hid = g_client->getService(NimBLEUUID(kSvcHid));
  if (hid == nullptr) {
    if (opts_.verbose) Serial.println(F("[MomoJoy] no HID service (0x1812) on this device"));
    return false;
  }

  // 1) Report protocol (not boot protocol).
  NimBLERemoteCharacteristic* pm = hid->getCharacteristic(NimBLEUUID(kChrProtocolMode));
  if (pm != nullptr) {
    uint8_t mode = 0x01;
    pm->writeValue(&mode, 1, false);
  }

  // 2) Wake the device up if it advertised as suspended.
  NimBLERemoteCharacteristic* cp = hid->getCharacteristic(NimBLEUUID(kChrHidControl));
  if (cp != nullptr) {
    uint8_t exitSuspend = 0x00;
    cp->writeValue(&exitSuspend, 1, false);
  }

  // 3) Report Map -> descriptor parser.
  NimBLERemoteCharacteristic* rm = hid->getCharacteristic(NimBLEUUID(kChrReportMap));
  if (rm == nullptr) {
    if (opts_.verbose) Serial.println(F("[MomoJoy] no Report Map characteristic"));
    return false;
  }
  // NimBLE-Arduino 1.4.x returns NimBLEAttValue (not std::string).
  NimBLEAttValue map = rm->readValue();
  reportMapLen_ = map.length() > sizeof(reportMap_) ? sizeof(reportMap_) : map.length();
  memcpy(reportMap_, map.data(), reportMapLen_);

  parser_.parse(reportMap_, reportMapLen_);
  mapper_.setParser(&parser_);

  if (opts_.verbose) {
    Serial.printf("[MomoJoy] report map: %u bytes, %u input fields, %u report id(s)\n",
                  static_cast<unsigned>(reportMapLen_),
                  static_cast<unsigned>(parser_.fieldCount()),
                  static_cast<unsigned>(parser_.reportCount()));
  }

  // 4) Subscribe to every input report characteristic.
  g_bindingCount = 0;
  std::vector<NimBLERemoteCharacteristic*>* chars = hid->getCharacteristics(true);
  if (chars == nullptr) return false;
  uint8_t subscribed = 0;
  for (auto* chr : *chars) {
    if (chr == nullptr) continue;
    if (!chr->getUUID().equals(NimBLEUUID(kChrReport))) continue;
    if (!chr->canNotify()) continue;

    uint8_t reportId = 0;
    NimBLERemoteDescriptor* ref = chr->getDescriptor(NimBLEUUID(kDscReportRef));
    if (ref != nullptr) {
      NimBLEAttValue v = ref->readValue();
      if (v.length() >= 2) {
        if (v[1] != 0x01) continue;  // 1 = Input, 2 = Output, 3 = Feature
        reportId = v[0];
      }
    }

    if (chr->subscribe(true, notifyCb)) {
      if (g_bindingCount < 8) {
        g_bindings[g_bindingCount].handle   = chr->getHandle();
        g_bindings[g_bindingCount].reportId = reportId;
        g_bindingCount++;
      }
      subscribed++;
    }
  }

  if (opts_.verbose) {
    Serial.printf("[MomoJoy] subscribed to %u input report(s)\n", subscribed);
  }
  if (subscribed == 0) return false;

  // 5) Battery level (optional).
  NimBLERemoteService* bat = g_client->getService(NimBLEUUID(kSvcBattery));
  if (bat != nullptr) {
    NimBLERemoteCharacteristic* lvl = bat->getCharacteristic(NimBLEUUID(kChrBatteryLevel));
    if (lvl != nullptr) {
      if (lvl->canRead()) {
        NimBLEAttValue v = lvl->readValue();
        if (v.length() > 0) g_batteryLevel = v[0];
      }
      if (lvl->canNotify()) lvl->subscribe(true, notifyCb);
    }
  }

  connected_ = true;
  state_ = MomoGamepadState();
  prevButtons_ = 0;
  prevDpad_ = 0;

  if (connectCb_) connectCb_(peerName_, peerAddr_);
  if (opts_.verbose) Serial.println(F("[MomoJoy] ready"));
  return true;
}

void MomoJoyClass::processQueue_() {
  RawReport r;
  edgePressed_ = 0;
  edgeReleased_ = 0;

  while (g_queue != nullptr && xQueueReceive(g_queue, &r, 0) == pdTRUE) {
    if (rawCb_) rawCb_(r.id, r.data, r.len);

    MomoGamepadState next = state_;
    if (mapper_.decode(r.id, r.data, r.len, next)) {
      next.battery = g_batteryLevel;
      state_ = next;

      const uint32_t changed = state_.buttons ^ prevButtons_;
      edgePressed_  |= changed & state_.buttons;
      edgeReleased_ |= changed & prevButtons_;
      prevButtons_   = state_.buttons;
      prevDpad_      = state_.dpad;
    }
  }

  if (buttonCb_ && (edgePressed_ || edgeReleased_)) {
    buttonCb_(edgePressed_, edgeReleased_);
  }
}

void MomoJoyClass::update() {
  if (!started_) return;

  if (lostLink_) {
    lostLink_ = false;
    connected_ = false;
    g_haveTarget = false;
    g_bindingCount = 0;
    state_ = MomoGamepadState();
    prevButtons_ = 0;
    prevDpad_ = 0;
    edgePressed_ = 0;
    edgeReleased_ = 0;
    if (opts_.verbose) Serial.println(F("[MomoJoy] disconnected"));
    if (disconnectCb_) disconnectCb_();
    if (opts_.autoReconnect) startScan_();
    return;
  }

  if (wantConnect_) {
    wantConnect_ = false;
    if (!connectToPeer_()) {
      g_haveTarget = false;
      if (opts_.autoReconnect) startScan_();
    }
    return;
  }

  if (needDiscover_) {
    needDiscover_ = false;
    if (!discoverHid_()) {
      if (g_client && g_client->isConnected()) g_client->disconnect();
      g_haveTarget = false;
      if (opts_.autoReconnect) startScan_();
    }
    return;
  }

  if (connected_) {
    processQueue_();
    return;
  }

  // Watchdog: restart scanning if it stalled.
  if (opts_.autoReconnect && !scanning_ && !g_haveTarget &&
      (millis() - lastScanKick_) > 5000) {
    startScan_();
  }
}

void MomoJoyClass::forgetBonds() {
  NimBLEDevice::deleteAllBonds();
  if (opts_.verbose) Serial.println(F("[MomoJoy] all bonds deleted"));
}

void MomoJoyClass::disconnect() {
  if (g_client && g_client->isConnected()) g_client->disconnect();
}

}  // namespace momojoy

#endif  // ARDUINO_ARCH_ESP32
