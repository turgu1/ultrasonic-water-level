
#include <cmath>
#include "WiFi.h"
#include "config.h"

// ----- sensingHandler() -----

/*
  ============= Bridge

  High priority delay

  ------------- High water level 1,500 meters from bridge

  Medium priority delay

  ------------- Medium water level 2,000 meters from bridge

  Low priority delay

  ------------- Low water level 4,500 meters from bridge
*/

#if DEBUGGING
struct {
    int low;
    int high;
    int pushElapse;
} tresholds[]{{200 + 25, 4500, LOW_WATER_DELAY},
              {150 + 25, 200 - 25, MEDIUM_WATER_DELAY},
              {30, 150 - 25, HIGH_WATER_DELAY}};
#else
struct {
    int low;
    int high;
    int pushElapse;
} tresholds[]{{2000 + 50, 4500, LOW_WATER_DELAY},
              {1500 + 50, 2000 - 50, MEDIUM_WATER_DELAY},
              {300, 1500 - 50, HIGH_WATER_DELAY}};
#endif

uint32_t sensingHandler(Agenda::Event const &event) {

  if constexpr (ULTRASONIC) {
    int value = ultrasonic.read();

    if (abs(value - currentSensorValue) < 1200) {

      currentSensorValue = value;

      for (int i = 0; i < 3; i++) {
        if ((currentSensorValue >= tresholds[i].low) && (currentSensorValue <= tresholds[i].high)) {
          int newElapse = tresholds[i].pushElapse;
          if (newElapse != currentSensorPushElapse) {
            currentSensorPushElapse = newElapse;
            agenda.priorityExec(Agenda::EventType::SENSING_PUSH);
          }
        }
      }
    }
  } else {
    currentSensorValue++;
  }

  if constexpr (OLED_DISPLAY) {
    display.show(currentSensorValue, ultrasonic.maxValue);
  }

  printf("Current Sensor value: %d,%03d meters.\n", currentSensorValue / 1000,
         currentSensorValue % 1000);

  return SENSING_DELAY;
}

// ----- sensingPushHandler() -----

uint32_t sensingPushHandler(Agenda::Event const &event) {
  if constexpr (PUSHOVER) {
    static char buff[200];

    sprintf(buff, "Hauteur libre sous le pont: %d,%03d mètres.\n", currentSensorValue / 1000,
            currentSensorValue % 1000);

    pushOver.send(buff, "0");
  }
  return currentSensorPushElapse;
}

// ----- watchdogHandler() -----

uint32_t watchdogHandler(Agenda::Event const &event) {

  if constexpr (PUSHOVER) {
    static char buff[500];

    sprintf(buff, "Info journalière\n\nRéseau: %s\nIP: %s\nMAC: %s\nRSSI: %" PRIi8,
            WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(),
            WiFi.RSSI());

    pushOver.send(buff, "0");
  }

  return WATCHDOG_DELAY;
}

// ----- winterHandler() -----

uint32_t winterHandler(Agenda::Event const &event) {
  static char buff[100];

  sprintf(buff, "C'est maintenant le temps de débrancher pour l'hiver le capteur de Hauteur Libre "
                "sous le pont!");

  pushOver.send(buff, "0");

  return WINTER_DELAY;
}
