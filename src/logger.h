#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstdarg>

class Logger {
  private:
    WiFiUDP udp;
    IPAddress broadcastAddress;

    void do_log(const char * severity, const char * fmt, va_list args);

  public:
    void setup();
    void info(const char *fmt, ...);
    void warning(const char * fmt, ...);
    void error(const char * fmt, ...);
    void flush();
};

PUBLIC Logger logger;