#ifndef lick_sensor_h
#define lick_sensor_h

#include "Arduino.h"

class lick_sensor {
  public:
    lick_sensor(int pin); // Constructor
    bool is_licked(int);
  private:
    int _pin;
};

#endif
