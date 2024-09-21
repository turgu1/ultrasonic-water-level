#pragma once

class Ultrasonic {
  private:
    int lastValue = 0;

  public:
    const int maxValue = 4500;

    void setup();
    int  read();
};

PUBLIC Ultrasonic ultrasonic;