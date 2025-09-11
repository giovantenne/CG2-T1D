#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "app_state.h"
#include "portal.h"
#include "config.h"
#include "config_store.h"
#include "display.h"
#include "api.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "board.h"

String initialize2(AutoConnectAux& aux, PageArgument& args) {
  return String();
}

bool startCP(IPAddress ip){
  displayStartCP();
  return true;
}

void rootPage() {
  Server.sendHeader("Location", "/_ac");
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
}

void deleteAllCredentials(void) {
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();
  Serial.println("Delete all credentials");
  while (ent--) {
    credential.load((int8_t)0, &config);
    credential.del((const char*)&config.ssid[0]);
  }
  resetConfigToDefaults();
  char content[] = "Factory reset; Device is restarting...";
  Server.send(200, "text/plain", content);
  delay(3000);
  // Erase all known WiFi networks from NVS and disconnect
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.disconnect(true, true);   // turn off WiFi and erase STA config
  esp_wifi_restore();            // ensure SDK WiFi config cleared
  esp_wifi_stop();               // stop WiFi driver
  esp_wifi_deinit();             // deinit WiFi driver
  // Wipe entire NVS (removes any leftover WiFi entries)
  nvs_flash_erase();
  nvs_flash_init();
  WiFi.softAPdisconnect(true);
  delay(3000);
  ESP.restart();
}

bool detectAP(void) {
  btn2.loop();
  cameFromCaptivePortal = true;
  int16_t  ns = WiFi.scanComplete();
  if (ns == WIFI_SCAN_RUNNING) {
  } else if (ns == WIFI_SCAN_FAILED) {
    if (millis() - wifiScanTimestamp > wifiScanIntervalMs) {
      WiFi.disconnect();
      WiFi.scanNetworks(true, true, false);
    }
  } else {
    Serial.printf("scanNetworks:%d\n", ns);
    int16_t  scanResult = 0;
    while (scanResult < ns) {
      AutoConnectCredential cred;
      station_config_t  staConfig;
      if (cred.load(WiFi.SSID(scanResult++).c_str(), &staConfig) >= 0) {
        Serial.printf("AP %s ready\n", (char*)staConfig.ssid);
        WiFi.scanDelete();
        return false;
      }
    }
    Serial.println("No found known AP");
    WiFi.scanDelete();
    wifiScanTimestamp = millis();
  }
  return true;
}

void handleSaveSettings(void){
  displayShowCheckingValues();

  String newEmail = Server.arg("inputEmail");
  String newPassword = Server.arg("inputPassword");
  int newPatientIndex = Server.arg("inputPatientIndex").toInt();

  if(isValidNumber(Server.arg("inputPatientIndex"))){
    HTTPClient http;
    http.begin(apiBaseUrl + "/llu/auth/login");
    http.addHeader("product", "llu.android");
    http.addHeader("version", "4.15.0");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    String postData = "{";
    postData += "\"email\":\"" + newEmail + "\",";
    postData += "\"password\":\"" + newPassword + "\"";
    postData += "}";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      JsonDocument authDoc;
      String authPayload = http.getString();
      DeserializationError error = deserializeJson(authDoc, authPayload);
      if (error || authDoc["status"] != 0){
        handleInvalidParams();
      }else{
        String t = authDoc["data"]["authTicket"]["token"];
        String ai = authDoc["data"]["user"]["id"];
        http.end();
        http.begin(apiBaseUrl + "/llu/connections");
        http.addHeader("product", "llu.android");
        http.addHeader("version", "4.15.0");
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Accept", "application/json");
        http.addHeader("Authorization", "Bearer " + t);
        http.addHeader("account-id", sha256hex(ai));
        http.GET();
        authPayload = http.getString();

        error = deserializeJson(authDoc, authPayload);
        String pi = authDoc["data"][newPatientIndex]["patientId"];
        connectionPatientId = pi;

        String s1 = authDoc["data"]["connection"]["firstName"];
        String s2 = authDoc["data"]["connection"]["lastName"];
        patientName = s1 + " " + s2;

        configStoreSetCredentials(newEmail, newPassword);
        configStoreSetAccountSha256(sha256hex(ai));
        configStoreSetPatientId(pi);
        configStoreSetToken(t);
        configStorePersist();

        delay(3000);

        Server.sendHeader("Location", "/setup?valid=true");
      }

    } else {
      handleInvalidParams();
    } 
  } else {
    handleInvalidParams();
  }
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
}
