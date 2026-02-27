#include <Arduino.h>

#include "CoronaAC.h"

CoronaAC::CoronaAC(uint8_t pin, bool debug, uint8_t temp, uint8_t fanMode, uint8_t mode, bool swing, bool econo)
                   : _(pin, debug, temp, fanMode, mode, swing, econo) {}

void CoronaAC::begin() {
    _.ac.begin();
    _.ac.setTemp(_.Temp);
    _.ac.setMode(_.Mode);
    _.ac.setFan(_.Fan);
    _.ac.setSwingVToggle(_.isSwinging);
    _.ac.setEcono(_.isEcono);
    if (_.debug) {
        Serial.println("CoronaAC init state.");
        printState();
    }
};

void CoronaAC::printState() {
    // Display current settings.
    Serial.println("CoronaAC state:");
    Serial.printf("  %s\n", _.ac.toString().c_str());

    // Display the encoded IR sequence.
    unsigned char* ir_code = _.ac.getRaw();
    Serial.print("IR Code: 0x");
    for (uint8_t i = 0; i < kCoronaAcStateLength; i++)
        Serial.printf("%02X", ir_code[i]);
    Serial.println();
}

CoronaACState CoronaAC::setState(CoronaACState state) {
    _.debug = state.debug;
    _.Temp = state.Temp;
    _.Fan = state.Fan;
    _.Mode = state.Mode;
    _.isSwinging = state.isSwinging;
    _.isEcono = state.isEcono;
    _.ac.send();
    return _;
}

CoronaACState CoronaAC::getState() {
    return _;
}

void CoronaAC::turnOn() {
    if (_.debug) {
        Serial.println("CMD: Turn CoronaAC on");
    }
    _.ac.on();
    _.ac.setPower(true);
    _.ac.send();
}

void CoronaAC::turnOff() {
    if (_.debug) {
        Serial.println("CMD: Turn CoronaAC off");
    }
    _.ac.off();
    _.ac.setPower(true);
    _.ac.send();
}

uint8_t CoronaAC::setTemp(const uint8_t desiredTemp ) {
    if (desiredTemp > kCoronaAcMaxTemp) {
        _.Temp = kCoronaAcMaxTemp;
    } else if (desiredTemp < kCoronaAcMinTemp) {
        _.Temp = kCoronaAcMinTemp;
    } else {
        _.Temp = desiredTemp;
    }

    if (_.debug) {
        Serial.print("CMD: setting temp to ");
        Serial.printf("%dC\n", _.Temp);
    }
    _.ac.setTemp(_.Temp);
    _.ac.send();
    return _.Temp;
}

uint8_t CoronaAC::reduceTemp() {
    return setTemp(_.Temp - 1);
}

uint8_t CoronaAC::increaseTemp() {
    return setTemp(_.Temp + 1);
}

bool CoronaAC::setSwing(const bool on) {
    _.isSwinging = on;
    if (_.debug) {
        Serial.print("CMD: setting swing to ");
        Serial.printf("%s\n", _.isSwinging ? "on": "off");
    }
    _.ac.setSwingVToggle(_.isSwinging);
    _.ac.send();
    return _.isSwinging;
}

bool CoronaAC::toggleSwing() {
    return setSwing(!_.isSwinging);
}

bool CoronaAC::setEcono(const bool on) {
    _.isEcono = on;
    if (_.debug) {
        Serial.print("CMD: setting economic mode to ");
        Serial.printf("%s\n", _.isEcono ? "on": "off");
    }
    _.ac.setEcono(_.isEcono);
    _.ac.send();
    return _.isEcono;
}

bool CoronaAC::toggleSwing() {
    return setEcono(!_.isEcono);
}

const char* CoronaAC::setFanMode(const uint8_t fanMode) {
    if (fanMode > kFanModesSize - 1) {
        _.Fan = 0;
    } else if (fanMode < 0) {
        _.Fan = kFanModesSize - 1;
    } else {
        _.Fan = fanMode;
    }

    if (_.debug) {
        Serial.print("CMD: setting Fan mode to ");
        Serial.printf("%s\n", kFanModes[_.Fan]);
    }
    _.ac.setFan(_.Fan);
    _.ac.send();
    return kFanModes[_.Fan];
}

const char* CoronaAC::nextFanMode() {
    return setFanMode(_.Fan + 1);
}

const char* CoronaAC::setACMode(const uint8_t acMode) {
    if (acMode > kACModesSize - 1) {
        _.Mode = 0;
    } else if (acMode < 0) {
        _.Mode = kACModesSize - 1;
    } else {
        _.Mode = acMode;
    }

    if (_.debug) {
        Serial.print("CMD: setting AC mode to ");
        Serial.printf("%s\n", kACModes[_.Mode]);
    }
    _.ac.setMode(_.Mode);
    _.ac.send();
    return kACModes[_.Mode];
}

const char* CoronaAC::nextACMode() {
    return setACMode(_.Mode + 1);
}
