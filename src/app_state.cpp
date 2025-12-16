// Definitions for global state
#include "app_state.h"
#include "board.h"

// Version and API
const String apiBaseUrl = "https://api.libreview.io";

// Display
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// Tasks and buttons
TaskHandle_t Task1;
TaskHandle_t Task2;
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
bool btn1Click = false;
bool btn2Click = false;

// Web/Portal
WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;

// AutoConnect components
ACText(caption1, "Do you want to perform a factory reset?");
ACSubmit(save1, "Yes, reset the device", "/delconnexecute");
AutoConnectAux aux1("/delconn", "Reset", true, {caption1, save1});
AutoConnectAux aux1Execute("/delconnexecute", "Wifi reset", false);
ACText(captionSource, "<hr><strong>Data source</strong><br><label for=\"dataProvider\">Provider</label><br><select name=\"dataProvider\" id=\"dataProvider\"><option value=\"libreview\">LibreView</option><option value=\"dexcom\">Dexcom</option></select><br><small>Select which API to use for glucose data.</small><hr><strong>LibreView</strong><br>");
ACText(captionLogin, "");
ACText(captionHr, "<hr>");
ACInput(inputEmail, "", "LibreView email", "", "");
ACInput(inputPassword, "", "LibreView password", "", "", AC_Tag_BR, AC_Input_Password);
ACInput(inputPatientIndex, "0", "Patient index", "", "0");
ACText(captionDexcom, "<hr><strong>Dexcom</strong><br>");
ACInput(inputDexcomUser, "", "Dexcom username/email/phone", "", "");
ACInput(inputDexcomPassword, "", "Dexcom password", "", "", AC_Tag_BR, AC_Input_Password);
ACText(selectDexcomRegion, "<label for=\"dexRegion\">Region</label><br><select name=\"dexRegion\" id=\"dexRegion\"><option value=\"ous\">Out of US</option><option value=\"us\">US</option><option value=\"jp\">Japan</option></select><br>");
ACSubmit(save2, "Save", "/setupexecute");
AutoConnectAux aux2("/setup", "Settings", true, {captionSource, captionLogin, inputEmail, inputPassword, inputPatientIndex, captionDexcom, inputDexcomUser, inputDexcomPassword, selectDexcomRegion, captionHr, save2});
AutoConnectAux aux3("/setupexecute", "", false);

// Data/state
String currentGlucose;
String trendArrowCode;
String patientName;
String percentChange;
JsonDocument glucoseDoc;
String currentTimestamp;
String lastTimestamp;
short missingUpdateCount = 0;

short displayBrightness = 21;
String userEmail;
String userPassword;
short selectedPatientIndex = 0;
String authToken = "";
String accountSha256;
String connectionPatientId;
uint8_t dataProvider = ProviderLibreView;
String dexcomUsername;
String dexcomPassword;
uint8_t dexcomRegion = DexcomRegionOUS;
String dexcomAccountId;
String dexcomSessionId;
bool dexcomLastDouble = false;

JsonArray glucoseGraphData;
bool isLoading = false;
short batteryPercent = 0;
bool isCharging = false;
short graphPoints = 48; // MAX 48

// PWM/display backlight
const short pwmFreq = 5000;
const short pwmResolution = 8;
const short pwmLedChannelTFT = 0;

// Captive portal flow
bool cameFromCaptivePortal = false;
unsigned long wifiScanTimestamp = 0;
const unsigned long wifiScanIntervalMs = 120 * 1000;
unsigned long lastTimedTaskAt = -1000000000;
const unsigned long timedTaskIntervalMs = 1000 * 60;

// Battery monitor
Pangodream_18650_CL BL(ADC_PIN, CONV_FACTOR, READS);
