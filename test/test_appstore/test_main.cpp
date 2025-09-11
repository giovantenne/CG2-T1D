#include <Arduino.h>
#include <unity.h>

#include "app_state.h"
#include "app_store.h"

void test_setMeasurement_updates_fields() {
  currentTimestamp = "old";
  lastTimestamp = "";
  currentGlucose = "0";
  trendArrowCode = "";

  setMeasurement(123, 3, String("new"));

  TEST_ASSERT_EQUAL_STRING("old", lastTimestamp.c_str());
  TEST_ASSERT_EQUAL_STRING("new", currentTimestamp.c_str());
  TEST_ASSERT_EQUAL_STRING("123", currentGlucose.c_str());
  TEST_ASSERT_EQUAL_STRING("3", trendArrowCode.c_str());
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_setMeasurement_updates_fields);
  UNITY_END();
}

void loop() {}

