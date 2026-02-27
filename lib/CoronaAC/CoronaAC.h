/**
 * CoronaAC.h - Corona AirConditioner IR remote emulator class
 * Created by Sergio DLA, January 31, 2026.
 */
#ifndef CoronaAC_h
#define CoronaAC_h

#include <Arduino.h>
#include <ir_Corona.h>

const bool CORAC_DEBUG = false;
const uint8_t CORAC_DEFAULT_TEMP = 23;
const uint8_t CORAC_DEFAULT_FAN = kCoronaAcFanAuto;
const uint8_t CORAC_DEFAULT_MODE = kCoronaAcModeCool;
const bool CORAC_DEFAULT_SWING = false;
const bool CORAC_DEFAULT_ECONO = true;

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

    CoronaACState(uint8_t pin_, bool debug_ = CORAC_DEBUG, uint8_t temp_ = CORAC_DEFAULT_TEMP,
                  uint8_t fan_ = CORAC_DEFAULT_FAN, uint8_t mode_ = CORAC_DEFAULT_MODE,
                  bool swing_ = CORAC_DEFAULT_SWING, bool econo_ = CORAC_DEFAULT_ECONO)
      : pin(pin_), ac(pin_), debug(debug_), Temp(temp_), Fan(fan_), Mode(mode_),
        isSwinging(swing_), isEcono(econo_), isPower(true) {}
};

class CoronaAC {
    public:
        explicit CoronaAC(const uint8_t pin, const bool debug = CORAC_DEBUG,
                          const uint8_t temp = CORAC_DEFAULT_TEMP,
                          const uint8_t fanMode = CORAC_DEFAULT_FAN,
                          const uint8_t mode = CORAC_DEFAULT_MODE,
                          const bool swing = false, const bool econo = false);
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
        CoronaACState getState();
        CoronaACState setState(CoronaACState state);
        void setDebug(bool debug) { _.debug = debug; }
        void turnOn();
        void turnOff();
        uint8_t setTemp(const uint8_t desiredTemp);
        uint8_t reduceTemp();
        uint8_t increaseTemp();
        bool setSwing(const bool on);
        bool toggleSwing();
        bool setEcono(const bool on);
        bool toggleEcono();
        const char* setFanMode(const uint8_t fanMode);
        const char* nextFanMode();
        const char* setACMode(const uint8_t acMode);
        const char* nextACMode();
    private:
        CoronaACState _;
};

#endif