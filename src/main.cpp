// 240x135
#include <Arduino.h>
// Firmware version (moved here per request)
extern const String firmwareVersion = "vT1D.7";
#ifndef UNIT_TEST
#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <qrcode.h>
#include "app_state.h"
#include "app_store.h"
#include "board.h"
#include "display.h"
#include "portal.h"
#include "config.h"
#include "hardware.h"
#include "buttons.h"
#include "api.h"
#include "ota.h"

void setup()
{
  Serial.begin(114600);
  Serial.println("Start");
  EEPROM.begin(EEPROM_SIZE);
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);
  tft.begin();
  tft.setRotation(1);
  spr.setSwapBytes(true);
  spr.setColorDepth(16);
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 120);
  Serial.println(esp_random());
  displaySplash(firmwareVersion);
  espDelay(2000);

  button_init();
  Server.on("/", rootPage);
  Server.on("/delconnexecute", deleteAllCredentials);
  Server.on("/setupexecute", handleSaveSettings);
  Config.autoReconnect = true;
  Config.reconnectInterval = 1;
  Config.ota = AC_OTA_BUILTIN;
  Config.title = "T1D-Sucks";
  Config.apid = "T1D-Sucks";
  /* Config.psk  = "12345678"; */
  Config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_UPDATE;
  Config.boundaryOffset = EEPROM_SIZE;
  Portal.config(Config);
  Portal.onDetect(startCP);
  Portal.whileCaptivePortal(detectAP);
  aux2.on(initialize2, AC_EXIT_AHEAD);
  currentGlucose = "";

  if(digitalRead(BUTTON_1) == 0){
    deleteAllCredentials();
  }

  if (Portal.begin()) {
    Serial.println("Connected to WiFi");
    if (WiFi.localIP().toString() != "0.0.0.0") {
      displayWifiConnected(WiFi.localIP().toString());
      Serial.println("Connection Opened");
      delay(3000);
      checkForOtaUpdate();
      readBatteryLevel();
      Portal.join({aux2, aux1, aux1Execute, aux3});
      loadConfigFromEEPROM();
      short convertedBrightness = displayBrightness*255/100;
      Serial.println(convertedBrightness);
      ledcWrite(pwmLedChannelTFT, convertedBrightness);
      if(userEmail==""){
        Serial.println("Account not connected");
        Serial.println("http://" + WiFi.localIP().toString() + "/setup");
        String url = "http://" + WiFi.localIP().toString() + "/setup";
        displayQRSetup(url);
      }
      if (WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
        WiFi.enableAP(false);
      }
    }
  }

  xTaskCreatePinnedToCore(
      buttonTask,
      "Task1",
      10000,
      NULL,
      0,
      &Task1,
      0);
}

void loop()
{
  while(true){
    Portal.handleClient();
    if (WiFi.status() != WL_CONNECTED) {
      renderWifiDisconnected();
      delay(2000);
    } else {
      if (btn1Click){
        btn1Click = false;
        Serial.println("btn1Click");
        short newBrightness = (displayBrightness + 10) % 100;
        setBrightness(newBrightness);
        short convertedBrightness = newBrightness*255/100;
        Serial.println(convertedBrightness);
        ledcWrite(pwmLedChannelTFT, convertedBrightness);
        EEPROM.write(0, newBrightness);
        EEPROM.commit();
      }
      if (btn2Click){
        btn2Click = false;
        Serial.println("btn2Click");
        short newPoints = graphPoints / 2;
        if (newPoints < 12)
          newPoints = 48;
        setPoints(newPoints);
        Serial.println("zoom...");
        lastTimedTaskAt = -1000000000;
        missingUpdateCount--;
      }

      if (userEmail != "" && (millis() - lastTimedTaskAt > timedTaskIntervalMs)) {
        readBatteryLevel();
        lastTimedTaskAt = millis();
        Serial.println("Execute timed task...");
        renderLoadingIndicator();
        fetchLibreViewData();
        renderTicker();
      }
    }
  }
}

#endif // UNIT_TEST

// moved function definitions live in src/*.cpp
