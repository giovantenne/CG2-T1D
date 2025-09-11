#include <Arduino.h>
#include <unity.h>

#include <EEPROM.h>
#include "board.h"
#include "config_store.h"

// Local reader to mirror how we persisted
static String readStringFrom(int addr) {
  int len = EEPROM.read(addr);
  char buf[256];
  for (int i = 0; i < len && i < 255; i++) {
    buf[i] = EEPROM.read(addr + 1 + i);
  }
  buf[len] = '\0';
  return String(buf);
}

void test_configstore_persist_roundtrip() {
  EEPROM.begin(EEPROM_SIZE);
  configStoreSetBrightness(77);
  configStoreSetCredentials("u@test.com", "pw123");
  configStoreSetAccountSha256("deadbeef");
  configStoreSetPatientId("pid-42");
  configStorePersist();

  int addr = 0;
  int b = EEPROM.read(addr++);
  TEST_ASSERT_EQUAL(77, b);
  String u = readStringFrom(addr);
  addr += 1 + u.length();
  String p = readStringFrom(addr);
  addr += 1 + p.length();
  String sha = readStringFrom(addr);
  addr += 1 + sha.length();
  String pid = readStringFrom(addr);

  TEST_ASSERT_EQUAL_STRING("u@test.com", u.c_str());
  TEST_ASSERT_EQUAL_STRING("pw123", p.c_str());
  TEST_ASSERT_EQUAL_STRING("deadbeef", sha.c_str());
  TEST_ASSERT_EQUAL_STRING("pid-42", pid.c_str());
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_configstore_persist_roundtrip);
  UNITY_END();
}

void loop() {}

