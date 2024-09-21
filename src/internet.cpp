#include <WiFi.h>
#include <multiwifi.h>

#include "config.h"

MultiWiFi multi;

void Internet::setup() {
  SSIDS;
  // delay(2000);
}

bool Internet::connect(bool resetAttempts) {
  static uint16_t attempt = 0;

  if (resetAttempts) {
    attempt = 0;
  }

  if (attempt > 10) {
    Serial.println("Too many WiFi Connection attempts, restarting...");
    waitFor(5000);
    ESP.restart();
  }

  attempt++;

  Serial.print("Looking for a network...");
  if (multi.run() == WL_CONNECTED) {
    Serial.print("Successfully connected to network: ");
    Serial.println(WiFi.SSID());
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Failed to connect to a WiFi network");
    Serial.println("Check if access point available or SSID and Password");
    if constexpr (OLED_DISPLAY) {
      display.show("WIFI?");
    }
    return false;
  }
}

void Internet::awaits() {
  auto start         = millis();
  bool resetAttempts = true;

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    if ((millis() - start) >= reconnectInterval) {
      connect(resetAttempts);
      start         = millis();
      resetAttempts = false;
    }
  }
}
