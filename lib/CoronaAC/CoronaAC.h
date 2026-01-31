/**
 * CoronaAC.h - Corona AirConditioner IR remote emulator class
 * Created by Sergio DLA, January 31, 2026.
 */
#ifndef CoronaAC_h
#define CoronaAC_h

#include "Arduino.h"

const char* kFanModes[4] = {"Auto", "Low", "Mid", "High"};
const uint8_t kFanModesSize = sizeof(kFanModes) / sizeof(kFanModes[0]);
const char* kACModes[4] = {"Heat", "Dry", "Cool", "Fan"};
const uint8_t kACModesSize = sizeof(kACModes) / sizeof(kACModes[0]);

struct CoronaACState {
    uint8_t pin;
    uint8_t Temp;
    uint8_t Fan;
    uint8_t Mode;
    bool isSwinging;
    bool isEcono;
    bool isPower;
};

class CoronaAC
{
    public:
        explicit CoronaAC(const int pin);
        void begin();
    private:
        CoronaACState _;
};

#endif