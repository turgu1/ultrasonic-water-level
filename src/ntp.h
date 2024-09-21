#pragma once

class Internet;

class NTP {

  private:
    const char ntpServer[16]{"ca.pool.ntp.org"};

  public:
    void printLocalTime();
    void setup(Internet &internet);
};

PUBLIC NTP ntp;
