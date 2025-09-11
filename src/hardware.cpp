#include "app_state.h"
#include "app_store.h"
#include "hardware.h"
#include "board.h"

void readBatteryLevel(){
  float volts = BL.getBatteryVolts();
  Serial.println(volts);
  if(volts >= MIN_USB_VOL){
    Serial.println("Charging...");
    setBattery(batteryPercent, true);
  }else{
    setBattery(BL.getBatteryChargeLevel(), false);
  }
}

void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}
