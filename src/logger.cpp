
#include "config.h"

#include "logger.h"

#include <cstdio>

void Logger::setup() {

  if constexpr (LOGGER_UDP) {
    broadcastAddress = IPAddress(10,1,0,6);
    Serial.print("IP Address: ");
    Serial.println(broadcastAddress);
  } 
  
  if constexpr (LOGGER_SERIAL) {
    Serial.begin(115200);
  }
}

void Logger::do_log(const char *severity, const char * fmt, va_list args) {

  static char buff[500];

  buff[0] = 0;

  const char *dateTime = "No DateTime";

  if constexpr (DO_RTC) {
    dateTime = DateTime(SECONDS_FROM_1970_TO_2000 + realTimeClock.getSeconds()).timestamp().c_str();
  }

  snprintf(buff, 499, "%s - %s - ", dateTime, severity);

  vsnprintf(&buff[strlen(buff)], 499, fmt, args);
  strncat(buff, "\r\n", 499);

  if constexpr (LOGGER_UDP) {
    if (udp.beginPacket(broadcastAddress, LOGGER_PORT) == 1) {
      if (udp.write((uint8_t *) buff, strlen(buff)) == strlen(buff)) {
        if (udp.endPacket() == 0) {
          Serial.printf("Unable to send packet: %d: %s!", errno, strerror(errno));
        }
      } else {
        Serial.println("Unable to write packet!");
      }
    } else {
      Serial.println("Internal WiFiUDP error!");
    }
  } 
  
  if constexpr (LOGGER_SERIAL) {
    Serial.print(buff);
  }
}

void Logger::info(const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  do_log("INFO", fmt, args);
}

void Logger::warning(const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  do_log("WARNING", fmt, args);
}

void Logger::error(const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  do_log("ERROR", fmt, args);
}

void Logger::flush() {

  if constexpr (LOGGER_SERIAL) {
    Serial.flush();
  }
}
