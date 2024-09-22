
#define __GLOBALS__ 1

#include "config.h"

#include <WiFi.h>

/*
  ToDos:

  - Ajuster la manière de configurer l'heure lorsque NTP ne fonctionne pas.
  - Interrupts de port série (si nécessaire)
  - lissage des données d'ultrason
  - site web
  - mot de passe OTA
  - niveaux d'alerte des messages PushOver
  - millis() rollover

  Lors de l'installation:
  - Mots de passes
  - digests de l'utilisateur et application PushOver
*/

RTC_DATA_ATTR int bootCount = 0;

void mainTask(void *pvParameters) {

  for (;;) {
    if constexpr (AGENDA) {
      agenda.loop();
    }

    waitFor(50);

    if constexpr (DO_OTA) {
      overTheAir.loop();
    }
  }
}

// ----- Main Functions ----------------------------------------------------------------

TaskHandle_t Task1;
TaskHandle_t Task2;

extern void webServerTask(void *pvParameters);

void setup() {

  if constexpr (INTERNET) {
    internet.setup();

    while (!internet.connect()) {
      waitFor(internet.reconnectInterval);
    }
 
     if constexpr (DO_NTP) {
      ntp.setup(internet);
      getLocalTime(&startTime);
    } else {
      getDefaultDateTime(&startTime);
    }

    if constexpr (DO_RTC) {
      realTimeClock.setup();
    }
  }

  logger.setup();

  logger.info("Demarrage");

  ++bootCount;
  logger.info("Boot number: %d", bootCount);

  // Print the wakeup reason for ESP32
  showWakeupReason();

  if constexpr (OLED_DISPLAY) {
    display.setup();
    display.show("Demarrage");
  }

  if constexpr (INTERNET) {
    internet.setup();

    while (!internet.connect()) {
      waitFor(internet.reconnectInterval);
    }

    if constexpr (DO_OTA) {
      overTheAir.setup();
    }

    if constexpr (PUSHOVER) {
      static char buff[500];

      sprintf(
          buff,
          "Demarrage - Hauteur libre sous le pont\n\nReseau: %s\nIP: %s\nMAC: %s\nRSSI: %" PRIi8,
          WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(),
          WiFi.RSSI());

      pushOver.send(buff, "0");
    }

    if constexpr (AGENDA) {
      agenda.setup();
    }
  }

  if constexpr (ULTRASONIC) {
    ultrasonic.setup();
  }

  xTaskCreatePinnedToCore(mainTask,   /* Task function. */
                          "mainTask", /* name of task. */
                          20000,      /* Stack size of task */
                          NULL,       /* parameter of the task */
                          1,          /* priority of the task */
                          &Task1,     /* Task handle to keep track of created task */
                          0);         /* pin task to core 0 */
  waitFor(500);

  if constexpr (WEBSERVER) {
    xTaskCreatePinnedToCore(webServerTask,   /* Task function. */
                            "webServerTask", /* name of task. */
                            20000,           /* Stack size of task */
                            NULL,            /* parameter of the task */
                            1,               /* priority of the task */
                            &Task2,          /* Task handle to keep track of created task */
                            1);              /* pin task to core 0 */
    waitFor(500);
  }
}

void loop() {}