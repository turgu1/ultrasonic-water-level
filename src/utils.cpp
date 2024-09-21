#include "config.h"

void waitFor(uint32_t waitTimeMs) {
  unsigned long start = millis();
  for (;;) {
    unsigned long now     = millis();
    unsigned long elapsed = now - start;
    if (elapsed >= waitTimeMs) {
      return;
    }
    delay(10);
  }
}

void deepSleepFor(uint32_t seconds) {
  esp_sleep_enable_timer_wakeup(uint64_t(1000000) * seconds);

  Serial.println("Going to sleep now");

  waitFor(1000);

  Serial.flush();
  esp_deep_sleep_start();
}

void showWakeupReason() {

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void getDefaultDateTime(tm *dateTime) {
  dateTime->tm_year = 2023 - 1900;
  dateTime->tm_mon  = 5;
  dateTime->tm_mday = 1;
  dateTime->tm_hour = 15;
  dateTime->tm_min  = 0;
  dateTime->tm_sec  = 0;
}
