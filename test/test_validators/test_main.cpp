#include <Arduino.h>
#include <unity.h>

#include "config.h"

void test_isValidNumber() {
  TEST_ASSERT_TRUE(isValidNumber("123"));
  TEST_ASSERT_TRUE(isValidNumber("a1"));
  TEST_ASSERT_FALSE(isValidNumber("abc"));
  TEST_ASSERT_FALSE(isValidNumber(""));
}

void test_isValidEmail() {
  TEST_ASSERT_TRUE(isValidEmail("user@example.com"));
  TEST_ASSERT_TRUE(isValidEmail("u@e.co"));
  TEST_ASSERT_FALSE(isValidEmail("userexample.com"));
  TEST_ASSERT_FALSE(isValidEmail("user@ example.com"));
  TEST_ASSERT_FALSE(isValidEmail("user@ex"));
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_isValidNumber);
  RUN_TEST(test_isValidEmail);
  UNITY_END();
}

void loop() {}

