#include "config.h"

void NTP::printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("NTP ERROR: Failed to obtain time");
    return;
  }
  Serial.print("NTP Date and Time: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void NTP::setup(Internet &internet) {
  internet.awaits();

  configTime(0, 0, ntpServer);
  printLocalTime();
}
