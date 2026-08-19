// =============================================================================
//  MomoJoy - BLE HID gamepad host for ESP32-S3 (Arduino core)
//  Target device: ShanWan Q34U in "Mode D" (Android / BLE HID)
//
//  Written from scratch on top of NimBLE-Arduino: scan -> pair/bond ->
//  read HID Report Map -> parse descriptor -> subscribe to input reports ->
//  decode into a normalised gamepad state.
//
//  Value ranges are Bluepad32-compatible on purpose:
//      sticks   -512 .. 511
//      triggers     0 .. 1023
//  so an existing MOMO sketch only needs its accessors renamed.
// =============================================================================
#pragma once

#include <Arduino.h>

#include "core/MomoGamepadState.h"
#include "core/MomoHidParser.h"
#include "core/MomoHidUsage.h"
#include "core/MomoMapper.h"

namespace momojoy {

struct MomoJoyOptions {
  // Local BLE name of the ESP32 (only used during pairing).
  const char* localName = "MOMO";

  // Connect only to a controller whose advertised name contains this string.
  // nullptr = accept any device advertising the HID service (0x1812).
  const char* nameFilter = nullptr;

  // Connect only to this MAC ("AA:BB:CC:DD:EE:FF"). nullptr = any.
  const char* addressFilter = nullptr;

  bool     autoReconnect = true;
  bool     clearBondsOnBoot = false;
  uint16_t deadzone = 24;             // in output units (0..512)
  uint16_t connectTimeoutSec = 10;
  const MomoProfile* profile = &kProfileAndroidGamepad;
  bool     verbose = true;            // log to Serial
};

typedef void (*MomoConnectCb)(const char* name, const char* address);
typedef void (*MomoDisconnectCb)();
typedef void (*MomoRawReportCb)(uint8_t reportId, const uint8_t* data, size_t len);
typedef void (*MomoButtonCb)(uint32_t pressedMask, uint32_t releasedMask);

class MomoJoyClass {
 public:
  // ---- lifecycle ----------------------------------------------------------
  bool begin(const MomoJoyOptions& options = MomoJoyOptions());
  void end();

  // Must be called from loop(). Non-blocking.
  void update();

  bool isConnected() const { return connected_; }
  bool isScanning() const  { return scanning_; }

  // Drop all stored pairings. Also power-cycle the controller afterwards.
  void forgetBonds();

  // Ask the controller to disconnect.
  void disconnect();

  // ---- state --------------------------------------------------------------
  const MomoGamepadState& state() const { return state_; }

  bool pressed(uint32_t mask) const      { return (state_.buttons & mask) != 0; }
  bool justPressed(uint32_t mask) const  { return (edgePressed_ & mask) != 0; }
  bool justReleased(uint32_t mask) const { return (edgeReleased_ & mask) != 0; }

  int16_t  lx() const { return state_.lx; }
  int16_t  ly() const { return state_.ly; }
  int16_t  rx() const { return state_.rx; }
  int16_t  ry() const { return state_.ry; }
  uint16_t l2() const { return state_.l2; }
  uint16_t r2() const { return state_.r2; }
  uint8_t  dpad() const { return state_.dpad; }
  uint8_t  battery() const { return state_.battery; }

  bool dpadUp() const    { return state_.dpadUp(); }
  bool dpadDown() const  { return state_.dpadDown(); }
  bool dpadLeft() const  { return state_.dpadLeft(); }
  bool dpadRight() const { return state_.dpadRight(); }

  // ---- tuning -------------------------------------------------------------
  void setDeadzone(uint16_t dz) { mapper_.setDeadzone(dz); }
  void setProfile(const MomoProfile* p) { mapper_.setProfile(p); }

  // ---- callbacks ----------------------------------------------------------
  void onConnect(MomoConnectCb cb)       { connectCb_ = cb; }
  void onDisconnect(MomoDisconnectCb cb) { disconnectCb_ = cb; }
  void onRawReport(MomoRawReportCb cb)   { rawCb_ = cb; }
  void onButton(MomoButtonCb cb)         { buttonCb_ = cb; }

  // ---- introspection (used by the calibration example) --------------------
  const MomoHidParser& descriptor() const { return parser_; }
  const uint8_t* reportMap() const { return reportMap_; }
  size_t reportMapLen() const { return reportMapLen_; }
  const char* peerName() const { return peerName_; }
  const char* peerAddress() const { return peerAddr_; }

  // Pretty-print helpers (see MomoJoySerial.cpp).
  void dumpReportMap(Stream& out) const;
  void dumpDescriptorFields(Stream& out) const;
  void printState(Stream& out) const;
  void printStateChanges(Stream& out);

  // Internal: called from the NimBLE task. Do not call directly.
  void _pushReport(uint8_t reportId, const uint8_t* data, size_t len);
  void _onBleConnect();
  void _onBleDisconnect();
  void _onDeviceFound(const char* name, const char* address);

 private:
  bool startScan_();
  bool connectToPeer_();
  bool discoverHid_();
  void processQueue_();

  MomoJoyOptions   opts_;
  MomoHidParser    parser_;
  MomoMapper       mapper_;
  MomoGamepadState state_;

  uint32_t prevButtons_  = 0;
  uint32_t edgePressed_  = 0;
  uint32_t edgeReleased_ = 0;
  uint8_t  prevDpad_     = 0;

  uint8_t  reportMap_[512];
  size_t   reportMapLen_ = 0;

  char peerName_[32] = {0};
  char peerAddr_[18] = {0};

  volatile bool connected_   = false;
  volatile bool scanning_    = false;
  volatile bool wantConnect_ = false;
  volatile bool needDiscover_= false;
  volatile bool lostLink_    = false;
  uint32_t      lastScanKick_ = 0;
  bool          started_      = false;

  MomoConnectCb    connectCb_    = nullptr;
  MomoDisconnectCb disconnectCb_ = nullptr;
  MomoRawReportCb  rawCb_        = nullptr;
  MomoButtonCb     buttonCb_     = nullptr;
};

}  // namespace momojoy

// Convenience: sketches normally just `using namespace momojoy;`
extern momojoy::MomoJoyClass MomoJoy;
