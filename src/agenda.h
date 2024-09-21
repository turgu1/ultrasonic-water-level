#pragma once

#include <cinttypes>
#include <vector>

class Agenda {
  public:
    enum class EventType : int8_t {
      WATCHDOG,     // Sent once every day
      SENSING_PUSH, // Sent depending on level of urgency
      SENSING,      // Get a sensor data
      WINTER_REMOVE // Time to remove the device for winter
    };
    struct Event;
    typedef uint32_t Handler(Event const &event);
    struct Event {
        uint32_t  time;
        EventType eventType;
        int       count;
        Handler  *handler;
    };

    void setup();
    void loop();
    void priorityExec(EventType eventType);
    void show();

    const char *eventTypeStr(EventType type);

  private:
    struct CustomLess {
        bool operator()(const Event &l, const Event &r) const { return l.time < r.time; }
    } customLess;

    std::vector<Event> eventsQueue;

    bool addNewEntry(uint32_t time, EventType eventType, Handler *handler);
};

PUBLIC Agenda agenda;
