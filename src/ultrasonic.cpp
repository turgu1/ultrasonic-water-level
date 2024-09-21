#if 0

#include "config.h"

#define TRIG 17
#define ECHO 16

void Ultrasonic::setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

int Ultrasonic::read() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIG, LOW);
  long t = pulseIn(ECHO, HIGH);
  lastValue = t / 29 / 2;
  
  Serial.printf("Value: %ld\n", t);
  return lastValue;
}

#endif