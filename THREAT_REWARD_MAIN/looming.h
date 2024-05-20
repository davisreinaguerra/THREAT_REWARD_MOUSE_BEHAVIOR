#ifndef looming_h
#define looming_h

#include "Arduino.h"

class looming {
  public:
    looming(int pin); // Constructor
    void loom_on();
    void loom_off();
  private:
    int _pin;
};

#endif