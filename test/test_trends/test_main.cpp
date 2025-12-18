#include <Arduino.h>
#include <unity.h>

#include "api.h"

void test_dexcom_trend_mapping() {
  TEST_ASSERT_EQUAL(1, testDexcomTrendToArrow("DoubleDown"));
  TEST_ASSERT_EQUAL(2, testDexcomTrendToArrow("SingleDown"));
  TEST_ASSERT_EQUAL(3, testDexcomTrendToArrow("FortyFiveDown"));
  TEST_ASSERT_EQUAL(4, testDexcomTrendToArrow("Flat"));
  TEST_ASSERT_EQUAL(5, testDexcomTrendToArrow("FortyFiveUp"));
  TEST_ASSERT_EQUAL(6, testDexcomTrendToArrow("SingleUp"));
  TEST_ASSERT_EQUAL(7, testDexcomTrendToArrow("DoubleUp"));
}

void test_libre_trend_mapping() {
  // Raw: 1=DoubleUp,2=SingleUp,3=FortyFiveUp,4=Flat,5=FortyFiveDown,6=SingleDown,7=DoubleDown
  TEST_ASSERT_EQUAL(5, testLibreTrendToArrow(1));
  TEST_ASSERT_EQUAL(5, testLibreTrendToArrow(2));
  TEST_ASSERT_EQUAL(4, testLibreTrendToArrow(3));
  TEST_ASSERT_EQUAL(3, testLibreTrendToArrow(4));
  TEST_ASSERT_EQUAL(2, testLibreTrendToArrow(5));
  TEST_ASSERT_EQUAL(1, testLibreTrendToArrow(6));
  TEST_ASSERT_EQUAL(1, testLibreTrendToArrow(7));
}

void test_parse_dex_date_ms() {
  auto toStr = [](int64_t v) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", (long long)v);
    return String(buf);
  };
  String t1 = toStr(testParseDexDateMs("Date(1765834651426)"));
  String t2 = toStr(testParseDexDateMs("Date(1765834651426+0100)"));
  TEST_ASSERT_EQUAL_STRING("1765834651426", t1.c_str());
  TEST_ASSERT_EQUAL_STRING("1765834651426", t2.c_str());
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_dexcom_trend_mapping);
  RUN_TEST(test_libre_trend_mapping);
  RUN_TEST(test_parse_dex_date_ms);
  UNITY_END();
}

void loop() {}
