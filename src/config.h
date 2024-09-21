#pragma once

#include "Arduino.h"

#define DEBUGGING 1

const constexpr bool AGENDA = true;

const constexpr bool INTERNET = true;

const constexpr bool MULTIWIFI = true && INTERNET;
const constexpr bool PUSHOVER  = true && INTERNET;
const constexpr bool WEBSERVER = true && INTERNET;
const constexpr bool DO_OTA    = true && INTERNET;
const constexpr bool DO_NTP    = true && INTERNET;

const constexpr bool DO_RTC       = true;
const constexpr bool OLED_DISPLAY = false;
const constexpr bool ULTRASONIC   = true;

#if DEBUGGING
#define SENSING_DELAY 10
#define LOW_WATER_DELAY 600
#define MEDIUM_WATER_DELAY 30
#define HIGH_WATER_DELAY 15
#define WATCHDOG_DELAY (60 * 10)
#define WINTER_DELAY (60 * 10)
#else
#define SENSING_DELAY 15
#define LOW_WATER_DELAY (60 * 60 * 24)
#define MEDIUM_WATER_DELAY (15 * 60)
#define HIGH_WATER_DELAY 30
#define WATCHDOG_DELAY (60 * 60 * 24)
#define WINTER_DELAY (60 * 60 * 24)
#endif

#include "secrets.h"

#include "globals.h"

#include "utils.h"
