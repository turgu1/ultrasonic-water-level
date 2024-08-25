#pragma once

#include <vector>

class MultiWiFi {
  public:
  void add(const char *ssid, const char *pass = NULL) {
    Network_t network;
    network.ssid = String(ssid);

    // Serial.print("MultiWifi: Added network: ");
    // Serial.print(ssid);
    // Serial.print("/");
    // Serial.println(pass);

    if (pass != nullptr) network.pass = String(pass);

    this->networks.push_back(network);
  };

  int run(unsigned long connectTimeout = 5000) {
    uint8_t status = WiFi.status();

    if (status == WL_CONNECTED) return status;

    auto scanResult = WiFi.scanNetworks();
    if (scanResult < 0) {
      Serial.println("No network returned in scan!");
      return -1;
    }

    for (auto network : this->networks) {
      for (size_t i = 0; i < scanResult; i++) {
        // SSID() is implemented with different return types so we wrap
        // it into String()
        // Serial.print(WiFi.SSID(i));
        // Serial.println("...");

        if (String(WiFi.SSID(i)) == network.ssid) {
          WiFi.begin(network.ssid.c_str(), network.pass.c_str());
          status = WiFi.status();

          auto startTime = millis();
          while ((status != WL_CONNECTED) && (millis() - startTime) <= connectTimeout) {
            delay(10);
            status = WiFi.status();
          }

          if (status == WL_CONNECTED) break;
        }
      }
      if (status == WL_CONNECTED) break;
    }

    return status;
  };

  private:
  struct Network_t {
    String ssid;
    String pass;
  };
  std::vector<Network_t> networks;
};
