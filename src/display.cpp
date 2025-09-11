#include "app_state.h"
#include "app.h"
#include "display.h"
#include <qrcode.h>
#include "board.h"
#include <limits.h>
#include "icons/wifiOff.h"
#include "icons/hourglassIcon.h"
#include "icons/errorIcon.h"
#include "icons/logo.h"
#include "icons/battery_01.h"
#include "icons/battery_02.h"
#include "icons/battery_03.h"
#include "icons/battery_04.h"
/* Use 'processing' to generate the fonts */
#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSans15.h"
#include "fonts/UbuntuBold24.h"
#include "fonts/ArialBold15.h"
#include "fonts/ArialBold24.h"
#include "fonts/Arial15.h"
#include "fonts/ArialBold46.h"

// Forward declarations for internal helpers
static void showGraphImpl(const AppState& state);

void renderWifiDisconnected(){
  Serial.println("Reconnect wifi...");
  spr.createSprite(240, 135);
  spr.loadFont(ArialBold24);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(80, 10, 80, 80, wifiOff);
  spr.loadFont(ArialBold24);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  if(cameFromCaptivePortal)
    spr.drawString("Connecting to WiFi", 120, 115);
  else
    spr.drawString("WiFi disconnected", 120, 115);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

static void renderTickerImpl(const AppState& state) {
  if(true || glucoseGraphData.size() > 0){
    spr.createSprite(240, 135);
    spr.fillSprite(TFT_BLACK);
    spr.loadFont(ArialBold46);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextDatum(MR_DATUM);
    spr.drawString(state.runtime.currentGlucose, 150, 0);

    spr.loadFont(NotoSans15);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.setTextDatum(ML_DATUM);
    spr.drawString("mg", 165, 0);
    spr.drawLine(165, 18, 185,18, TFT_DARKGREY);
    spr.drawString("dL", 165, 29);
    short xo = 50;
    short yo = 0;
    if(state.runtime.trendArrowCode == "1"){
      spr.fillTriangle(xo+155, yo, xo+175, yo, xo+165, yo+30, TFT_WHITE); // 1
    }else if(state.runtime.trendArrowCode == "2"){
      spr.fillTriangle(xo+150, yo+10, xo+165, yo, xo+175, yo+30, TFT_WHITE); // 2
    }else if(state.runtime.trendArrowCode == "3"){
      spr.fillTriangle(xo+150, yo+5, xo+180, yo+15, xo+150, yo+25, TFT_WHITE); // 3
    }else if(state.runtime.trendArrowCode == "4"){
      spr.fillTriangle(xo+150, yo+20, xo+175, yo, xo+165, yo+30, TFT_WHITE); // 4
    }else if(state.runtime.trendArrowCode == "5"){
      spr.fillTriangle(xo+155, yo+30, xo+175, yo+30, xo+165, yo, TFT_WHITE); // 5
    }
    spr.unloadFont();
    if(!state.runtime.isCharging){
      if(state.runtime.batteryPercent >=80){
        spr.pushImage(10, 0,  29, 14, battery_04);
      }else if(state.runtime.batteryPercent < 80 && state.runtime.batteryPercent >= 50 ){
        spr.pushImage(10, 0,  29, 14, battery_03);
      }else if(state.runtime.batteryPercent < 50 && state.runtime.batteryPercent >= 20 ){
        spr.pushImage(10, 0,  29, 14, battery_02);
      }else if(state.runtime.batteryPercent < 20 ){
        spr.pushImage(10, 0,  29, 14, battery_01);
      }
    }else{
      spr.pushImage(10, 0,  29, 14, battery_04);
    }
    spr.pushSprite(0, 0);
    showGraphImpl(state);
  }
}

static void showGraphImpl(const AppState& state){
  int minValue = INT_MAX;
  int maxValue = INT_MIN;

  glucoseGraphData = glucoseDoc["data"]["graphData"];
  for (int i = 48 - state.runtime.graphPoints; i < glucoseGraphData.size(); i++) {
    int value = glucoseGraphData[i]["ValueInMgPerDl"];
    if (value < minValue) minValue = value;
    if (value > maxValue) maxValue = value;
  }

  short targetLow = glucoseDoc["data"]["connection"]["targetLow"];
  short targetHigh = glucoseDoc["data"]["connection"]["targetHigh"];
  if (minValue > targetLow)
    minValue = targetLow;
  if (minValue > state.runtime.currentGlucose.toInt())
    minValue = state.runtime.currentGlucose.toInt();
  if (maxValue < targetHigh)
    maxValue = targetHigh;
  if (maxValue < state.runtime.currentGlucose.toInt())
    maxValue = state.runtime.currentGlucose.toInt();

  short th = 100 - ((targetHigh - minValue) * 100 / (maxValue-minValue));
  short tl = 100 - ((targetLow - minValue) * 100 / (maxValue-minValue));

  spr.createSprite(240, 100);
  spr.fillSprite(TFT_BLACK);

  if(state.runtime.currentTimestamp!=state.runtime.lastTimestamp)
    ;
  else
    ;

  if(state.runtime.missingUpdateCount < 5)
    spr.fillRect(0, th, 240, tl, TFT_DARKGREEN);
  else
    spr.fillRect(0, th, 240, tl, TFT_DARKGREY);

  short v0;
  short v1;
  short y0;
  short y1 = 0;
  short x0;
  short x1 = 0;

  for (int i = 48 - state.runtime.graphPoints; i < glucoseGraphData.size() - 1; i++) {
    v0 = glucoseGraphData[i]["ValueInMgPerDl"];
    v1 = glucoseGraphData[i+1]["ValueInMgPerDl"];
    y0 = 100 - ((v0 - minValue) * 100 / (maxValue-minValue));
    y1 = 100 - ((v1 - minValue) * 100 / (maxValue-minValue));
    x0 = ((i-(48-state.runtime.graphPoints))*(240/state.runtime.graphPoints));
    x1 = ((i+1-(48-state.runtime.graphPoints))*(240/state.runtime.graphPoints));
    spr.drawLine(x0, y0, x1, y1, TFT_WHITE);
    spr.drawLine(x0, y0-1, x1, y1-1, TFT_WHITE);
    spr.drawLine(x0, y0+1, x1, y1+1, TFT_WHITE);
  }
  int v = state.runtime.currentGlucose.toInt();
  y0 = y1;
  y1 = 100 - ((v - minValue) * 100 / (maxValue-minValue));
  x0 = x1;
  x1 = 236;
  spr.drawLine(x0, y0, x1, y1, TFT_WHITE);
  spr.drawLine(x0, y0-1, x1, y1-1, TFT_WHITE);
  spr.drawLine(x0, y0+1, x1, y1+1, TFT_WHITE);

  if (y1==0)
    y1=3;

  if(state.runtime.currentTimestamp!=state.runtime.lastTimestamp)
    spr.fillCircle(236, y1, 4, TFT_GREEN);
  else
    spr.fillCircle(236, y1, 4, TFT_ORANGE);

  spr.drawCircle(236, y1, 5, TFT_WHITE);

  spr.loadFont(NotoSansBold15);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextDatum(ML_DATUM);
  spr.drawString(String(maxValue), 10, 2);
  spr.drawLine(0, 0, 6, 0, TFT_WHITE);
  spr.drawLine(0, 1, 6, 1, TFT_WHITE);

  spr.drawString(String(minValue), 10, 96);
  spr.drawLine(0, 98, 6, 98, TFT_WHITE);
  spr.drawLine(0, 99, 6, 99, TFT_WHITE);

  short vl=state.runtime.graphPoints/4;
  for(int i=0; i < vl; i++){
    short x = i*(240/vl);
    spr.drawLine(x, 95 , x, 99, TFT_WHITE);
    spr.drawLine(x+1, 95 , x+1, 99, TFT_WHITE);
  }
  spr.drawLine(238, 94 , 238, 99, TFT_WHITE);
  spr.drawLine(239, 94 , 239, 99, TFT_WHITE);

  spr.pushSprite(0, 35);
}

// Public wrappers
void renderTicker() {
  auto s = getAppState();
  renderTickerImpl(s);
}

void renderGraph() {
  auto s = getAppState();
  showGraphImpl(s);
}

void renderLoadingIndicator(){
  if(glucoseGraphData.size() > 0){
    tft.loadFont(NotoSansBold15);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("Loading", 5, 26);
    tft.unloadFont();
  }
}

void displayShowCheckingValues(){
  spr.createSprite(240, 135);
  spr.loadFont(ArialBold15);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(20, 50, 30, 30, hourglassIcon);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Checking values...", 65, 67);
  spr.pushSprite(0, 0);
  spr.unloadFont();
}

void displayShowInvalidParameters(){
  spr.createSprite(240, 135);
  spr.loadFont(ArialBold15);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(20, 50, 30, 30, errorIcon);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.drawString("Invalid parameters", 65, 67);
  spr.pushSprite(0, 0);
  spr.unloadFont();
}

void displayNetworkError(){
  tft.loadFont(NotoSansBold15);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Error", 5, 26);
  tft.unloadFont();
}

void displaySettingDefaults(){
  spr.createSprite(240, 135);
  spr.loadFont(NotoSansBold15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Setting up", 120, 50);
  spr.drawString("default values", 120, 80);
  spr.pushSprite(0, 0);
  spr.unloadFont();
}

void displayStartCP(){
  spr.createSprite(240, 135);
  spr.loadFont(NotoSansBold15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Please connect to WiFi", 120, 16);
  spr.setTextDatum(MR_DATUM);
  spr.drawString("SSID:", 115, 16 + 25);
  spr.drawString("Password:", 115, 16 + 50);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.drawString("T1D-Sucks", 125, 16 + 25);
  spr.drawString("12345678", 125, 16 + 50);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("and browse to", 120, 16 + 75);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.drawString("http://172.217.28.1", 120, 16 + 100);
  spr.pushSprite(0, 0);
  spr.unloadFont();
}

void displayOtaUpdateScreen(){
  spr.createSprite(240, 135);
  spr.loadFont(ArialBold24);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Firmware update", 120, 40);
  spr.drawString("in progress...", 120, 80);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

void displaySplash(const String& version){
  spr.createSprite(240, 135);
  spr.loadFont(NotoSansBold15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.fillScreen(TFT_BLACK);
  spr.pushImage(46, 0,  135, 135, logo);
  spr.setTextDatum(MR_DATUM);
  spr.drawString(version, 220, 125);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

void displayWifiConnected(const String& ip){
  spr.createSprite(240, 135);
  spr.loadFont(ArialBold24);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.drawString("WiFi connected", 120, 40);
  spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  spr.drawString(ip, 120, 80);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

void displayQRSetup(const String& url){
  spr.createSprite(240, 135);
  const char* qrUrl = url.c_str();
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrUrl);
  int border = 2;
  int size = 4;
  int qrSizeInPixels = (qrcode.size + 2 * border) * size;
  int offsetX = 104;
  int offsetY = (tft.height() - qrSizeInPixels) / 2;
  spr.fillSprite(TFT_BLACK);
  spr.fillRect(offsetX, offsetY, qrSizeInPixels, qrSizeInPixels, TFT_BLACK);
  for (int y = 0; y < qrcode.size; y++) {
    for (int x = 0; x < qrcode.size; x++) {
      int color = qrcode_getModule(&qrcode, x, y) ? TFT_WHITE : TFT_BLACK;
      spr.fillRect(offsetX + (x + border) * size, offsetY + (y + border) * size, size, size, color);
    }
  }
  spr.createSprite(240, 135);
  spr.setTextDatum(MC_DATUM);
  spr.loadFont(NotoSansBold15);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Scan this", 60, 20);
  spr.drawString("QR Code", 60, 45);
  spr.drawString("to link", 60, 70);
  spr.drawString("your", 60, 95);
  spr.drawString("account", 60, 120);
  spr.pushSprite(0,0);
}
