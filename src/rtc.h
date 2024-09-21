#pragma once

#include "RTClib.h"

class RealTimeClock {
  private:
    RTC_DS3231 rtcDevice;

  public:
    bool setup();
    DateTime now();
    uint32_t getSeconds();
};

PUBLIC RealTimeClock realTimeClock;