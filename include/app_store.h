// Mutating operations on global app state
#pragma once

#include <Arduino.h>

void setBrightness(short b);
void setPoints(short p);
void setBattery(short level, bool isCharging);
void setMeasurement(int valueMgPerDl, int trendArrowCode, const String& ts);

