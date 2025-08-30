// 240x135
#include "utils.h"

static const String zticker_version = "vT1D.4";
static const String apiBaseUrl = "https://api.libreview.io";

String initialize2(AutoConnectAux&, PageArgument&);
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked
TaskHandle_t Task1;
TaskHandle_t Task2;
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
bool btn1Click = false;
bool btn2Click = false;
WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;
String value;
String trendArrow;
String name;
String percChange;
DynamicJsonDocument doc(20480);
String timestamp;
String oldTimestamp;
short missingUpdates;

short brightness = 21;
String username;
String password;
short patientIndex;
String token = "";
String sha256AccountId;
String patientId;

JsonArray graphData;
bool isLoading=false;
short batteryLevel;
bool charging = false;
short points = 48; // MAX 48
// Setting PWM properties, do not change this!
const short pwmFreq = 5000;
const short pwmResolution = 8;
const short pwmLedChannelTFT = 0;
bool comingFromCP;
unsigned long tmDetection;
const unsigned long scanInterval = 120 * 1000;
unsigned long timedTaskmDetection = -1000000000;
const unsigned long timedTaskInterval = 1000 * 60;
ACText(caption1, "Do you want to perform a factory reset?");
ACSubmit(save1, "Yes, reset the device", "/delconnexecute");
AutoConnectAux aux1("/delconn", "Reset", true, {caption1, save1});
AutoConnectAux aux1Execute("/delconnexecute", "Wifi reset", false);
ACText(captionLogin, "<hr><strong>Login</strong><br>");
ACText(captionHr, "<hr>");
ACInput(inputEmail, "", "Email", "", "");
ACInput(inputPassword, "", "Password", "", "");
ACInput(inputPatientIndex, "0", "Patient index", "", "0");
ACSubmit(save2, "Save", "/setupexecute");
AutoConnectAux aux2("/setup", "Settings", true, {captionLogin, inputEmail, inputPassword, inputPatientIndex, captionHr, save2});
AutoConnectAux aux3("/setupexecute", "", false);

void setup()
{
  Serial.begin(114600);
  Serial.println("Start");
  EEPROM.begin(EEPROM_SIZE);
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);
  tft.begin();
  tft.setRotation(1);
  spr.setSwapBytes(true);
  spr.setColorDepth(16);
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 120);
  Serial.println(esp_random());
  spr.createSprite(240, 135);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.fillScreen(TFT_BLACK);
  spr.pushImage(46, 0,  135, 135, logo);
  spr.setTextDatum(MR_DATUM);
  spr.drawString(zticker_version, 220, 125);
  spr.pushSprite(0,0);
  spr.unloadFont();
  espDelay(2000);
  button_init();
  Server.on("/", rootPage);
  Server.on("/delconnexecute", deleteAllCredentials);
  Server.on("/setupexecute", saveSettings);
  Config.autoReconnect = true;
  Config.reconnectInterval = 1;
  Config.ota = AC_OTA_BUILTIN;
  Config.title = "T1D-Sucks";
  Config.apid = "T1D-Sucks";
  /* Config.psk  = "12345678"; */
  Config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_UPDATE;
  Config.boundaryOffset = EEPROM_SIZE;
  Portal.config(Config);
  Portal.onDetect(startCP);
  Portal.whileCaptivePortal(detectAP);
  aux2.on(initialize2, AC_EXIT_AHEAD);
  value = "";

  if(digitalRead(BUTTON_1) == 0){
    deleteAllCredentials();
  }

  if (Portal.begin()) {
    Serial.println("Connected to WiFi");
    if (WiFi.localIP().toString() != "0.0.0.0") {
      spr.createSprite(240, 135);
      spr.loadFont(ARIAL_BOLD_24);
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
      spr.drawString("WiFi connected", 120, 40);
      spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
      spr.drawString(WiFi.localIP().toString(), 120, 80);
      spr.pushSprite(0,0);
      spr.unloadFont();
      Serial.println("Connection Opened");
      delay(3000);
      checkForOtaUpdate();
      readBatteryLevel();
      Portal.join({aux2, aux1, aux1Execute, aux3});
      readConfig();
      short convertedBrightness = brightness*255/100;
      Serial.println(convertedBrightness);
      ledcWrite(pwmLedChannelTFT, convertedBrightness);
      if(username==""){
        Serial.println("Account not connected");
        Serial.println("http://" + WiFi.localIP().toString() + "/setup");
        spr.createSprite(240, 135);
        String url = "http://" + WiFi.localIP().toString() + "/setup";
        const char* qrUrl = url.c_str();
        QRCode qrcode;
        uint8_t qrcodeData[qrcode_getBufferSize(3)];  // versione 3 = 29x29
        qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrUrl);
        int border = 2;
        int size = 4;
        int qrSizeInPixels = (qrcode.size + 2 * border) * size;
        // int offsetX = (tft.width() - qrSizeInPixels) / 2;
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
        spr.loadFont(NOTO_SANS_BOLD_15);
        // spr.loadFont(NOTO_SANS_BOLD_15);
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString("Scan this", 60, 20);
        spr.drawString("QR Code", 60, 45);
        spr.drawString("to link", 60, 70);
        spr.drawString("your", 60, 95);
        spr.drawString("account", 60, 120);
        spr.pushSprite(0,0);
      }
      if (WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
        WiFi.enableAP(false);
      }
    }
  }

  xTaskCreatePinnedToCore(
      buttonTask, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */
}

void loop()
{
  while(true){
    Portal.handleClient();
    if (WiFi.status() != WL_CONNECTED) {
      showWifiDisconnected();
      delay(2000);
    } else {
      if (btn1Click){
        btn1Click = false;
        Serial.println("btn1Click");
        brightness = (brightness + 10) % 100;
        short convertedBrightness = brightness*255/100;
        Serial.println(convertedBrightness);
        ledcWrite(pwmLedChannelTFT, convertedBrightness);
        int addr = 0;
        EEPROM.write(0, brightness);
        EEPROM.commit();
      }
      if (btn2Click){
        btn2Click = false;
        Serial.println("btn2Click");
        points = points / 2;
        if (points < 12)
          points = 48;
        Serial.println("zoom...");
        timedTaskmDetection = -1000000000;
        // missingUpdates--;
      }

      if (username != "" && (millis() - timedTaskmDetection > timedTaskInterval)) {
        readBatteryLevel();
        timedTaskmDetection = millis();
        Serial.println("Execute timed task...");
        showLoading();
        fetchData();
        showTicker();
      }
    }
    /* Serial.println(ESP.getFreeHeap()); */
  }
}

void buttonTask(void * params)
{
  while(true) {
    if (WiFi.status() == WL_CONNECTED) {
      btn1.loop();
      btn2.loop();
    }
  }
}

void showWifiDisconnected(){
  Serial.println("Reconnect wifi...");
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_24);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(80, 10, 80, 80, wifiOff);
  spr.loadFont(ARIAL_BOLD_24);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  if(comingFromCP)
    spr.drawString("Connecting to WiFi", 120, 115);
  else
    spr.drawString("WiFi disconnected", 120, 115);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

String initialize2(AutoConnectAux& aux, PageArgument& args) {
  // String tmpSymbol;
  // tmpSymbol=pairs[0];
  // tmpSymbol.toUpperCase();
  // pair1.value = tmpSymbol;
  // precision1.value = precisions[0];
  // tmpSymbol=pairs[1];
  // tmpSymbol.toUpperCase();
  // pair2.value = tmpSymbol;
  // precision2.value = precisions[1];
  // tmpSymbol=pairs[2];
  // tmpSymbol.toUpperCase();
  // pair3.value = tmpSymbol;
  // precision3.value = precisions[2];
  // inputBrightness.value=brightness;
  return String();
}

bool startCP(IPAddress ip){
  spr.createSprite(240, 135);
  spr.loadFont(NOTO_SANS_BOLD_15);
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
  return true;
}


void rootPage() {
  Server.sendHeader("Location", "/_ac");
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
}

void deleteAllCredentials(void) {
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();
  Serial.println("Delete all credentials");
  while (ent--) {
    credential.load((int8_t)0, &config);
    credential.del((const char*)&config.ssid[0]);
  }
  setDefaultValues();
  char content[] = "Factory reset; Device is restarting...";
  Server.send(200, "text/plain", content);
  delay(3000);
  WiFi.disconnect(false, true);
  delay(3000);
  ESP.restart();
}

void setDefaultValues() {
  Serial.println("Setting up default values!");
  spr.createSprite(240, 135);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Setting up", 120, 50);
  spr.drawString("default values", 120, 80);
  spr.pushSprite(0, 0);
  spr.unloadFont();

  brightness = 41;
  username="";
  password="";
  sha256AccountId="";
  patientId="";
  token="";

  int addr = 0;
  EEPROM.write(addr++, brightness);
  writeStringToEEPROM(addr, username);
  addr += 1 + username.length();
  writeStringToEEPROM(addr, password);
  addr += 1 + password.length();
  writeStringToEEPROM(addr, sha256AccountId);
  addr += 1 + sha256AccountId.length();
  writeStringToEEPROM(addr, patientId);
  addr += 1 + patientId.length();
  EEPROM.commit();

  delay(3000);
}

bool fetchData() {
  Serial.println("fetchData");

  HTTPClient http;
  http.begin(apiBaseUrl + "/llu/connections/" + patientId + "/graph");
  http.addHeader("Authorization", "Bearer " + token);
  http.addHeader("product", "llu.android");
  http.addHeader("version", "4.15.0");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.addHeader("account-id", sha256AccountId);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    // Serial.println(payload);


    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      tft.loadFont(NOTO_SANS_BOLD_15);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextDatum(ML_DATUM);
      tft.drawString("Error", 5, 26);
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.c_str());
      return false;
    }
    int v = doc["data"]["connection"]["glucoseMeasurement"]["ValueInMgPerDl"];
    value = v;
    int t = doc["data"]["connection"]["glucoseMeasurement"]["TrendArrow"];
    trendArrow = t;
    oldTimestamp=timestamp;
    String ts = doc["data"]["connection"]["glucoseMeasurement"]["Timestamp"];
    timestamp = ts;
  } else {
    tft.loadFont(NOTO_SANS_BOLD_15);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("Error" + httpResponseCode, 5, 26);
    Serial.print("HTTP error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return true;
}

void saveSettings(void){
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(20, 50, 30, 30, hourglassIcon);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Checking values...", 65, 67);
  spr.pushSprite(0, 0);
  spr.unloadFont();

  String newEmail = Server.arg("inputEmail");
  String newPassword = Server.arg("inputPassword");
  int newPatientIndex = Server.arg("inputPatientIndex").toInt();

  if(isValidNumber(Server.arg("inputPatientIndex"))){
    HTTPClient http;
    http.begin(apiBaseUrl + "/llu/auth/login");
    http.addHeader("product", "llu.android");
    http.addHeader("version", "4.15.0");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    String postData = "{";
    postData += "\"email\":\"" + newEmail + "\",";
    postData += "\"password\":\"" + newPassword + "\"";
    postData += "}";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      DynamicJsonDocument authDoc(20480);
      String authPayload = http.getString();
      DeserializationError error = deserializeJson(authDoc, authPayload);
      if (error || authDoc["status"] != 0){
        showInvalidParams();
      }else{
        String t = authDoc["data"]["authTicket"]["token"];
        String ai = authDoc["data"]["user"]["id"];
        http.end();
        http.begin(apiBaseUrl + "/llu/connections");
        http.addHeader("product", "llu.android");
        http.addHeader("version", "4.15.0");
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Accept", "application/json");
        http.addHeader("Authorization", "Bearer " + t);
        http.addHeader("account-id", sha256hex(ai));
        int httpResponseCode = http.GET();
        authPayload = http.getString();

        error = deserializeJson(authDoc, authPayload);
        String pi = authDoc["data"][newPatientIndex]["patientId"];
        patientId = pi;

        String s1 = authDoc["data"]["connection"]["firstName"];
        String s2 = authDoc["data"]["connection"]["lastName"];
        name = s1 + " " + s2;

        username=newEmail;
        password=newPassword;
        sha256AccountId=sha256hex(ai);
        patientId=pi;

        int addr = 0;
        EEPROM.write(addr++, brightness);
        writeStringToEEPROM(addr, username);
        addr += 1 + username.length();
        writeStringToEEPROM(addr, password);
        addr += 1 + password.length();
        writeStringToEEPROM(addr, sha256AccountId);
        addr += 1 + sha256AccountId.length();
        writeStringToEEPROM(addr, patientId);
        addr += 1 + patientId.length();
        EEPROM.commit();

        delay(3000);

        token=t;
        Server.sendHeader("Location", "/setup?valid=true");
      }

    } else {
      showInvalidParams();
    } 
  } else {
    showInvalidParams();
  }
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
}

boolean isValidNumber(String str){
  for(byte i=0;i<str.length();i++)
  {
    if(isDigit(str.charAt(i))) return true;
  }
  return false;
}

void showInvalidParams(){
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(20, 50, 30, 30, errorIcon);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.drawString("Invalid parameters", 65, 67);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  delay(2000);
  Server.sendHeader("Location", "/setup?valid=false&msg=invalidParameters");
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

bool isValidEmail(String email) {
  email.trim();
  int atIndex = email.indexOf('@');
  int dotAfterAt = email.indexOf('.', atIndex + 2);
  int spaceIndex = email.indexOf(' ');
  return atIndex > 0 &&
         dotAfterAt > atIndex &&
         spaceIndex == -1;
}

void readConfig(){
  Serial.println("readConfig");

  int addr = 0;
  brightness = EEPROM.read(addr++);
  username = readStringFromEEPROM(addr);
  addr += 1 + username.length();
  password = readStringFromEEPROM(addr);
  addr += 1 + password.length();
  sha256AccountId = readStringFromEEPROM(addr);
  addr += 1 + sha256AccountId.length();
  patientId = readStringFromEEPROM(addr);

  if(isValidEmail(username)){
    HTTPClient http;
    http.begin(apiBaseUrl + "/llu/auth/login");
    http.addHeader("product", "llu.android");
    http.addHeader("version", "4.15.0");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    String postData = "{";
    postData += "\"email\":\"" + username + "\",";
    postData += "\"password\":\"" + password + "\"";
    postData += "}";
    int httpResponseCode = http.POST(postData);
    Serial.println(postData);
    if (httpResponseCode > 0) {
      DynamicJsonDocument authDoc(20480);
      String authPayload = http.getString();
      // Serial.println(authPayload);
      DeserializationError error = deserializeJson(authDoc, authPayload);
      if (error || authDoc["status"] != 0){
        setDefaultValues();
      }else{
        String t = authDoc["data"]["authTicket"]["token"];
        String s1 = doc["data"]["user"]["firstName"];
        String s2 = doc["data"]["user"]["lastName"];
        name = s1 + " " + s2;
        token=t;
      }
    } else {
      setDefaultValues();
    } 
  }else{
    setDefaultValues();
  }
}

void button_init()
{
  btn1.setLongClickTime(2000);
  btn2.setLongClickTime(2000);
  btn2.setLongClickDetectedHandler([](Button2 & b) {
      Serial.println("Going to sleep...");
      int r = digitalRead(TFT_BL);
      digitalWrite(TFT_BL, !r);
      tft.writecommand(TFT_DISPOFF);
      tft.writecommand(TFT_SLPIN);
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
      esp_deep_sleep_start();
      });

  btn1.setLongClickDetectedHandler([](Button2 & b) {
      readBatteryLevel();
      timedTaskmDetection = millis();
      Serial.println("Execute forced task...");
      // missingUpdates--;
      showLoading();
      fetchData();
      showTicker();
      });

  btn1.setPressedHandler([](Button2 & b) {
      btn1Click = true;
      btn2Click = false;
      Serial.println("Button 1 pressed..");
      });

  btn2.setClickHandler([](Button2 & b) {
      Serial.println("Button 2 pressed..");
      btn1Click = false;
      btn2Click = true;
      });
}

void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void showTicker() {
  if(true || graphData.size() > 0){
    spr.createSprite(240, 135);
    spr.fillSprite(TFT_BLACK);
    spr.loadFont(TICKER_FONT);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextDatum(MR_DATUM);
    spr.drawString(value, 150, 0);

    spr.loadFont(NOTO_SANS_15);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.setTextDatum(ML_DATUM);
    spr.drawString("mg", 165, 0);
    spr.drawLine(165, 18, 185,18, TFT_DARKGREY);
    spr.drawString("dL", 165, 29);
    short xo = 50;
    short yo = 0;
    if(trendArrow == "1"){
      spr.fillTriangle(xo+155, yo, xo+175, yo, xo+165, yo+30, TFT_WHITE); // 1
    }else if(trendArrow == "2"){
      spr.fillTriangle(xo+150, yo+10, xo+165, yo, xo+175, yo+30, TFT_WHITE); // 2
    }else if(trendArrow == "3"){
      spr.fillTriangle(xo+150, yo+5, xo+180, yo+15, xo+150, yo+25, TFT_WHITE); // 3
    }else if(trendArrow == "4"){
      spr.fillTriangle(xo+150, yo+20, xo+175, yo, xo+165, yo+30, TFT_WHITE); // 4
    }else if(trendArrow == "5"){
      spr.fillTriangle(xo+155, yo+30, xo+175, yo+30, xo+165, yo, TFT_WHITE); // 5
    }
    spr.unloadFont();
    if(!charging){
      if(batteryLevel >=80){
        spr.pushImage(10, 0,  29, 14, battery_04);
      }else if(batteryLevel < 80 && batteryLevel >= 50 ){
        spr.pushImage(10, 0,  29, 14, battery_03);
      }else if(batteryLevel < 50 && batteryLevel >= 20 ){
        spr.pushImage(10, 0,  29, 14, battery_02);
      }else if(batteryLevel < 20 ){
        spr.pushImage(10, 0,  29, 14, battery_01);
      }
    }else{
      spr.pushImage(10, 0,  29, 14, battery_04);
    }
    spr.pushSprite(0, 0);
    showGraph();
  }
}

void showGraph(){
  int minValue = INT_MAX;
  int maxValue = INT_MIN;

  graphData = doc["data"]["graphData"];
  for (int i = 48 - points; i < graphData.size(); i++) {
    int value = graphData[i]["ValueInMgPerDl"];
    if (value < minValue) minValue = value;
    if (value > maxValue) maxValue = value;
  }

  short targetLow = doc["data"]["connection"]["targetLow"];
  short targetHigh = doc["data"]["connection"]["targetHigh"];
  //
  // if (minValue > 40 && minValue < 70)
  //   minValue = 0;
  // else if (minValue > 40)
  //   minValue = 40;
  if (minValue > targetLow)
    minValue = targetLow;
  if (minValue > value.toInt())
    minValue = value.toInt();
  if (maxValue < targetHigh)
    maxValue = targetHigh;
  if (maxValue < value.toInt())
    maxValue = value.toInt();
  // maxValue = ((maxValue + 19) / 20) * 20;
  // minValue = (minValue / 20) * 20;

  short th = 100 - ((targetHigh - minValue) * 100 / (maxValue-minValue));
  short tl = 100 - ((targetLow - minValue) * 100 / (maxValue-minValue));

  spr.createSprite(240, 100);
  spr.fillSprite(TFT_BLACK);

  if(timestamp!=oldTimestamp)
    missingUpdates = 0;
  else
    missingUpdates++;

  if(missingUpdates < 5)
    spr.fillRect(0, th, 240, tl, TFT_DARKGREEN);
  else
    spr.fillRect(0, th, 240, tl, TFT_DARKGREY);

  short v0;
  short v1;
  short y0;
  short y1;
  short x0;
  short x1;

  for (int i = 48 - points; i < graphData.size() - 1; i++) {
    v0 = graphData[i]["ValueInMgPerDl"];
    v1 = graphData[i+1]["ValueInMgPerDl"];
    y0 = 100 - ((v0 - minValue) * 100 / (maxValue-minValue));
    y1 = 100 - ((v1 - minValue) * 100 / (maxValue-minValue));
    x0 = ((i-(48-points))*(240/points));
    x1 = ((i+1-(48-points))*(240/points));
    spr.drawLine(x0, y0, x1, y1, TFT_WHITE);
    spr.drawLine(x0, y0-1, x1, y1-1, TFT_WHITE);
    spr.drawLine(x0, y0+1, x1, y1+1, TFT_WHITE);
  }
  int v = value.toInt();
  y0 = y1;
  y1 = 100 - ((v - minValue) * 100 / (maxValue-minValue));
  x0 = x1;
  x1 = 236;
  spr.drawLine(x0, y0, x1, y1, TFT_WHITE);
  spr.drawLine(x0, y0-1, x1, y1-1, TFT_WHITE);
  spr.drawLine(x0, y0+1, x1, y1+1, TFT_WHITE);

  if (y1==0)
    y1=3;

  if(missingUpdates==0)
    spr.fillCircle(236, y1, 4, TFT_GREEN);
  else
    spr.fillCircle(236, y1, 4, TFT_ORANGE);

  spr.drawCircle(236, y1, 5, TFT_WHITE);

  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextDatum(ML_DATUM);
  spr.drawString(String(maxValue), 10, 2);
  spr.drawLine(0, 0, 6, 0, TFT_WHITE);
  spr.drawLine(0, 1, 6, 1, TFT_WHITE);

  spr.drawString(String(minValue), 10, 96);
  spr.drawLine(0, 98, 6, 98, TFT_WHITE);
  spr.drawLine(0, 99, 6, 99, TFT_WHITE);

  short vl=points/4;
  for(int i=0; i < vl; i++){
    short x = i*(240/vl);
    spr.drawLine(x, 95 , x, 99, TFT_WHITE);
    spr.drawLine(x+1, 95 , x+1, 99, TFT_WHITE);
  }
  spr.drawLine(238, 94 , 238, 99, TFT_WHITE);
  spr.drawLine(239, 94 , 239, 99, TFT_WHITE);

  spr.pushSprite(0, 35);
}

void showLoading(){
  if(graphData.size() > 0){
    tft.loadFont(NOTO_SANS_BOLD_15);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("Loading", 5, 26);
    tft.unloadFont();
  }
}

bool detectAP(void) {
  btn2.loop();
  comingFromCP = true;
  int16_t  ns = WiFi.scanComplete();
  if (ns == WIFI_SCAN_RUNNING) {
  } else if (ns == WIFI_SCAN_FAILED) {
    if (millis() - tmDetection > scanInterval) {
      WiFi.disconnect();
      WiFi.scanNetworks(true, true, false);
    }
  } else {
    Serial.printf("scanNetworks:%d\n", ns);
    int16_t  scanResult = 0;
    while (scanResult < ns) {
      AutoConnectCredential cred;
      station_config_t  staConfig;
      if (cred.load(WiFi.SSID(scanResult++).c_str(), &staConfig) >= 0) {
        Serial.printf("AP %s ready\n", (char*)staConfig.ssid);
        WiFi.scanDelete();
        return false;
      }
    }
    Serial.println("No found known AP");
    WiFi.scanDelete();
    tmDetection = millis();
  }
  return true;
}

void readBatteryLevel(){
  Serial.println(BL.getBatteryVolts());
  if(BL.getBatteryVolts() >= MIN_USB_VOL){
    Serial.println("Charging...");
    charging = true;
  }else{
    charging = false;
    batteryLevel = BL.getBatteryChargeLevel();
  }
}
String sha256hex(const String& input) {
  byte hash[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts_ret(&ctx, 0); // 0 = SHA-256, 1 = SHA-224
  mbedtls_sha256_update_ret(&ctx, (const unsigned char *)input.c_str(), input.length());
  mbedtls_sha256_finish_ret(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  String output = "";
  for (int i = 0; i < 32; i++) {
    if (hash[i] < 16) output += "0";
    output += String(hash[i], HEX);
  }
  return output;
}
void checkForOtaUpdate(){
  Serial.println("checkForOtaUpdate");
  int dotIndex = zticker_version.indexOf('.');
  String suffix = zticker_version.substring(dotIndex + 1);
  int versionNumber = suffix.toInt();
  versionNumber++;
  String newVersion = String(versionNumber);
  HTTPClient http;
  String url = "http://cg-fw.b-cdn.net/CG2-T1D/" + newVersion + ".bin";
  http.begin(url);
  int httpCode = http.sendRequest("HEAD");
  if (httpCode == 200) {
    displayOtaUpdate();
    performOtaUpdate(url, "");
  }
  http.end();  // Chiude la connessione
}

void displayOtaUpdate(){
  Serial.println("displayOtaUpdate");
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_24);
  spr.fillSprite(TFT_BLACK);
  spr.loadFont(ARIAL_BOLD_24);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Firmware update", 120, 40);
  spr.drawString("in progress...", 120, 80);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

void performOtaUpdate(String url, String ssl_ca){
  Serial.println("performOtaUpdate");

  esp_task_wdt_init(60, true);

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.cert_pem = (char *)ssl_ca.c_str();
  config.skip_cert_common_name_check = true;

  esp_err_t ret = esp_https_ota(&config);

  if (ret == ESP_OK) {
    Serial.println("OTA SUCCESS!");
    esp_restart();
  } else {
    Serial.println("OTA ESP_FAIL");
  }
}

