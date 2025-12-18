#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
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
const char* kDexAppIdUS = "d89443d2-327c-4a6f-89e5-496bbb0317db";
const char* kDexAppIdOUS = "d89443d2-327c-4a6f-89e5-496bbb0317db";
const char* kDexAppIdJP = "d8665ade-9673-4e27-9ff6-92db4ce13d13";
WiFiClientSecure dexClient;

String maskSecret(const String& secret) {
  if (secret.length() <= 2) return "**";
  return secret.substring(0, 2) + "***";
}

void logHttp(const char* label, const String& url, const String& extra = "") {
  Serial.print("[HTTP] ");
  Serial.print(label);
  Serial.print(" -> ");
  Serial.print(url);
  if (extra.length() > 0) {
    Serial.print(" ");
    Serial.print(extra);
  }
  Serial.println();
}

String dexcomBaseUrl(uint8_t region) {
  switch (region) {
    case DexcomRegionUS:
      return "https://share2.dexcom.com/ShareWebServices/Services";
    case DexcomRegionJP:
      return "https://share.dexcom.jp/ShareWebServices/Services";
    case DexcomRegionOUS:
    default:
      return "https://shareous1.dexcom.com/ShareWebServices/Services";
  }
}

const char* dexcomAppId(uint8_t region) {
  switch (region) {
    case DexcomRegionUS:
      return kDexAppIdUS;
    case DexcomRegionJP:
      return kDexAppIdJP;
    case DexcomRegionOUS:
    default:
      return kDexAppIdOUS;
  }
}

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

bool beginDexcomRequest(HTTPClient& http, WiFiClientSecure& client, const String& url) {
  client.stop();
  client.setInsecure(); // skip CA to save memory
  client.setTimeout(12000);
  if (!http.begin(client, url)) {
    return false;
  }
  http.setReuse(true);
  http.useHTTP10(true); // reduce RAM usage
  http.addHeader("Content-Type", kJsonMimeType);
  return true;
}

int64_t parseDexDateMs(const String& dexDate) {
  int start = dexDate.indexOf('(');
  int end = dexDate.indexOf(')');
  if (start == -1 || end == -1 || end <= start + 1) {
    return 0;
  }
  String inner = dexDate.substring(start + 1, end);
  int plusIdx = inner.indexOf('+');
  if (plusIdx > 0) {
    inner = inner.substring(0, plusIdx);
  }
  return (int64_t)strtoull(inner.c_str(), nullptr, 10);
}

int dexcomTrendToArrow(const String& trend) {
  // Map Dexcom trends to 7-symbol arrow codes:
  // 1=doubleDown 2=down 3=diagDown 4=flat 5=diagUp 6=up 7=doubleUp
  if (trend == "DoubleDown") return 1;
  if (trend == "SingleDown") return 2;
  if (trend == "FortyFiveDown") return 3;
  if (trend == "Flat") return 4;
  if (trend == "FortyFiveUp") return 5;
  if (trend == "SingleUp") return 6;
  if (trend == "DoubleUp") return 7;
  if (trend == "None" || trend == "NonComputable" || trend == "RateOutOfRange") return 4;
  return 4;
}

int libreTrendToArrow(int raw) {
  // Libre uses 5 arrows: 1=down, 2=diag down, 3=flat, 4=diag up, 5=up
  // Raw TrendArrow: 1=DoubleUp,2=SingleUp,3=FortyFiveUp,4=Flat,5=FortyFiveDown,6=SingleDown,7=DoubleDown
  switch (raw) {
    case 7: return 1; // double down -> down
    case 6: return 1; // single down -> down
    case 5: return 2; // forty five down -> diag down
    case 4: return 3; // flat
    case 3: return 4; // forty five up -> diag up
    case 2: return 5; // single up -> up
    case 1: return 5; // double up -> up
    default: return 3;
  }
}
}  // namespace

#ifdef UNIT_TEST
int testDexcomTrendToArrow(const String& trend) { return dexcomTrendToArrow(trend); }
int testLibreTrendToArrow(int raw) { return libreTrendToArrow(raw); }
int64_t testParseDexDateMs(const String& dexDate) { return parseDexDateMs(dexDate); }
#endif

bool apiLogin(const String& email, const String& password, ApiAuthResult& result) {
  HTTPClient http;
  if (!beginJsonRequest(http, apiBaseUrl + "/llu/auth/login", kConnectionsVersion)) {
    return false;
  }

  logHttp("LibreView login", apiBaseUrl + "/llu/auth/login", "user=" + email + " pw=" + maskSecret(password));
  String postData = "{";
  postData += "\"email\":\"" + email + "\",";
  postData += "\"password\":\"" + password + "\"";
  postData += "}";

  int httpResponseCode = http.POST(postData);
  Serial.printf("[HTTP] LibreView login status: %d\n", httpResponseCode);
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

  logHttp("LibreView connections", apiBaseUrl + "/llu/connections", "acc=" + accountSha256);
  http.addHeader("Authorization", "Bearer " + token);
  http.addHeader("account-id", accountSha256);

  int httpResponseCode = http.GET();
  Serial.printf("[HTTP] LibreView connections status: %d\n", httpResponseCode);
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
  logHttp("LibreView graph", apiBaseUrl + "/llu/connections/" + connectionPatientId + "/graph");
  http.addHeader("Authorization", "Bearer " + authToken);
  http.addHeader("account-id", accountSha256);

  int httpResponseCode = http.GET();
  Serial.printf("[HTTP] LibreView graph status: %d\n", httpResponseCode);

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
    Serial.println("[LibreView] graph parsed, measurement updated");
  } else {
    displayNetworkError();
    Serial.print("HTTP error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return true;
}

bool fetchDexcomData() {
  dexcomNewData = false;
  if (dexcomUsername.length() == 0 || dexcomPassword.length() == 0) {
    Serial.println("Dexcom credentials missing");
    return false;
  }

  String baseUrl = dexcomBaseUrl(dexcomRegion);
  const char* appId = dexcomAppId(dexcomRegion);

  // Step 1: accountId (reuse if available)
  if (dexcomAccountId.length() == 0) {
    HTTPClient http;
    if (!beginDexcomRequest(http, dexClient, baseUrl + "/General/AuthenticatePublisherAccount")) {
      return false;
    }
    logHttp("Dexcom AuthenticatePublisherAccount", baseUrl, "user=" + dexcomUsername + " pw=" + maskSecret(dexcomPassword));
    String authBody = "{";
    authBody += "\"accountName\":\"" + dexcomUsername + "\",";
    authBody += "\"password\":\"" + dexcomPassword + "\",";
    authBody += "\"applicationId\":\"" + String(appId) + "\"";
    authBody += "}";
    int code = http.POST(authBody);
    Serial.printf("[HTTP] Dexcom accountId status: %d\n", code);
    if (code <= 0) {
      http.end();
      return false;
    }
    String accountId = http.getString();
    http.end();
    accountId.replace("\"", "");
    if (accountId.length() == 0) {
      return false;
    }
    dexcomAccountId = accountId;
  } else {
    Serial.println("[Dexcom] Reusing accountId");
  }

  // Step 2: sessionId
  if (dexcomSessionId.length() == 0) {
    HTTPClient http2;
    if (!beginDexcomRequest(http2, dexClient, baseUrl + "/General/LoginPublisherAccountById")) {
      return false;
    }
    logHttp("Dexcom LoginPublisherAccountById", baseUrl);
    String sessionBody = "{";
    sessionBody += "\"accountId\":\"" + dexcomAccountId + "\",";
    sessionBody += "\"password\":\"" + dexcomPassword + "\",";
    sessionBody += "\"applicationId\":\"" + String(appId) + "\"";
    sessionBody += "}";
    int code = http2.POST(sessionBody);
    Serial.printf("[HTTP] Dexcom sessionId status: %d\n", code);
    if (code <= 0) {
      http2.end();
      dexcomSessionId = "";
      return false;
    }
    String sessionId = http2.getString();
    http2.end();
    sessionId.replace("\"", "");
    if (sessionId.length() == 0) {
      dexcomSessionId = "";
      return false;
    }
    dexcomSessionId = sessionId;
  } else {
    Serial.println("[Dexcom] Reusing sessionId");
  }

  // Step 3: readings
  HTTPClient http3;
  if (!beginDexcomRequest(http3, dexClient, baseUrl + "/Publisher/ReadPublisherLatestGlucoseValues")) {
    return false;
  }
  logHttp("Dexcom ReadPublisherLatestGlucoseValues", baseUrl);
  String readingsBody = "{";
  readingsBody += "\"sessionId\":\"" + dexcomSessionId + "\",";
  readingsBody += "\"minutes\":720,";
  readingsBody += "\"maxCount\":144";
  readingsBody += "}";
  int code = http3.POST(readingsBody);
  Serial.printf("[HTTP] Dexcom readings status: %d\n", code);
  if (code <= 0) {
    http3.end();
    dexcomSessionId = "";
    return false;
  }
  String payload = http3.getString();
  http3.end();

  DynamicJsonDocument doc(16384);
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.println("Dexcom JSON parse error");
    return false;
  }
  JsonArray arr = doc.as<JsonArray>();
  if (arr.isNull() || arr.size() == 0) {
    return false;
  }
  Serial.printf("[Dexcom] readings=%d\n", arr.size());

  // Build glucoseDoc in Libre-like shape for rendering
  glucoseDoc.clear();
  JsonObject data = glucoseDoc.createNestedObject("data");
  JsonObject connection = data.createNestedObject("connection");
  connection["targetLow"] = 70;
  connection["targetHigh"] = 180;
  JsonArray graph = data.createNestedArray("graphData");

  // Downsample Dexcom 5-min points into ~48 slots (like Libre graph)
  const int kMaxBuckets = 48;
  int total = arr.size();
  int bucketSize = (total + kMaxBuckets - 1) / kMaxBuckets; // ceil
  if (bucketSize < 1) bucketSize = 1;

  int aggValues[kMaxBuckets];
  int aggTrends[kMaxBuckets];
  long aggTs[kMaxBuckets];
  int aggCount = 0;

  int groupCount = 0;
  long groupSum = 0;
  int groupTrend = 3;
  long groupTs = 0;

  // Iterate newest -> oldest to keep latest bucket aligned to right edge
  for (int i = 0; i < total && aggCount < kMaxBuckets; i++) {
    JsonObject src = arr[i];
    groupSum += src["Value"].as<int>();
    groupTrend = dexcomTrendToArrow(src["Trend"].as<String>());
    groupTs = parseDexDateMs(src["WT"].as<String>());
    groupCount++;

    bool flush = (groupCount >= bucketSize) || (i == total - 1);
    if (flush) {
      aggValues[aggCount] = (int)(groupSum / groupCount);
      aggTrends[aggCount] = groupTrend;
      aggTs[aggCount] = groupTs;
      aggCount++;
      groupCount = 0;
      groupSum = 0;
    }
  }

  // Push oldest -> newest into graph
  for (int j = aggCount - 1; j >= 0; j--) {
    JsonObject dst = graph.createNestedObject();
    dst["ValueInMgPerDl"] = aggValues[j];
    dst["TrendArrow"] = aggTrends[j];
    dst["Timestamp"] = aggTs[j];
  }

  // Latest reading is index 0 in Dexcom response
  JsonObject latest = arr[0];
  int arrowCode = dexcomTrendToArrow(latest["Trend"].as<String>());
  int64_t tsMsLatest = parseDexDateMs(latest["WT"].as<String>());
  char tsBuf[24];
  snprintf(tsBuf, sizeof(tsBuf), "%lld", (long long)tsMsLatest);
  String tsStr(tsBuf);
  if (dexcomLastTs.length() > 0 && tsStr != dexcomLastTs) {
    dexcomNewData = true;
  }
  dexcomLastTs = tsStr;
  Serial.printf("[Dexcom] latest value=%d trend=%s arrow=%d ts=%s\n",
                latest["Value"].as<int>(),
                latest["Trend"].as<const char*>(),
                arrowCode,
                tsBuf);
  setMeasurement(latest["Value"].as<int>(), arrowCode, String(tsBuf));
  return true;
}

bool fetchCurrentData() {
  Serial.printf("[DATA] provider=%d libreEmail=%s dexUser=%s region=%d\n",
                dataProvider,
                userEmail.c_str(),
                dexcomUsername.c_str(),
                dexcomRegion);
  if (dataProvider == ProviderDexcom) {
    return fetchDexcomData();
  }
  return fetchLibreViewData();
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
