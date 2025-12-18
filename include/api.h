// API client
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

struct ApiAuthResult {
  String token;
  String accountId;
  String accountSha256;
  String userFirstName;
  String userLastName;
};

bool apiLogin(const String& email, const String& password, ApiAuthResult& result);
bool apiFetchConnections(const String& token, const String& accountSha256, DynamicJsonDocument& outDoc);
bool fetchLibreViewData();
bool fetchDexcomData();
bool fetchCurrentData();
String sha256hex(const String& input);

#ifdef UNIT_TEST
// Test helpers (mapping/time parsing)
int testDexcomTrendToArrow(const String& trend);
int testLibreTrendToArrow(int raw);
int64_t testParseDexDateMs(const String& dexDate);
#endif
