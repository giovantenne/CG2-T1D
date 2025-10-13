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

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
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
  EEPROM.commit();

  delay(3000);
}

void loadConfigFromEEPROM(){
  Serial.println("readConfig");

  int addr = 0;
  short b = EEPROM.read(addr++);
  String u = readStringFromEEPROM(addr);
  addr += 1 + u.length();
  String p = readStringFromEEPROM(addr);
  addr += 1 + p.length();
  String sha = readStringFromEEPROM(addr);
  addr += 1 + sha.length();
  String pid = readStringFromEEPROM(addr);

  configStoreSetBrightness(b);
  configStoreSetCredentials(u, p);
  configStoreSetAccountSha256(sha);
  configStoreSetPatientId(pid);

  if(isValidEmail(userEmail)){
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
  }else{
    resetConfigToDefaults();
  }
}

void handleInvalidParams(){
  displayShowInvalidParameters();
  delay(2000);
  Server.sendHeader("Location", "/setup?valid=false&msg=invalidParameters");
}
