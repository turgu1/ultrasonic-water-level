#if 1

#include "config.h"

#define MAX_ULTRASONIC_VALUE 450

void SerialUltrasonic::setup() {
  port = new HardwareSerial(portNbr);
  port->begin(baudRate, mode, rxdGpio, txdGpio);
  port->setHwFlowCtrlMode(UART_HW_FLOWCTRL_DISABLE);
}

int SerialUltrasonic::read() {
  uint8_t data[4];

  port->flush();
  port->write('a');
  waitFor(200);

  if (port->available() >= 4) {
    while ((port->available() > 0) && (port->read() != 0xFF))
      ;
    if (port->available() >= 3) {
      data[0] = 0xFF;

      for (int i = 1; i < 4; i++) {
        data[i] = port->read();
      }

      uint8_t sum = data[0] + data[1] + data[2];
      if (sum == data[3]) {
        lastValue = ((data[1] << 8) + data[2]);
        logger.info("Serial Ultrasonic: Last Value: %d.%03d meters.", lastValue / 1000,
                      lastValue % 1000);
        return lastValue;
      } else {
        logger.info("Serial Ultrasonic ERROR: Bad checksum.");
      }
    } else {
      logger.info("Serial Ultrasonic ERROR: Sync issue.");
    }
  } else {
    logger.info("Serial Ultrasonic ERROR: Nothing available...");
  }
}

#endif