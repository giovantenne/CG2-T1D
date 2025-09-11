#include "app_state.h"
#include "buttons.h"
#include "hardware.h"
#include "display.h"
#include "api.h"

void buttonTask(void * params)
{
  while(true) {
    if (WiFi.status() == WL_CONNECTED) {
      btn1.loop();
      btn2.loop();
    }
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
      lastTimedTaskAt = millis();
      Serial.println("Execute forced task...");
      missingUpdateCount--;
      renderLoadingIndicator();
      fetchLibreViewData();
      renderTicker();
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
