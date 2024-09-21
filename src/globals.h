#pragma once

#if __GLOBALS__
#define PUBLIC
#else
#define PUBLIC extern
#endif

#include <time.h>

#include "internet.h"
#include "ntp.h"
#include "ota.h"
#include "pushover.h"
#include "rtc.h"
// #include "ultrasonic.h"
#include "agenda.h"
#include "display.h"
#include "handlers.h"
#include "serialultrasonic.h"

// This is the time when the device was booted
PUBLIC tm startTime;

// The elapse time between transmission of the computed hight.
// Will be adjusted against the current hight to augment
// the number of transmissions when the hight is becoming narrow.
// In seconds.
PUBLIC uint32_t currentSensorPushElapse
#if __GLOBALS__
    = LOW_WATER_DELAY
#endif
    ;

// This is the current hight retrieved from the sensor. 
// Read every SENSING_DELAY seconds. In millimeters
PUBLIC int currentSensorValue
#if __GLOBALS__
    = 120
#endif
    ;