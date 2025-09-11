// High-level application state structures
#pragma once

#include <Arduino.h>

struct ConfigState {
  short displayBrightness = 0;
  String userEmail;
  String userPassword;
  String accountSha256;
  String connectionPatientId;
  short selectedPatientIndex = 0;
};

struct RuntimeState {
  String currentGlucose;
  String trendArrowCode;
  String patientName;
  String currentTimestamp;
  String lastTimestamp;
  short missingUpdateCount = 0;
  short graphPoints = 48;
  short batteryPercent = 0;
  bool isCharging = false;
  bool cameFromCaptivePortal = false;
  unsigned long wifiScanTimestamp = 0;
  unsigned long lastTimedTaskAt = 0;
};

struct AppState {
  ConfigState config;
  RuntimeState runtime;
};

// Snapshot current globals to a value-type AppState
AppState getAppState();
