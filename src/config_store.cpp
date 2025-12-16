#include <EEPROM.h>
#include "app_state.h"
#include "board.h"
#include "config_store.h"

static void writeStringToEEPROMLocal(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

void configStoreSetCredentials(const String& user, const String& pass) {
  userEmail = user;
  userPassword = pass;
}

void configStoreSetAccountSha256(const String& sha256Id) {
  accountSha256 = sha256Id;
}

void configStoreSetPatientId(const String& id) {
  connectionPatientId = id;
}

void configStoreSetToken(const String& t) {
  authToken = t;
}

void configStoreSetBrightness(short b) {
  displayBrightness = b;
}

void configStoreSetProvider(uint8_t provider) {
  dataProvider = provider;
}

void configStoreSetDexcomCredentials(const String& user, const String& pass) {
  dexcomUsername = user;
  dexcomPassword = pass;
  dexcomAccountId = "";
  dexcomSessionId = "";
}

void configStoreSetDexcomRegion(uint8_t region) {
  dexcomRegion = region;
}

void configStorePersist() {
  int addr = 0;
  EEPROM.write(addr++, displayBrightness);
  writeStringToEEPROMLocal(addr, userEmail);
  addr += 1 + userEmail.length();
  writeStringToEEPROMLocal(addr, userPassword);
  addr += 1 + userPassword.length();
  writeStringToEEPROMLocal(addr, accountSha256);
  addr += 1 + accountSha256.length();
  writeStringToEEPROMLocal(addr, connectionPatientId);
  addr += 1 + connectionPatientId.length();
  EEPROM.write(addr++, dataProvider);
  writeStringToEEPROMLocal(addr, dexcomUsername);
  addr += 1 + dexcomUsername.length();
  writeStringToEEPROMLocal(addr, dexcomPassword);
  addr += 1 + dexcomPassword.length();
  EEPROM.write(addr++, dexcomRegion);
  EEPROM.commit();
}
