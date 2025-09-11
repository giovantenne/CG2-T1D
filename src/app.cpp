#include "app.h"
#include "app_state.h"

AppState getAppState() {
  AppState s;
  // Config
  s.config.displayBrightness = displayBrightness;
  s.config.userEmail = userEmail;
  s.config.userPassword = userPassword;
  s.config.accountSha256 = accountSha256;
  s.config.connectionPatientId = connectionPatientId;
  s.config.selectedPatientIndex = selectedPatientIndex;
  // Runtime
  s.runtime.currentGlucose = currentGlucose;
  s.runtime.trendArrowCode = trendArrowCode;
  s.runtime.patientName = patientName;
  s.runtime.currentTimestamp = currentTimestamp;
  s.runtime.lastTimestamp = lastTimestamp;
  s.runtime.missingUpdateCount = missingUpdateCount;
  s.runtime.graphPoints = graphPoints;
  s.runtime.batteryPercent = batteryPercent;
  s.runtime.isCharging = isCharging;
  s.runtime.cameFromCaptivePortal = cameFromCaptivePortal;
  s.runtime.wifiScanTimestamp = wifiScanTimestamp;
  s.runtime.lastTimedTaskAt = lastTimedTaskAt;
  return s;
}
