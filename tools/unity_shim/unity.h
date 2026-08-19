// Minimal Unity-compatible shim so `tools/run_native_tests.sh` can run the same
// test sources with a plain g++ when the PlatformIO registry is unreachable.
// On a normal machine `pio test -e native` uses the real Unity instead.
#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void setUp(void);
void tearDown(void);

#ifdef __cplusplus
}
#endif

static int  unity_failures = 0;
static int  unity_tests = 0;
static int  unity_current_failed = 0;
static const char* unity_current_name = "";

#define UNITY_BEGIN() (unity_failures = 0, unity_tests = 0, 0)

#define UNITY_END()                                                          \
  (printf("\n-----------------------\n%d Tests %d Failures 0 Ignored\n%s\n", \
          unity_tests, unity_failures,                                       \
          unity_failures ? "FAIL" : "OK"),                                   \
   unity_failures)

#define RUN_TEST(fn)                                     \
  do {                                                   \
    unity_current_name = #fn;                            \
    unity_current_failed = 0;                            \
    unity_tests++;                                       \
    setUp();                                             \
    fn();                                                \
    tearDown();                                          \
    if (!unity_current_failed) printf("%s:PASS\n", #fn); \
  } while (0)

#define UNITY_FAIL_(fmt, ...)                                              \
  do {                                                                     \
    if (!unity_current_failed) unity_failures++;                           \
    unity_current_failed = 1;                                              \
    printf("%s:FAIL: line %d: " fmt "\n", unity_current_name, __LINE__,    \
           __VA_ARGS__);                                                   \
  } while (0)

#define TEST_ASSERT_TRUE(c)  do { if (!(c)) UNITY_FAIL_("expected TRUE: %s", #c); } while (0)
#define TEST_ASSERT_FALSE(c) do { if ((c))  UNITY_FAIL_("expected FALSE: %s", #c); } while (0)
#define TEST_ASSERT(c)       TEST_ASSERT_TRUE(c)
#define TEST_ASSERT_NULL(p)     do { if ((p) != NULL) UNITY_FAIL_("expected NULL: %s", #p); } while (0)
#define TEST_ASSERT_NOT_NULL(p) do { if ((p) == NULL) UNITY_FAIL_("expected non-NULL: %s", #p); } while (0)

#define TEST_ASSERT_EQUAL_INT(e, a)                                       \
  do {                                                                    \
    long long _e = (long long)(e), _a = (long long)(a);                   \
    if (_e != _a) UNITY_FAIL_("expected %lld was %lld", _e, _a);           \
  } while (0)

#define TEST_ASSERT_EQUAL_UINT(e, a)                                      \
  do {                                                                    \
    unsigned long long _e = (unsigned long long)(e);                      \
    unsigned long long _a = (unsigned long long)(a);                      \
    if (_e != _a) UNITY_FAIL_("expected %llu was %llu", _e, _a);           \
  } while (0)

#define TEST_ASSERT_EQUAL(e, a) TEST_ASSERT_EQUAL_INT(e, a)

#define TEST_ASSERT_INT_WITHIN(d, e, a)                                        \
  do {                                                                         \
    long long _d = (long long)(d), _e = (long long)(e), _a = (long long)(a);   \
    if (llabs(_e - _a) > _d)                                                   \
      UNITY_FAIL_("expected %lld +/- %lld was %lld", _e, _d, _a);              \
  } while (0)

#define TEST_ASSERT_EQUAL_STRING(e, a)                                    \
  do {                                                                    \
    if (strcmp((e), (a)) != 0) UNITY_FAIL_("expected \"%s\" was \"%s\"", (e), (a)); \
  } while (0)

#define TEST_FAIL_MESSAGE(m) UNITY_FAIL_("%s", (m))
