#pragma once

#include "HardwareSerial.h"

class SerialUltrasonic {
  private:
    const int portNbr  = 2;
    const int mode     = SERIAL_8N1;
    const int baudRate = 9600;
    const int rxdGpio  = 16;
    const int txdGpio  = 17;

    HardwareSerial *port;

    int lastValue;

  public:
    const int maxValue = 4500;

    void    setup();
    int     read();
};

PUBLIC SerialUltrasonic ultrasonic;