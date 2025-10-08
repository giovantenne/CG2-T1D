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

  bool requestSuccessful = false;
  bool hasValidIndex = isValidNumber(Server.arg("inputPatientIndex"));

  if (hasValidIndex) {
    ApiAuthResult authResult;
    if (apiLogin(newEmail, newPassword, authResult)) {
      DynamicJsonDocument connectionsDoc(8192);
      if (apiFetchConnections(authResult.token, authResult.accountSha256, connectionsDoc)) {
        JsonArray connections = connectionsDoc["data"].as<JsonArray>();
        if (!connections.isNull()) {
          size_t connectionCount = connections.size();
          if (connectionCount > 0 && newPatientIndex >= 0 && static_cast<size_t>(newPatientIndex) < connectionCount) {
            JsonVariant selected = connections[newPatientIndex];
            String patientId = selected["patientId"].as<String>();
            if (patientId.length() > 0) {
              JsonVariant connectionInfo = selected["connection"];
              String firstName = connectionInfo["firstName"].as<String>();
              String lastName = connectionInfo["lastName"].as<String>();
              patientName = firstName + " " + lastName;
              patientName.trim();
              if (patientName.length() == 0) {
                patientName = authResult.userFirstName + " " + authResult.userLastName;
                patientName.trim();
              }

              configStoreSetCredentials(newEmail, newPassword);
              configStoreSetAccountSha256(authResult.accountSha256);
              configStoreSetPatientId(patientId);
              configStoreSetToken(authResult.token);
              configStorePersist();

              requestSuccessful = true;
            }
          }
        }
      }
    }
  }

  if (requestSuccessful) {
    delay(3000);
    Server.sendHeader("Location", "/setup?valid=true");
  } else {
    handleInvalidParams();
  }
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
}
