// Centralized global state declarations
#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>
using fs::File;
#include "SPIFFS.h"
#include <AutoConnect.h>
#include <TFT_eSPI.h>
#include <Button2.h>
#include <ArduinoJson.h>
#include <Pangodream_18650_CL.h>

// Version and API
extern const String firmwareVersion;
extern const String apiBaseUrl;

// Display
extern TFT_eSPI tft;
extern TFT_eSprite spr;

// Tasks and buttons
extern TaskHandle_t Task1;
extern TaskHandle_t Task2;
extern Button2 btn1;
extern Button2 btn2;
extern bool btn1Click;
extern bool btn2Click;

// Web/Portal
extern WebServer Server;
extern AutoConnect Portal;
extern AutoConnectConfig Config;

// AutoConnect pages/components
extern AutoConnectText caption1;
extern AutoConnectSubmit save1;
extern AutoConnectAux aux1;
extern AutoConnectAux aux1Execute;
extern AutoConnectText captionSource;
extern AutoConnectText captionLogin;
extern AutoConnectText captionHr;
extern AutoConnectInput inputEmail;
extern AutoConnectInput inputPassword;
extern AutoConnectInput inputPatientIndex;
extern AutoConnectText captionDexcom;
extern AutoConnectInput inputDexcomUser;
extern AutoConnectInput inputDexcomPassword;
extern AutoConnectText selectDexcomRegion;
extern AutoConnectSubmit save2;
extern AutoConnectAux aux2;
extern AutoConnectAux aux3;

// Data/state
extern String currentGlucose;
extern String trendArrowCode;
extern String patientName;
extern String percentChange;
extern JsonDocument glucoseDoc;
extern String currentTimestamp;
extern String lastTimestamp;
extern short missingUpdateCount;

extern short displayBrightness;
extern String userEmail;
extern String userPassword;
extern short selectedPatientIndex;
extern String authToken;
extern String accountSha256;
extern String connectionPatientId;
extern uint8_t dataProvider;
extern String dexcomUsername;
extern String dexcomPassword;
extern uint8_t dexcomRegion;
extern String dexcomAccountId;
extern String dexcomSessionId;
extern bool dexcomLastDouble;

// Providers
enum DataProvider : uint8_t {
  ProviderLibreView = 0,
  ProviderDexcom = 1
};

// Dexcom regions
enum DexcomRegion : uint8_t {
  DexcomRegionUS = 0,
  DexcomRegionOUS = 1,
  DexcomRegionJP = 2
};

extern JsonArray glucoseGraphData;
extern bool isLoading;
extern short batteryPercent;
extern bool isCharging;
extern short graphPoints;

// PWM/display backlight
extern const short pwmFreq;
extern const short pwmResolution;
extern const short pwmLedChannelTFT;

// Captive portal flow
extern bool cameFromCaptivePortal;
extern unsigned long wifiScanTimestamp;
extern const unsigned long wifiScanIntervalMs;
extern unsigned long lastTimedTaskAt;
extern const unsigned long timedTaskIntervalMs;

// Battery monitor
extern Pangodream_18650_CL BL;
