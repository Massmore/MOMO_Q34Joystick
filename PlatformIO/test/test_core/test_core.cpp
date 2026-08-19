// MomoJoy - unit tests for the pure-C++ core.
//   PC:  pio test -e native
// The .cpp files are included directly so the test target needs no LDF magic.
#include <unity.h>

#include <string.h>

#include "core/MomoGamepadState.cpp"
#include "core/MomoHidParser.cpp"
#include "core/MomoMapper.cpp"

using namespace momojoy;

// A report descriptor in the shape an Android-mode BLE gamepad (ShanWan Q34U
// "Mode D", Betop, Ipega, ...) advertises: 4 analog axes, 2 analog triggers,
// one 4-bit hat and 15 buttons, all inside report ID 1.
static const uint8_t kAndroidPadDescriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x05, 0x02,        //   Usage Page (Simulation Controls)
    0x09, 0xC5,        //   Usage (Brake)
    0x09, 0xC4,        //   Usage (Accelerator)
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 0x02,
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (degrees)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)

    0x75, 0x04,        //   4 bits of padding
    0x95, 0x01,
    0x81, 0x03,        //   Input (Const,Var,Abs)

    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x0F,        //   Usage Maximum (Button 15)
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x0F,
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x75, 0x01,        //   1 bit of padding
    0x95, 0x01,
    0x81, 0x03,
    0xC0               // End Collection
};

// Same controller but with no Report ID (some cheap pads do this).
static const uint8_t kNoIdDescriptor[] = {
    0x05, 0x01, 0x09, 0x05, 0xA1, 0x01,
    0x09, 0x30, 0x09, 0x31,
    0x15, 0x81, 0x25, 0x7F,        // Logical -127..127 (signed axes)
    0x75, 0x08, 0x95, 0x02, 0x81, 0x02,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x08,
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
    0xC0
};

// Home / Back delivered in a second report (report ID 3) on the Consumer page.
static const uint8_t kConsumerTail[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        //   Report ID (3)
    0x0A, 0x23, 0x02,  //   Usage (AC Home)
    0x0A, 0x24, 0x02,  //   Usage (AC Back)
    0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x02,
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x75, 0x06, 0x95, 0x01, 0x81, 0x03,   // padding
    0xC0
};

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
static void test_parse_basic_layout() {
  MomoHidParser p;
  TEST_ASSERT_TRUE(p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor)));
  TEST_ASSERT_TRUE(p.usesReportIds());
  TEST_ASSERT_EQUAL_UINT(1, p.reportCount());
  TEST_ASSERT_EQUAL_UINT(1, p.reportIdAt(0));
  TEST_ASSERT_EQUAL_UINT(9, p.reportSizeBytes(1));   // 4+2+1+2 bytes
  TEST_ASSERT_FALSE(p.overflowed());
}

static void test_parse_axis_fields() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));

  const HidField* x = p.find(kPageGenericDesktop, kUsageX);
  TEST_ASSERT_NOT_NULL(x);
  TEST_ASSERT_EQUAL_UINT(0, x->bitOffset);
  TEST_ASSERT_EQUAL_UINT(8, x->bitSize);
  TEST_ASSERT_EQUAL_INT(0, x->logicalMin);
  TEST_ASSERT_EQUAL_INT(255, x->logicalMax);

  const HidField* rz = p.find(kPageGenericDesktop, kUsageRz);
  TEST_ASSERT_NOT_NULL(rz);
  TEST_ASSERT_EQUAL_UINT(24, rz->bitOffset);

  const HidField* brake = p.find(kPageSimulation, kUsageBrake);
  TEST_ASSERT_NOT_NULL(brake);
  TEST_ASSERT_EQUAL_UINT(32, brake->bitOffset);

  const HidField* accel = p.find(kPageSimulation, kUsageAccelerator);
  TEST_ASSERT_NOT_NULL(accel);
  TEST_ASSERT_EQUAL_UINT(40, accel->bitOffset);
}

static void test_parse_hat_and_buttons() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));

  const HidField* hat = p.find(kPageGenericDesktop, kUsageHat);
  TEST_ASSERT_NOT_NULL(hat);
  TEST_ASSERT_EQUAL_UINT(48, hat->bitOffset);
  TEST_ASSERT_EQUAL_UINT(4, hat->bitSize);
  TEST_ASSERT_EQUAL_INT(7, hat->logicalMax);

  const HidField* b = p.findButtonBlock();
  TEST_ASSERT_NOT_NULL(b);
  TEST_ASSERT_EQUAL_UINT(56, b->bitOffset);   // padding after hat consumed 4 bits
  TEST_ASSERT_EQUAL_UINT(1, b->usage);        // Button 1

  int buttonFields = 0;
  for (size_t i = 0; i < p.fieldCount(); ++i) {
    if (p.field(i).usagePage == kPageButton) buttonFields++;
  }
  TEST_ASSERT_EQUAL_INT(15, buttonFields);
}

static void test_parse_without_report_id() {
  MomoHidParser p;
  TEST_ASSERT_TRUE(p.parse(kNoIdDescriptor, sizeof(kNoIdDescriptor)));
  TEST_ASSERT_FALSE(p.usesReportIds());
  TEST_ASSERT_EQUAL_UINT(3, p.reportSizeBytes(0));
  const HidField* x = p.find(kPageGenericDesktop, kUsageX);
  TEST_ASSERT_NOT_NULL(x);
  TEST_ASSERT_TRUE(x->isSigned());
  TEST_ASSERT_EQUAL_INT(-127, x->logicalMin);
}

// ---------------------------------------------------------------------------
static void test_bit_extraction() {
  const uint8_t data[] = {0xAB, 0xCD, 0xEF};
  TEST_ASSERT_EQUAL_INT(0xAB, hidExtract(data, 3, 0, 8, false));
  TEST_ASSERT_EQUAL_INT(0x0B, hidExtract(data, 3, 0, 4, false));
  TEST_ASSERT_EQUAL_INT(0x0A, hidExtract(data, 3, 4, 4, false));
  TEST_ASSERT_EQUAL_INT(0xDAB, hidExtract(data, 3, 0, 12, false));
  TEST_ASSERT_EQUAL_INT(1, hidExtract(data, 3, 0, 1, false));
  TEST_ASSERT_EQUAL_INT(0, hidExtract(data, 3, 2, 1, false));
  // out of range -> 0, never a buffer overrun
  TEST_ASSERT_EQUAL_INT(0, hidExtract(data, 3, 20, 8, false));
  // sign extension
  const uint8_t neg[] = {0x80};
  TEST_ASSERT_EQUAL_INT(-128, hidExtract(neg, 1, 0, 8, true));
  TEST_ASSERT_EQUAL_INT(128, hidExtract(neg, 1, 0, 8, false));
}

// ---------------------------------------------------------------------------
static void test_axis_scaling() {
  TEST_ASSERT_EQUAL_INT(-512, scaleAxis(0, 0, 255));
  TEST_ASSERT_EQUAL_INT(511, scaleAxis(255, 0, 255));
  TEST_ASSERT_INT_WITHIN(4, 0, scaleAxis(128, 0, 255));
  TEST_ASSERT_INT_WITHIN(4, 0, scaleAxis(0, -127, 127));
  // clamping
  TEST_ASSERT_EQUAL_INT(-512, scaleAxis(-999, 0, 255));
  TEST_ASSERT_EQUAL_INT(511, scaleAxis(999, 0, 255));
  // degenerate descriptor
  TEST_ASSERT_EQUAL_INT(0, scaleAxis(50, 10, 10));
}

static void test_trigger_scaling() {
  TEST_ASSERT_EQUAL_UINT(0, scaleTrigger(0, 0, 255));
  TEST_ASSERT_EQUAL_UINT(1023, scaleTrigger(255, 0, 255));
  TEST_ASSERT_INT_WITHIN(4, 512, scaleTrigger(128, 0, 255));
}

static void test_hat_to_dpad() {
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_UP, hatToDpad(0, 0, 7));
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_UP | MOMO_DPAD_RIGHT, hatToDpad(1, 0, 7));
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_RIGHT, hatToDpad(2, 0, 7));
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_DOWN, hatToDpad(4, 0, 7));
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_LEFT, hatToDpad(6, 0, 7));
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_UP | MOMO_DPAD_LEFT, hatToDpad(7, 0, 7));
  // null / centred
  TEST_ASSERT_EQUAL_UINT(0, hatToDpad(8, 0, 7));
  TEST_ASSERT_EQUAL_UINT(0, hatToDpad(15, 0, 7));
  // 1..8 style
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_UP, hatToDpad(1, 1, 8));
  TEST_ASSERT_EQUAL_UINT(MOMO_DPAD_DOWN, hatToDpad(5, 1, 8));
}

static void test_deadzone() {
  int16_t x = 10, y = 10;
  applyDeadzone(x, y, 24);
  TEST_ASSERT_EQUAL_INT(0, x);
  TEST_ASSERT_EQUAL_INT(0, y);

  x = 300; y = 0;
  applyDeadzone(x, y, 24);
  TEST_ASSERT_EQUAL_INT(300, x);
}

// ---------------------------------------------------------------------------
static void test_mapper_decodes_full_report() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));

  MomoMapper m;
  m.setProfile(&kProfileAndroidGamepad);
  m.setParser(&p);
  m.setDeadzone(24);
  TEST_ASSERT_TRUE(m.ready());

  // X=0 (full left), Y=255 (full down), Z=128, Rz=128,
  // brake=255, accel=0, hat=2 (right), buttons: 1 (A) and 12 (Start)
  uint8_t payload[9] = {0};
  payload[0] = 0;      // X
  payload[1] = 255;    // Y
  payload[2] = 128;    // Z
  payload[3] = 128;    // Rz
  payload[4] = 255;    // brake  -> L2
  payload[5] = 0;      // accel  -> R2
  payload[6] = 0x02;   // hat = 2 (right), high nibble padding
  payload[7] = 0x01;   // button 1 (A)
  payload[8] = 0x08;   // buttons 9..15 live here; bit3 = button 12 (Start)

  MomoGamepadState s;
  TEST_ASSERT_TRUE(m.decode(1, payload, sizeof(payload), s));

  TEST_ASSERT_EQUAL_INT(-512, s.lx);
  TEST_ASSERT_EQUAL_INT(511, s.ly);
  TEST_ASSERT_INT_WITHIN(4, 0, s.rx);
  TEST_ASSERT_INT_WITHIN(4, 0, s.ry);
  TEST_ASSERT_EQUAL_UINT(1023, s.l2);
  TEST_ASSERT_EQUAL_UINT(0, s.r2);
  TEST_ASSERT_TRUE(s.dpadRight());
  TEST_ASSERT_FALSE(s.dpadUp());
  TEST_ASSERT_TRUE(s.pressed(MOMO_BTN_A));
  TEST_ASSERT_TRUE(s.pressed(MOMO_BTN_START));
  TEST_ASSERT_TRUE(s.pressed(MOMO_BTN_L2));   // derived from the analog brake
  TEST_ASSERT_FALSE(s.pressed(MOMO_BTN_B));
  TEST_ASSERT_FALSE(s.pressed(MOMO_BTN_R2));
  TEST_ASSERT_EQUAL_UINT(1, s.seq);
}

static void test_mapper_centre_is_zero_after_deadzone() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));
  MomoMapper m;
  m.setParser(&p);
  m.setDeadzone(24);

  uint8_t payload[9] = {128, 128, 128, 128, 0, 0, 0x0F, 0x00, 0x00};
  MomoGamepadState s;
  TEST_ASSERT_TRUE(m.decode(1, payload, sizeof(payload), s));
  TEST_ASSERT_EQUAL_INT(0, s.lx);
  TEST_ASSERT_EQUAL_INT(0, s.ly);
  TEST_ASSERT_EQUAL_INT(0, s.rx);
  TEST_ASSERT_EQUAL_INT(0, s.ry);
  TEST_ASSERT_EQUAL_UINT(0, s.dpad);      // hat = 15 -> centred
  TEST_ASSERT_EQUAL_UINT(0, s.buttons);
}

static void test_mapper_rejects_wrong_report_id() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));
  MomoMapper m;
  m.setParser(&p);

  uint8_t payload[9] = {0};
  MomoGamepadState s;
  TEST_ASSERT_FALSE(m.decode(3, payload, sizeof(payload), s));
}

static void test_mapper_short_payload_is_safe() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));
  MomoMapper m;
  m.setParser(&p);

  uint8_t payload[3] = {0, 255, 128};
  MomoGamepadState s;
  TEST_ASSERT_TRUE(m.decode(1, payload, sizeof(payload), s));
  TEST_ASSERT_EQUAL_INT(-512, s.lx);
  TEST_ASSERT_EQUAL_INT(511, s.ly);
  TEST_ASSERT_EQUAL_UINT(0, s.buttons);   // missing bytes read as 0, no crash
}

static void test_malformed_descriptor_does_not_crash() {
  const uint8_t junk[] = {0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x81};  // truncated
  MomoHidParser p;
  p.parse(junk, sizeof(junk));   // must simply return, no UB
  TEST_ASSERT_TRUE(true);

  MomoHidParser p2;
  TEST_ASSERT_FALSE(p2.parse(nullptr, 0));
}

static void test_consumer_report_home_and_back() {
  uint8_t combined[sizeof(kAndroidPadDescriptor) + sizeof(kConsumerTail)];
  memcpy(combined, kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));
  memcpy(combined + sizeof(kAndroidPadDescriptor), kConsumerTail, sizeof(kConsumerTail));

  MomoHidParser p;
  TEST_ASSERT_TRUE(p.parse(combined, sizeof(combined)));
  TEST_ASSERT_EQUAL_UINT(2, p.reportCount());
  TEST_ASSERT_EQUAL_UINT(1, p.reportSizeBytes(3));

  const HidField* home = p.find(kPageConsumer, kUsageConsumerHome, 3);
  TEST_ASSERT_NOT_NULL(home);
  TEST_ASSERT_EQUAL_UINT(0, home->bitOffset);

  MomoMapper m;
  m.setParser(&p);
  m.setDeadzone(24);

  MomoGamepadState s;

  // HOME pressed in report 3
  uint8_t consumer[1] = {0x01};
  TEST_ASSERT_TRUE(m.decode(3, consumer, sizeof(consumer), s));
  TEST_ASSERT_TRUE(s.pressed(MOMO_BTN_HOME));

  // A normal gamepad report must not wipe it
  uint8_t pad[9] = {128, 128, 128, 128, 0, 0, 0x0F, 0x00, 0x00};
  TEST_ASSERT_TRUE(m.decode(1, pad, sizeof(pad), s));
  TEST_ASSERT_TRUE(s.pressed(MOMO_BTN_HOME));

  // Release
  consumer[0] = 0x00;
  TEST_ASSERT_TRUE(m.decode(3, consumer, sizeof(consumer), s));
  TEST_ASSERT_FALSE(s.pressed(MOMO_BTN_HOME));

  // BACK -> SELECT
  consumer[0] = 0x02;
  TEST_ASSERT_TRUE(m.decode(3, consumer, sizeof(consumer), s));
  TEST_ASSERT_TRUE(s.pressed(MOMO_BTN_SELECT));
}

static void test_unknown_report_id_still_rejected() {
  MomoHidParser p;
  p.parse(kAndroidPadDescriptor, sizeof(kAndroidPadDescriptor));
  MomoMapper m;
  m.setParser(&p);
  uint8_t payload[4] = {1, 2, 3, 4};
  MomoGamepadState s;
  TEST_ASSERT_FALSE(m.decode(7, payload, sizeof(payload), s));
}

// ---------------------------------------------------------------------------
int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_basic_layout);
  RUN_TEST(test_parse_axis_fields);
  RUN_TEST(test_parse_hat_and_buttons);
  RUN_TEST(test_parse_without_report_id);
  RUN_TEST(test_bit_extraction);
  RUN_TEST(test_axis_scaling);
  RUN_TEST(test_trigger_scaling);
  RUN_TEST(test_hat_to_dpad);
  RUN_TEST(test_deadzone);
  RUN_TEST(test_mapper_decodes_full_report);
  RUN_TEST(test_mapper_centre_is_zero_after_deadzone);
  RUN_TEST(test_mapper_rejects_wrong_report_id);
  RUN_TEST(test_mapper_short_payload_is_safe);
  RUN_TEST(test_consumer_report_home_and_back);
  RUN_TEST(test_unknown_report_id_still_rejected);
  RUN_TEST(test_malformed_descriptor_does_not_crash);
  return UNITY_END();
}
