#include <Arduino.h>
#include <unity.h>

#include "api.h"

void test_sha256hex_abc() {
  String out = sha256hex("abc");
  TEST_ASSERT_EQUAL_STRING_LEN("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", out.c_str(), 64);
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_sha256hex_abc);
  UNITY_END();
}

void loop() {}

