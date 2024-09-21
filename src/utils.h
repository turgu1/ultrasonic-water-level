#pragma once

#include <time.h>

extern void waitFor(uint32_t waitTimeMs);
extern void deepSleepFor(uint32_t seconds);
extern void showWakeupReason();
extern void getDefaultDateTime(tm *dateTime);