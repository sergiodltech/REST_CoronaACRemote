/**
 * CoronaAC.h - Corona AirConditioner IR remote emulator class
 * Created by Sergio DLA, January 31, 2026.
 */
#ifndef CoronaAC_h
#define CoronaAC_h

#include <Arduino.h>
#include <ir_Corona.h>

const char* kFanModes[4] = {"Auto", "Low", "Mid", "High"};
const uint8_t kFanModesSize = sizeof(kFanModes) / sizeof(kFanModes[0]);
const char* kACModes[4] = {"Heat", "Dry", "Cool", "Fan"};
const uint8_t kACModesSize = sizeof(kACModes) / sizeof(kACModes[0]);

struct CoronaACState {
    bool debug;
    uint8_t pin;
    IRCoronaAc ac;
    uint8_t Temp;
    uint8_t Fan;
    uint8_t Mode;
    bool isSwinging;
    bool isEcono;
    bool isPower;

    CoronaACState(uint8_t pin_, bool debug_ = false, uint8_t temp_, uint8_t fan_, uint8_t mode_)
      : pin(pin_), ac(pin_), debug(debug_), Temp(temp_), Fan(fan_), Mode(mode_),
        isSwinging(false), isEcono(true), isPower(true) {}
};

class CoronaAC {
    public:
        explicit CoronaAC(const uint8_t pin, const bool debug = false,
                          const uint8_t temp = 23,
                          const uint8_t fanMode = kCoronaAcFanAuto,
                          const uint8_t mode = kCoronaAcModeCool);
        void begin();

        // State getters
        void printState();
        uint8_t getTemp() const { return _.Temp; }
        const char* getFanMode() const { return kFanModes[_.Fan]; }
        const char* getMode() const { return kACModes[_.Mode]; }
        bool isSwinging() const { return _.isSwinging; }
        bool isEcono() const { return _.isEcono; }
        bool isPower() const { return _.isPower; }

        // State modifiers and commands
        void setDebug(bool debug) { _.debug = debug; }
        void turnOn();
        void turnOff();
        uint8_t setTemp(const uint8_t desiredTemp);
        uint8_t reduceTemp();
        uint8_t increaseTemp();
        bool setSwing(const bool on);
        bool toggleSwing();
        const char* setFanMode(const uint8_t fanMode);
        const char* nextFanMode();
        const char* setACMode(const uint8_t acMode);
        const char* nextACMode();
    private:
        CoronaACState _;
};

#endif