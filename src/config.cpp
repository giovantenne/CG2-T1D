#include <EEPROM.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "app_state.h"
#include "display.h"
#include "config.h"
#include "config_store.h"
#include "board.h"

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int &addrOffset, size_t maxLen)
{
  int newStrLen = EEPROM.read(addrOffset);
  addrOffset++;
  if (newStrLen < 0 || newStrLen > (int)maxLen) {
    return "";
  }
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + i);
  }
  addrOffset += newStrLen;
  data[newStrLen] = '\0';
  return String(data);
}

bool isValidEmail(String email) {
  email.trim();
  int atIndex = email.indexOf('@');
  int dotAfterAt = email.indexOf('.', atIndex + 2);
  int spaceIndex = email.indexOf(' ');
  return atIndex > 0 &&
         dotAfterAt > atIndex &&
         spaceIndex == -1;
}

boolean isValidNumber(String str){
  for(byte i=0;i<str.length();i++)
  {
    if(isDigit(str.charAt(i))) return true;
  }
  return false;
}

void resetConfigToDefaults() {
  Serial.println("Setting up default values!");
  displaySettingDefaults();

  displayBrightness = 41;
  userEmail="";
  userPassword="";
  accountSha256="";
  connectionPatientId="";
  authToken="";
  dataProvider = ProviderLibreView;
  dexcomUsername = "";
  dexcomPassword = "";
  dexcomRegion = DexcomRegionOUS;
  dexcomFreshFound = false;
  dexcomNewData = false;
  dexcomSkipCounter = 0;
  dexcomLastTs = "";
  dexcomAccountId = "";
  dexcomSessionId = "";

  int addr = 0;
  EEPROM.write(addr++, displayBrightness);
  writeStringToEEPROM(addr, userEmail);
  addr += 1 + userEmail.length();
  writeStringToEEPROM(addr, userPassword);
  addr += 1 + userPassword.length();
  writeStringToEEPROM(addr, accountSha256);
  addr += 1 + accountSha256.length();
  writeStringToEEPROM(addr, connectionPatientId);
  addr += 1 + connectionPatientId.length();
  EEPROM.write(addr++, dataProvider);
  writeStringToEEPROM(addr, dexcomUsername);
  addr += 1 + dexcomUsername.length();
  writeStringToEEPROM(addr, dexcomPassword);
  addr += 1 + dexcomPassword.length();
  EEPROM.write(addr++, dexcomRegion);
  EEPROM.commit();

  delay(3000);
}

void loadConfigFromEEPROM(){
  Serial.println("readConfig");

  int addr = 0;
  short b = EEPROM.read(addr++);
  String u = readStringFromEEPROM(addr);
  String p = readStringFromEEPROM(addr);
  String sha = readStringFromEEPROM(addr);
  String pid = readStringFromEEPROM(addr);
  uint8_t provider = EEPROM.read(addr++);
  if (provider > ProviderDexcom) {
    provider = ProviderLibreView;
  }
  String dexUser = readStringFromEEPROM(addr);
  String dexPass = readStringFromEEPROM(addr);
  uint8_t region = EEPROM.read(addr++);
  if (region > DexcomRegionJP) {
    region = DexcomRegionOUS;
  }

  configStoreSetBrightness(b);
  configStoreSetCredentials(u, p);
  configStoreSetAccountSha256(sha);
  configStoreSetPatientId(pid);
  configStoreSetProvider(provider);
  configStoreSetDexcomCredentials(dexUser, dexPass);
  configStoreSetDexcomRegion(region);
  dexcomAccountId = "";
  dexcomSessionId = "";

  if(dataProvider == ProviderLibreView && isValidEmail(userEmail)){
    HTTPClient http;
    http.begin(apiBaseUrl + "/llu/auth/login");
    http.addHeader("product", "llu.android");
    http.addHeader("version", "4.15.0");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    String postData = "{";
    postData += "\"email\":\"" + userEmail + "\",";
    postData += "\"password\":\"" + userPassword + "\"";
    postData += "}";
    int httpResponseCode = http.POST(postData);
    Serial.println(postData);
    if (httpResponseCode > 0) {
      JsonDocument authDoc;
      String authPayload = http.getString();
      DeserializationError error = deserializeJson(authDoc, authPayload);
      int status = authDoc["status"] | -1;
      if (error || status != 0){
        resetConfigToDefaults();
      }else{
        String t = authDoc["data"]["authTicket"]["token"];
        String s1 = authDoc["data"]["user"]["firstName"];
        String s2 = authDoc["data"]["user"]["lastName"];
        patientName = s1 + " " + s2;
        authToken=t;
      }
    } else {
      resetConfigToDefaults();
    } 
  } else if (dataProvider == ProviderDexcom) {
    // Dexcom path: no validation here; credentials are checked during Dexcom fetch.
  } else {
    resetConfigToDefaults();
  }
}

void handleInvalidParams(){
  displayShowInvalidParameters();
  delay(2000);
  Server.sendHeader("Location", "/setup?valid=false&msg=invalidParameters");
}
