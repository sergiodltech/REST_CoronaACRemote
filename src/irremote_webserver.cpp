#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>

#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Corona.h>

#include <TinyIRSender.hpp>

#include "CoronaAC.h"
#include "secrets.h"

#define BUILTIN_LED D4
#define IR_LED D5
#define GPIO_IR_LED 14

///////// esp8266WebServer

const char* ssid = AP_SSID;
const char* password = AP_PASS;

ESP8266WebServer server(80);
static char outputBuffer[50];
const uint8_t obSize = sizeof(outputBuffer);

const char* www_username = BAUTH_USER;
const char* www_password = BAUTH_PASS;

///////// IRremote8266

// Local defaults
const bool AC_DEBUG = true;
const uint8_t AC_TEMP = 18;
const bool AC_SWING = false;
const bool AC_ECONO = true;
const uint8_t AC_FAN = CORAC_DEFAULT_FAN;
const uint8_t AC_MODE = kCoronaAcModeHeat;

// Globals
static bool g_DebugAC = AC_DEBUG;
static uint8_t g_Temp = AC_TEMP;
static bool g_Swing = AC_SWING;
static bool g_Econo = AC_ECONO;
static uint8_t g_Fan = AC_FAN;
static uint8_t g_Mode = AC_MODE;

CoronaAC ac(IR_LED, g_DebugAC, g_Temp);

void okResponse(const char* text) {
  server.send(200, "text/plain", text);
}
void failResponse(const char* text, const int code = 400) {
  server.send(code, "text/plain", text);
}

void turnACOn() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  char message[] = "CMD: turning on AC";
  Serial.println(message);
  ac.turnOn();
  okResponse(message);
}

void turnACOff() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  char message[] = "CMD: turning off AC";
  Serial.println(message);
  ac.turnOff();
  okResponse(message);
}

uint8_t sanitizeACTemp(const uint8_t desiredTemp) {
  uint8_t result;
  if (desiredTemp > kCoronaAcMaxTemp) {
    result = kCoronaAcMaxTemp;
  } else if (desiredTemp < kCoronaAcMinTemp) {
    result = kCoronaAcMinTemp;
  } else {
    result = desiredTemp;
  }
  return result;
}

void setACTemp(const uint8_t desiredTemp) {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }

  g_Temp = sanitizeACTemp(desiredTemp);
  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting temp to %dC", g_Temp);

  Serial.println(outputBuffer);
  ac.setTemp(g_Temp);
  okResponse(outputBuffer);
}

void reduceACTemp() {
  setACTemp(g_Temp - 1);
}

void increaseACTemp() {
  setACTemp(g_Temp + 1);
}

void toggleSwing() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  g_Swing = !g_Swing;
  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: toggling swing to %s", g_Swing ? "on": "off");
  Serial.println(outputBuffer);
  ac.setSwing(g_Swing);
  okResponse(outputBuffer);
}

void toggleEcono() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  g_Econo = !g_Econo;
  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: toggling economic mode to %s", g_Econo ? "on": "off");
  Serial.println(outputBuffer);
  ac.setEcono(g_Econo);
  okResponse(outputBuffer);
}

uint8_t sanitizeFanMode(const uint8_t desiredFanMode) {
  uint8_t result;
  if (desiredFanMode > kFanModesSize - 1) {
    result = 0;
  } else if (desiredFanMode < 0) {
    result = kFanModesSize - 1;
  } else {
    result = desiredFanMode;
  }
  return result;
}

void setFanMode(const uint8_t desiredFanMode) {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }

  g_Fan = sanitizeFanMode(desiredFanMode);
  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting Fan mode to %s", kFanModes[g_Fan]);

  Serial.println(outputBuffer);
  ac.setFanMode(g_Fan);
  okResponse(outputBuffer);
}

void rotateFanMode() {
  setFanMode(g_Fan + 1);
}

uint8_t sanitizeACMode(const uint8_t desiredACMode) {
  uint8_t result;
  if (desiredACMode > kACModesSize - 1) {
    result = 0;
  } else if (desiredACMode < 0) {
    result = kACModesSize - 1;
  } else {
    result = desiredACMode;
  }
  return result;
}

void setACMode(const uint8_t desiredACMode) {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }

  g_Mode = sanitizeACMode(desiredACMode);
  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting AC mode to %s", kACModes[g_Mode]);

  Serial.println(outputBuffer);
  ac.setACMode(g_Mode);
  okResponse(outputBuffer);
}

void rotateACMode() {
  setACMode(g_Mode + 1);
}

void setACState(const uint8_t temp = -1, const uint8_t mode = -1, const uint8_t fan = -1,
                const uint8_t swing = -1, const uint8_t econo = -1, const uint8_t debug = -1) {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }

  CoronaACState newState = ac.getState();
  if (temp != -1) {
    g_Temp = sanitizeACTemp(temp);
    newState.Temp = g_Temp;
  }
  if (mode != -1) {
    g_Mode = sanitizeACMode(mode);
    newState.Mode = g_Mode;
  }
  if (fan != -1) {
    g_Fan = sanitizeFanMode(fan);
    newState.Fan = g_Fan;
  }
  if (swing != -1) {
    g_Swing = swing > 0;
    newState.isSwinging = g_Swing;
  }
  if (econo != -1) {
    g_Econo = econo > 0;
    newState.isEcono = g_Econo;
  }
  if (debug != -1) {
    g_DebugAC = debug > 0;
    newState.debug = g_DebugAC;
  }

  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting AC state to %s", newState.ac.toString().c_str());
  Serial.println(outputBuffer);
  ac.setState(newState);
  okResponse(outputBuffer);
}

///////// IRRemote, NEC Commands

const uint16_t kIRConAddress = 0x0CE7;
const uint8_t kIRConCmdFullOn = 0x97;
const uint8_t kIRConCmdOff = 0x8B;
const uint8_t kIRConCmdNightLight = 0x8F;
const uint8_t kIRConCmdScene = 0xA2;
const uint8_t kIRConCmdMoreLight = 0x9B;
const uint8_t kIRConCmdLessLight = 0x9F;
const uint8_t kIRConRepeats = 1;

void lightsFullOn() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  char message[] = "CMD: turning on full lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdFullOn, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdFullOn, kIRConRepeats);
  okResponse(message);
}

void lightsOff() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  char message[] = "CMD: turning off lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdOff, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdOff, kIRConRepeats);
  okResponse(message);
}

void lightsLessLight() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  char message[] = "CMD: lowering the lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdLessLight, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdLessLight, kIRConRepeats);
  okResponse(message);
}

void lightsMoreLight() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return;
  }
  char message[] = "CMD: increasing the lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdMoreLight, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdMoreLight, kIRConRepeats);
  okResponse(message);
}

///////// Blinking LED functions

void blink_normal_led() {
  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);
  digitalWrite(BUILTIN_LED, HIGH);
}
void blink_ir_led() {
  digitalWrite(IR_LED, HIGH);
  delay(1000);
  digitalWrite(IR_LED, LOW);
}



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  delay(200);

  // esp8266WebServer
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  ArduinoOTA.begin();

  ////// Login test
  server.on("/", []() {
    if (!server.authenticate(www_username, www_password)) {
      server.requestAuthentication();
      return;
    }
    server.send(200, "text/plain", "Login OK");
    blink_normal_led();
  });

  server.on("/turnACOn", turnACOn);
  server.on("/turnACOff", turnACOff);
  server.on("/increaseTemp", increaseACTemp);
  server.on("/reduceTemp", reduceACTemp);
  server.on(UriBraces("/setTemp/{}"), []() {
    String temp = server.pathArg(0);
    setACTemp(temp.toInt());
  });
  server.on("/toggleSwing", toggleSwing);
  server.on("/rotateFanMode", rotateFanMode);
  server.on(UriBraces("/setFanMode/{}"), []() {
    String fanMode = server.pathArg(0);
    uint8_t desiredFanMode;
    if (fanMode == "Low") {
      desiredFanMode = 1;
    } else if (fanMode == "Mid") {
      desiredFanMode = 2;
    } else if (fanMode == "High") {
      desiredFanMode = 3;
    } else {
      desiredFanMode = 0; // Auto
    }
    setFanMode(desiredFanMode);
  });
  server.on("/rotateACMode", rotateACMode);
  server.on(UriBraces("/setACMode/{}"), []() {
    String acMode = server.pathArg(0);
    uint8_t desiredACMode;
    if (acMode == "Heat") {
      desiredACMode = 0;
    } else if (acMode == "Dry") {
      desiredACMode = 1;
    } else if (acMode == "Cool") {
      desiredACMode = 2;
    } else {
      desiredACMode = 3; // Fan
    }
    setACMode(desiredACMode);
  });
  server.on("/turnLightsOn", lightsFullOn);
  server.on("/turnLightsOff", lightsOff);
  server.on("/reduceLights", lightsLessLight);
  server.on("/increaseLights", lightsMoreLight);

  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
