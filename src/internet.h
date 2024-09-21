#pragma once

class Internet {

  public:
    const int reconnectInterval = 10000;

    void setup();
    void awaits();
    bool connect(bool resetAttempts = false);
};

PUBLIC Internet internet;
