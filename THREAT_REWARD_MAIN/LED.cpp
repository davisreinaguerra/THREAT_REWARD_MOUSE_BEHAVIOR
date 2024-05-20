#include "LED.h"
#include "Arduino.h"

// constructor
LED::LED(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void LED::LED_on() {
  digitalWrite(_pin, HIGH);
}

void LED::LED_off() {
  digitalWrite(_pin, LOW);
}