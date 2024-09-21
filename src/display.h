#pragma once

class Display {
  public:
    void setup();
    void show(int val, int maxVal);
    void show(const char *buff);
};

PUBLIC Display display;
