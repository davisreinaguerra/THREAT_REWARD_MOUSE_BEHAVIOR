#ifndef sound_h
#define sound_h

#include "Arduino.h"

class sound_player {
  public:
    sound_player(int pin); // Constructor
    void sound_on();
    void sound_off();
  private:
    int _pin;
};

#endif