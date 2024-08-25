
#include <Arduino.h>
#include <cmath>

#include "config.h"

// ----- Ultrasonic Sensor -------------------------------------------------------------------

#if OPTTION_ULTRASOUND

#define ULTRASOUND_TRIG 12
#define ULTRASOUND_ECHO 13

#define MAX_ULTRASOUND_VALUE 405

void ultrasoundSetup() {
  pinMode(ULTRASOUND_TRIG, OUTPUT);
  pinMode(ULTRASOUND_ECHO, INPUT);
}

int readUltrasound() {
  digitalWrite(ULTRASOUND_TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(ULTRASOUND_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASOUND_TRIG, LOW);
  long t = pulseIn(ULTRASOUND_ECHO, HIGH);
  return t / 29 / 2;
}

#endif

// ----- Display -----------------------------------------------------------------------------

#if OPTTION_OLED_DISPLAY

#include <Wire.h>
// #include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_1_SW_I2C
u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);

void displaySetup() { u8g2.begin(); }

void displayValue(int val) {
  char buff[20];

  sprintf(buff, "%d", val);
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 10, 128, 20);
    u8g2.drawBox(10, 15, min(110, 110 * val / MAX_ULTRASOUND_VALUE), 10);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int w = u8g2.getStrWidth(buff);
    u8g2.drawStr(64 - (w / 2), 60, buff);
  } while (u8g2.nextPage());
}

void displayStr(const char *buff) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int w = u8g2.getStrWidth(buff);
    u8g2.drawStr(64 - (w / 2), 40, buff);
  } while (u8g2.nextPage());
}

#else

void displaySetup() {}

void displayValue(int val) {}

void displayStr(const char *buff) {}

#endif

// ----- WiFi --------------------------------------------------------------------------------

#if OPTTION_INTERNET

uint16_t reconnect_interval = 10000;

#if OPTTION_MULTIWIFI

#include <WiFi.h>
#include <multiwifi.h>

MultiWiFi multi;

void WiFiSetup() { SSIDS; delay(2000); }

bool WiFiConnect(bool resetAttempts = false) {
  static uint16_t attempt = 0;

  if (resetAttempts) {
    attempt = 0;
  }

  if (attempt > 10) {
    Serial.println("Too many WiFi Connection attempts, restarting...");
    delay(5000);
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
      displayStr("WIFI?");
    }
    return false;
  }
}

void Awaits() {
  uint32_t ts            = millis();
  bool     resetAttempts = true;

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    if (millis() > (ts + reconnect_interval)) {
      WiFiConnect(resetAttempts);
      ts            = millis();
      resetAttempts = false;
    }
  }
}

#else

#include <WiFi.h>

#if OPTTION_WOKWI
const char *ssid     = "Wokwi-GUEST";
const char *password = "";
const int   channel  = 6;
#else
const char *ssid     = "Genevrier Secteur D";
const char *password = "b52dcgene";
const char  channel  = 6;
#endif

void WiFiSetup() {}

bool WiFiConnect(bool resetAttempts = false) {
  static uint16_t attempt = 0;

  if (resetAttempts) {
    attempt = 0;
  }

  if (attempt > 10) {
    Serial.println("Too many WiFi Connection attempts, restarting...");
    delay(5000);
    ESP.restart();
  }

  attempt++;

  if (attempt == 1) {
    Serial.print("Connecting to ");
    if constexpr (WOKWI) {
      WiFi.begin(ssid, password, channel);
    } else {
      WiFi.begin(ssid, password);
    }
    Serial.println(ssid);
  }

  uint8_t i = 0;
  while ((WiFi.status() != WL_CONNECTED) && (i++ < 50)) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("");
  if (i > 50) {
    Serial.print("Connection: TIMEOUT on attempt: ");
    Serial.println(attempt);
    if constexpr (OLED_DISPLAY) {
      displayStr("WIFI?");
    }
    if (attempt % 2 == 0)
      Serial.println("Check if access point available or SSID and Password\r\n");
    return false;
  }

  Serial.println("Connection: ESTABLISHED");
  Serial.print("Got IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

void Awaits() {

  uint32_t ts            = millis();
  bool     resetAttempts = true;

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    if (millis() > (ts + reconnect_interval)) {
      WiFiConnect(resetAttempts);
      ts = millis();
    }
    resetAttempts = false;
  }
}

#endif
#endif

// ----- PushOver -------------------------------------------------------------------------

#if OPTTION_PUSHOVER && OPTTION_INTERNET

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char *apiToken  = PUSHOVER_API_TOKEN;
const char *userToken = PUSHOVER_USER_TOKEN;

// Pushover API endpoint
const char *pushoverApiEndpoint = "https://api.pushover.net/1/messages.json";

// Pushover root certificate (valid from 11/10/2006 to 15/01/2038)
const char *PUSHOVER_ROOT_CA = "-----BEGIN CERTIFICATE-----\n"
                               "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
                               "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
                               "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
                               "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
                               "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
                               "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
                               "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
                               "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
                               "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
                               "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
                               "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
                               "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
                               "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
                               "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
                               "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
                               "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
                               "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
                               "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
                               "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
                               "MrY=\n"
                               "-----END CERTIFICATE-----\n";

void pushOverSend(const char *msg, const char *priority = "0") {

  Awaits();

  StaticJsonDocument<512> notification;

  notification["token"]     = apiToken;       // required
  notification["user"]      = userToken;      // required
  notification["message"]   = msg;            // required
  notification["title"]     = PUSHOVER_TITLE; // optional
  notification["url"]       = "";             // optional
  notification["url_title"] = "";             // optional
  notification["html"]      = "";             // optional
  notification["priority"]  = priority;       // optional
  notification["sound"]     = "cosmic";       // optional
  notification["timestamp"] = "";             // optional

  String jsonStringNotification;
  serializeJson(notification, jsonStringNotification);

  WiFiClientSecure client;

  client.setCACert(PUSHOVER_ROOT_CA);

  HTTPClient https;

  https.begin(client, pushoverApiEndpoint);

  https.addHeader("Content-Type", "application/json");

  int httpResponseCode = https.POST(jsonStringNotification);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP response code: %d\n", httpResponseCode);
    String response = https.getString();
    Serial.println("Response:");
    Serial.println(response);
  } else {
    Serial.printf("HTTP response code: %d\n", httpResponseCode);
  }

  https.end();
}

#endif

void mainTask(void *pvParameters) {

  static int val = 0;

  for (;;) {
    if constexpr (ULTRASOUND) {
      val = readUltrasound();
    } else {
      val++;
    }

    printf("Distance: %d\n", val);

    if constexpr (PUSHOVER) {
      static char buff[200];
      sprintf(buff, "Hauteur libre sous le pont: %d", val);
      pushOverSend(buff, "0");
    }

    if constexpr (OLED_DISPLAY) {
      displayValue(val);
    }

    delay(30 * 1000); // Every 30 seconds
  }
}

// ----- Main Functions ----------------------------------------------------------------

TaskHandle_t Task1;
TaskHandle_t Task2;

extern void webServerTask(void *pvParameters);

void setup() {

  Serial.begin(115200);
  Serial.println("Startup");

  if constexpr (OLED_DISPLAY) {
    displaySetup();
    displayStr("Startup");
  }

  if constexpr (INTERNET) {
    WiFiSetup();

    while (!WiFiConnect()) {
      delay(reconnect_interval);
    }

    if constexpr (PUSHOVER) {
      static char buff[200];
      sprintf(buff, "Startup, IP: %s, RSSI: %" PRIi8, WiFi.localIP().toString().c_str(),
              WiFi.RSSI());
      pushOverSend(buff, "0");
    }
  }

  if constexpr (ULTRASOUND) {
    ultrasoundSetup();
  }

  xTaskCreatePinnedToCore(mainTask,   /* Task function. */
                          "mainTask", /* name of task. */
                          20000,      /* Stack size of task */
                          NULL,       /* parameter of the task */
                          1,          /* priority of the task */
                          &Task1,     /* Task handle to keep track of created task */
                          0);         /* pin task to core 0 */
  delay(500);

  if constexpr (WEBSERVER) {
    xTaskCreatePinnedToCore(webServerTask,   /* Task function. */
                            "webServerTask", /* name of task. */
                            20000,           /* Stack size of task */
                            NULL,            /* parameter of the task */
                            1,               /* priority of the task */
                            &Task2,          /* Task handle to keep track of created task */
                            1);              /* pin task to core 0 */
    delay(500);
  }
}

void loop() {}