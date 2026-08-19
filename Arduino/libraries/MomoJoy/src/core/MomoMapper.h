// MomoJoy - maps a decoded HID report onto MomoGamepadState (pure C++)
#pragma once

#include "MomoGamepadState.h"
#include "MomoHidParser.h"

namespace momojoy {

// Maps HID button index (1-based, usage page 0x09) -> MomoButton bit.
struct MomoButtonMap {
  uint8_t  hidButton;
  uint32_t momoBit;
};

struct MomoProfile {
  const char*          name;
  const MomoButtonMap* buttons;
  uint8_t              buttonCount;
  bool                 invertLeftY;
  bool                 invertRightY;
};

// ShanWan Q34U in "Mode D" (Android / BLE HID) and most Android-mode BLE
// gamepads use this 15-button layout.
extern const MomoProfile kProfileAndroidGamepad;

// 1:1 fallback: HID button N -> MOMO_BTN bit N-1.
extern const MomoProfile kProfileGeneric;

class MomoMapper {
 public:
  void setParser(const MomoHidParser* parser) { parser_ = parser; resolve(); }
  void setProfile(const MomoProfile* profile) { profile_ = profile ? profile : &kProfileAndroidGamepad; }
  void setDeadzone(uint16_t dz) { deadzone_ = dz; }
  uint16_t deadzone() const { return deadzone_; }
  const MomoProfile* profile() const { return profile_; }

  // Re-scan the parsed descriptor and cache the field pointers we need.
  void resolve();

  // Decode one input report payload (report ID already stripped).
  // Returns true when the report matched a known layout.
  bool decode(uint8_t reportId, const uint8_t* payload, size_t len,
              MomoGamepadState& out) const;

  bool ready() const { return parser_ != nullptr && resolved_; }

  // Introspection, used by the RawDump example / calibration tool.
  const HidField* axisField(uint8_t idx) const { return axes_[idx]; }

 private:
  const MomoHidParser* parser_  = nullptr;
  const MomoProfile*   profile_ = &kProfileAndroidGamepad;
  uint16_t             deadzone_ = 24;

  // Some controllers ship Home / Back in a separate Consumer-page report.
  struct ConsumerKey {
    const HidField* field;
    uint32_t        momoBit;
  };
  static const uint8_t kMaxConsumerKeys = 6;

  bool decodeConsumer_(const uint8_t* payload, size_t len, MomoGamepadState& out) const;

  // axes_[0..3] = LX, LY, RX, RY ; axes_[4..5] = L2, R2
  const HidField* axes_[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
  const HidField* hat_     = nullptr;
  const HidField* buttons_ = nullptr;   // first field of the button block
  uint8_t         buttonBits_ = 0;      // how many button fields follow
  uint8_t         inputReportId_ = 0;

  ConsumerKey     consumer_[kMaxConsumerKeys] = {};
  uint8_t         consumerCount_ = 0;
  uint8_t         consumerReportId_ = 0xFF;

  bool            resolved_ = false;
};

}  // namespace momojoy
