// Display public interface
#pragma once

#include <Arduino.h>

void renderWifiDisconnected();
void renderTicker();
void renderGraph();
void renderLoadingIndicator();

void displayShowCheckingValues();
void displayShowInvalidParameters();
void displayNetworkError();
void displaySettingDefaults();
void displayStartCP();
void displayOtaUpdateScreen();
void displaySplash(const String& version);
void displayWifiConnected(const String& ip);
void displayQRSetup(const String& url);
