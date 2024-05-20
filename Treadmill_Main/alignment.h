#ifndef alignment_h
#define alignment_h

#include "Arduino.h"

class alignment {
  public:
    alignment(int pin); // constructor
    void align_onset();
    void align_offset(); 
    void align_shunt(bool what_to_shunt);
  private:
    int _pin;
};

#endif