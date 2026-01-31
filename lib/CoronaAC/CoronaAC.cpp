#include "Arduino.h"

#include <IRremoteESP8266.h>
#include <ir_Corona.h>
#include "CoronaAC.h"

CoronaAC::CoronaAC(int pin)
{
    _.pin = pin;
};

void CoronaAC::begin()
{
    IRCoronaAc ac(_.pin);
};