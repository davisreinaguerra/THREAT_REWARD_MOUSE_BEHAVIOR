#include "sound.h"
#include "Arduino.h"

// constructor
sound_player::sound_player(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void sound_player::sound_on() {
  digitalWrite(_pin, HIGH);
} 

void sound_player::sound_off() {
  digitalWrite(_pin, LOW);
} 