#include "config.h"
#include "rtc.h"

bool RealTimeClock::setup() {

  if (rtcDevice.begin()) {
    if constexpr (DO_NTP) {
      tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        rtcDevice.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
      } else {
        return false;
      }
    }
    return true;
  } else {
    Serial.println("ERROR: RTC Device not detected!");
  }

  return false;
}

DateTime RealTimeClock::now() {
  return rtcDevice.now();
}

uint32_t RealTimeClock::getSeconds() {
  return rtcDevice.now().secondstime();
}

