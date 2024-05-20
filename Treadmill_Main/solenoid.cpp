#include "solenoid.h"
#include "Arduino.h"

// constructor
solenoid::solenoid(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void solenoid::valve_on() {
  digitalWrite(_pin, HIGH);
}

void solenoid::valve_off() {
  digitalWrite(_pin, LOW);
}

void solenoid::pulse_valve(int duration) {
  digitalWrite(_pin, HIGH);
  delay(duration);
  digitalWrite(_pin, LOW);
}
