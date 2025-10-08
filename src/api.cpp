#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "app_state.h"
#include "display.h"
#include "app_store.h"
#include "api.h"
#include "board.h"
#include "mbedtls/sha256.h"

namespace {
const char* kProductHeader = "llu.android";
const char* kJsonMimeType = "application/json";
const char* kConnectionsVersion = "4.16.0";

void applyCommonHeaders(HTTPClient& http, const char* version) {
  http.addHeader("product", kProductHeader);
  http.addHeader("version", version);
  http.addHeader("Content-Type", kJsonMimeType);
  http.addHeader("Accept", kJsonMimeType);
}

bool beginJsonRequest(HTTPClient& http, const String& url, const char* version) {
  if (!http.begin(url)) {
    return false;
  }
  applyCommonHeaders(http, version);
  return true;
}
}  // namespace

bool apiLogin(const String& email, const String& password, ApiAuthResult& result) {
  HTTPClient http;
  if (!beginJsonRequest(http, apiBaseUrl + "/llu/auth/login", kConnectionsVersion)) {
    return false;
  }

  String postData = "{";
  postData += "\"email\":\"" + email + "\",";
  postData += "\"password\":\"" + password + "\"";
  postData += "}";

  int httpResponseCode = http.POST(postData);
  if (httpResponseCode <= 0) {
    http.end();
    return false;
  }

  String authPayload = http.getString();
  http.end();

  DynamicJsonDocument authDoc(4096);
  DeserializationError error = deserializeJson(authDoc, authPayload);
  if (error) {
    return false;
  }

  if (authDoc["status"].as<int>() != 0) {
    return false;
  }

  result.token = authDoc["data"]["authTicket"]["token"].as<String>();
  result.accountId = authDoc["data"]["user"]["id"].as<String>();
  result.accountSha256 = sha256hex(result.accountId);
  result.userFirstName = authDoc["data"]["user"]["firstName"].as<String>();
  result.userLastName = authDoc["data"]["user"]["lastName"].as<String>();

  return result.token.length() > 0 && result.accountId.length() > 0;
}

bool apiFetchConnections(const String& token, const String& accountSha256, DynamicJsonDocument& outDoc) {
  HTTPClient http;
  if (!beginJsonRequest(http, apiBaseUrl + "/llu/connections", kConnectionsVersion)) {
    return false;
  }

  http.addHeader("Authorization", "Bearer " + token);
  http.addHeader("account-id", accountSha256);

  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  DeserializationError error = deserializeJson(outDoc, payload);
  if (error) {
    outDoc.clear();
    return false;
  }

  if (outDoc["status"].as<int>() != 0) {
    outDoc.clear();
    return false;
  }

  return true;
}

bool fetchLibreViewData() {
  Serial.println("fetchData");

  HTTPClient http;
  if (!beginJsonRequest(http, apiBaseUrl + "/llu/connections/" + connectionPatientId + "/graph", kConnectionsVersion)) {
    return false;
  }
  http.addHeader("Authorization", "Bearer " + authToken);
  http.addHeader("account-id", accountSha256);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    DeserializationError error = deserializeJson(glucoseDoc, payload);
    if (error) {
      displayNetworkError();
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }
    int v = glucoseDoc["data"]["connection"]["glucoseMeasurement"]["ValueInMgPerDl"];
    int t = glucoseDoc["data"]["connection"]["glucoseMeasurement"]["TrendArrow"];
    String ts = glucoseDoc["data"]["connection"]["glucoseMeasurement"]["Timestamp"];
    setMeasurement(v, t, ts);
  } else {
    displayNetworkError();
    Serial.print("HTTP error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return true;
}

String sha256hex(const String& input) {
  byte hash[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts_ret(&ctx, 0); // 0 = SHA-256, 1 = SHA-224
  mbedtls_sha256_update_ret(&ctx, (const unsigned char *)input.c_str(), input.length());
  mbedtls_sha256_finish_ret(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  String output = "";
  for (int i = 0; i < 32; i++) {
    if (hash[i] < 16) output += "0";
    output += String(hash[i], HEX);
  }
  return output;
}
