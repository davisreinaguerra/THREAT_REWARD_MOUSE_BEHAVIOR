#include "lick_sensor.h"
#include "Arduino.h"

// constructor
lick_sensor::lick_sensor(int pin) {
  pinMode(pin, INPUT);
  _pin = pin;
}

bool lick_sensor::is_licked(int n_checks) {
  for (int i = 0; i < n_checks; i++) {
    if (digitalRead(_pin) == true) {return true;}
  }
  return false;
}