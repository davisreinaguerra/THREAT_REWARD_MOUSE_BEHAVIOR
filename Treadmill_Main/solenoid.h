#ifndef solenoid_h
#define solenoid_h

#include "Arduino.h"

class solenoid {
  public:
    solenoid(int pin); // Constructor
    void valve_on();
    void valve_off();
    void pulse_valve(int reward_duration);
  private:
    int _pin;
};

#endif