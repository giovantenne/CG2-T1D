// Configuration storage and validation
#pragma once

#include <Arduino.h>

void resetConfigToDefaults();
boolean isValidNumber(String str);
void handleInvalidParams();
void writeStringToEEPROM(int addrOffset, const String &strToWrite);
String readStringFromEEPROM(int &addrOffset, size_t maxLen = 64);
void loadConfigFromEEPROM();
bool isValidEmail(String s);
