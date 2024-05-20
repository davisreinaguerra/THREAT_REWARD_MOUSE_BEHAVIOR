#include "looming.h"
#include "Arduino.h"

// constructor
looming::looming(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void looming::loom_on() {
  digitalWrite(_pin, HIGH);
} 

void looming::loom_off() {
  digitalWrite(_pin, LOW);
} 