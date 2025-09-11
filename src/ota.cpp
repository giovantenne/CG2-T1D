#include <Arduino.h>
#include <HTTPClient.h>
#include "esp_https_ota.h"
#include "esp_task_wdt.h"
#include "app_state.h"
#include "display.h"
#include "ota.h"

void checkForOtaUpdate(){
  Serial.println("checkForOtaUpdate");
  int dotIndex = firmwareVersion.indexOf('.');
  String suffix = firmwareVersion.substring(dotIndex + 1);
  int versionNumber = suffix.toInt();
  versionNumber++;
  String newVersion = String(versionNumber);
  HTTPClient http;
  String url = "http://cg-fw.b-cdn.net/CG2-T1D/" + newVersion + ".bin";
  http.begin(url);
  int httpCode = http.sendRequest("HEAD");
  if (httpCode == 200) {
    displayOtaUpdate();
    performOtaUpdate(url, "");
  }
  http.end();
}

void displayOtaUpdate(){
  Serial.println("displayOtaUpdate");
  displayOtaUpdateScreen();
}

void performOtaUpdate(String url, String ssl_ca){
  Serial.println("performOtaUpdate");

  esp_task_wdt_init(60, true);

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.cert_pem = (char *)ssl_ca.c_str();
  config.skip_cert_common_name_check = true;

  esp_err_t ret = esp_https_ota(&config);

  if (ret == ESP_OK) {
    Serial.println("OTA SUCCESS!");
    esp_restart();
  } else {
    Serial.println("OTA ESP_FAIL");
  }
}
