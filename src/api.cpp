#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "app_state.h"
#include "display.h"
#include "app_store.h"
#include "api.h"
#include "board.h"
#include "mbedtls/sha256.h"

bool fetchLibreViewData() {
  Serial.println("fetchData");

  HTTPClient http;
  http.begin(apiBaseUrl + "/llu/connections/" + connectionPatientId + "/graph");
  http.addHeader("Authorization", "Bearer " + authToken);
  http.addHeader("product", "llu.android");
  http.addHeader("version", "4.15.0");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
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
