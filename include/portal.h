// Portal and web handlers
#pragma once

#include <Arduino.h>
#include <AutoConnect.h>

String initialize2(AutoConnectAux& aux, PageArgument& args);
bool startCP(IPAddress ip);
void rootPage();
void deleteAllCredentials(void);
void handleSaveSettings(void);
bool detectAP(void);
