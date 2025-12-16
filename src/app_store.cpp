#include "app_state.h"

void setBrightness(short b) {
  displayBrightness = b;
}

void setPoints(short p) {
  graphPoints = p;
}

void setBattery(short level, bool isCharging) {
  batteryPercent = level;
  isCharging = isCharging;
}

void setMeasurement(int valueMgPerDl, int trendArrowCode, const String& ts) {
  String prevTs = currentTimestamp;
  lastTimestamp = currentTimestamp;
  currentGlucose = String(valueMgPerDl);
  ::trendArrowCode = String(trendArrowCode);
  currentTimestamp = ts;
  if (trendArrowCode == 1 || trendArrowCode == 7) {
    dexcomLastDouble = true;
  } else {
    dexcomLastDouble = false;
  }
}
