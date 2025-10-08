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
String sha256hex(const String& input);
