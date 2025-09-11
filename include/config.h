// Configuration storage and validation
#pragma once

#include <Arduino.h>

void resetConfigToDefaults();
boolean isValidNumber(String str);
void handleInvalidParams();
void writeStringToEEPROM(int addrOffset, const String &strToWrite);
String readStringFromEEPROM(int addrOffset);
void loadConfigFromEEPROM();
bool isValidEmail(String s);
