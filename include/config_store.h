// ConfigStore: centralized mutation + persistence for config
#pragma once

#include <Arduino.h>

void configStoreSetCredentials(const String& user, const String& pass);
void configStoreSetAccountSha256(const String& sha256Id);
void configStoreSetPatientId(const String& id);
void configStoreSetToken(const String& t);
void configStoreSetBrightness(short b);
void configStorePersist();

