#include "config.h"

bool Agenda::addNewEntry(uint32_t time, EventType eventType, Handler *handler) {
  if (time > realTimeClock.getSeconds()) {
    Event e = {.time = time, .eventType = eventType, .count = 0, .handler = handler};
    eventsQueue.push_back(e);
    return true;
  }
  return false;
}

void Agenda::setup() {
  if constexpr (ULTRASONIC) {
    // Retrieve sensor data every 15 seconds
    DateTime time = realTimeClock.now() + TimeSpan(15);
    addNewEntry(time.secondstime(), EventType::SENSING, sensingHandler);
  }

  if constexpr (PUSHOVER) {
    // Push message through PushOver using the currentSensorPushElapse value
    DateTime time = realTimeClock.now() + TimeSpan(currentSensorPushElapse);
    addNewEntry(time.secondstime(), EventType::SENSING_PUSH, sensingPushHandler);

    // Watchdog every day at 8:00:00 DST (-4) in the morning, starting tomorrow
    time = realTimeClock.now();
    DateTime newTime =
        DateTime(time.year(), time.month(), time.day(), 8 + 4, 0, 0) + TimeSpan(WATCHDOG_DELAY);
    addNewEntry(newTime.secondstime(), EventType::WATCHDOG, watchdogHandler);

    // Push message when it's time to disconnect the sensor, every November 1st
    // and the following days at 8:15:00 DST (-4).
    newTime = DateTime(time.year(), 11, 1, 8 + 4, 15, 0);
    addNewEntry(newTime.secondstime(), EventType::WINTER_REMOVE, winterHandler);
  }

  std::sort(eventsQueue.begin(), eventsQueue.end(), customLess);

  show();
}

void Agenda::loop() {
  for (; !eventsQueue.empty();) {

    uint32_t now = realTimeClock.getSeconds();

    Event &e = eventsQueue[0];
    if (e.time <= now) {

      printf("Calling handler for EventType %s...\n", eventTypeStr(e.eventType));

      uint32_t newElapse = (*e.handler)(e);

      e.time = now + newElapse;
      e.count += 1;

      std::sort(eventsQueue.begin(), eventsQueue.end(), customLess);
    } else {
      break;
    }
  }
}

void Agenda::priorityExec(EventType eventType) {
  for (auto &e : eventsQueue) {
    if (e.eventType == eventType) {
      printf("Calling in priority handler for EventType %s...\n", eventTypeStr(e.eventType));

      uint32_t newElapse = (*e.handler)(e);

      uint32_t now = realTimeClock.getSeconds();

      e.time = now + newElapse;
      e.count += 1;

      std::sort(eventsQueue.begin(), eventsQueue.end(), customLess);
    }
  }
}
void Agenda::show() {

  printf("----- Agenda entries ----- Now: %s -----\n",
         DateTime(SECONDS_FROM_1970_TO_2000 + realTimeClock.getSeconds()).timestamp().c_str());

  int i = 1;
  for (auto &e : eventsQueue) {
    DateTime dateTime = DateTime(SECONDS_FROM_1970_TO_2000 + e.time);
    printf("%d : Time:%s Type:%s Count:%d\n", i, dateTime.timestamp().c_str(),
           eventTypeStr(e.eventType), e.count);
    i++;
  }

  printf("---- end -----\n");
}

const char *Agenda::eventTypeStr(EventType eventType) {

  switch (eventType) {
  case EventType::WATCHDOG:
    return "WATCHDOG";

  case EventType::SENSING:
    return "SENSING";

  case EventType::SENSING_PUSH:
    return "SENSING_PUSH";

  case EventType::WINTER_REMOVE:
    return "WINTER_REMOVE";
  }

  return "NONE!";
}