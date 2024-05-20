#ifndef LED_h
#define LED_h

#include "Arduino.h"

class LED {
  public:
    LED(int pin); // Constructor
    void LED_on();
    void LED_off();
  private:
    int _pin;
};

#endif