#include "config.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

void PushOver::send(const char *msg, const char *priority) {

  internet.awaits();

  JsonDocument notification;

  notification["token"]     = PUSHOVER_API_TOKEN;  // required
  notification["user"]      = PUSHOVER_USER_TOKEN; // required
  notification["message"]   = msg;                // required
  notification["title"]     = PUSHOVER_TITLE;      // optional
  notification["url"]       = "";                  // optional
  notification["url_title"] = "";                  // optional
  notification["html"]      = "";                  // optional
  notification["priority"]  = priority;            // optional
  notification["sound"]     = "cosmic";            // optional
  notification["timestamp"] = "";                  // optional

  String jsonStringNotification;
  serializeJson(notification, jsonStringNotification);

  WiFiClientSecure client;

  client.setCACert(PUSHOVER_ROOT_CA);

  HTTPClient https;

  https.begin(client, pushoverApiEndpoint);

  https.addHeader("Content-Type", "application/json");

  int httpResponseCode = https.POST(jsonStringNotification);

  if (httpResponseCode > 0) {
    logger.info("HTTP response code: %d", httpResponseCode);
    String response = https.getString();
    logger.info("Response: %s", response.c_str());
  } else {
    logger.info("HTTP response code: %d", httpResponseCode);
  }

  https.end();
}
