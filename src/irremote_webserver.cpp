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

#define BUILTIN_LED D4
#define IR_LED D5
#define GPIO_IR_LED 14

///////// esp8266WebServer

#ifndef STASSID
#define STASSID "TP-LINK555"
#define STAPSK "ohmaigaaaloungewifi"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
static char outputBuffer[50];
const uint8_t obSize = sizeof(outputBuffer);

const char* www_username = "admin";
const char* www_password = "esp8266";

///////// IRremote8266

const char* kFanModes[4] = {"Auto", "Low", "Mid", "High"};
const uint8_t kFanModesSize = sizeof(kFanModes) / sizeof(kFanModes[0]);
const char* kACModes[4] = {"Heat", "Dry", "Cool", "Fan"};
const uint8_t kACModesSize = sizeof(kACModes) / sizeof(kACModes[0]);

const uint16_t kIrLed = IR_LED;
static uint8_t g_Temp = 18;
static uint8_t g_Fan = kCoronaAcFanAuto;
static uint8_t g_Mode = kCoronaAcModeHeat;
static bool g_Swing = false;
static bool g_Econo = true;
static bool g_Power = true;
IRCoronaAc ac(kIrLed);

void okResponse(const char* text) {
  server.send(200, "text/plain", text);
}
void failResponse(const char* text, const int code = 400) {
  server.send(code, "text/plain", text);
}

void turnACOn() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  char message[] = "CMD: turning on AC";
  Serial.println(message);
  ac.on();
  ac.setPower(true);
  ac.send();
  okResponse(message);
}

void turnACOff() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  char message[] = "CMD: turning off AC";
  Serial.println(message);
  ac.off();
  ac.setPower(false);
  ac.send();
  okResponse(message);
}

void setACTemp(const int desiredTemp) {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }

  if (desiredTemp > kCoronaAcMaxTemp) {
    g_Temp = kCoronaAcMaxTemp;
  } else if (desiredTemp < kCoronaAcMinTemp) {
    g_Temp = kCoronaAcMinTemp;
  } else {
    g_Temp = desiredTemp;
  }

  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting temp to %dC", g_Temp);

  Serial.println(outputBuffer);
  ac.setTemp(g_Temp);
  ac.send();
  okResponse(outputBuffer);
}

void reduceACTemp() {
  return setACTemp(g_Temp - 1);
}

void increaseACTemp() {
  return setACTemp(g_Temp + 1);
}

void toggleSwing() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  g_Swing = !g_Swing;
  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: toggling swing to %s", g_Swing ? "on": "off");
  Serial.println(outputBuffer);
  ac.setSwingVToggle(g_Swing);
  ac.send();
  okResponse(outputBuffer);
}

void setFanMode(const uint8_t desiredFanMode) {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }

  if (desiredFanMode > kFanModesSize - 1) {
    g_Fan = 0;
  } else if (desiredFanMode < 0) {
    g_Fan = kFanModesSize - 1;
  } else {
    g_Fan = desiredFanMode;
  }

  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting Fan mode to %s", kFanModes[g_Fan]);

  Serial.println(outputBuffer);
  ac.setFan(g_Fan);
  ac.send();
  okResponse(outputBuffer);
}

void rotateFanMode() {
  return setFanMode(g_Fan + 1);
}

void setACMode(const uint8_t desiredACMode) {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }

  if (desiredACMode > kACModesSize - 1) {
    g_Mode = 0;
  } else if (desiredACMode < 0) {
    g_Mode = kACModesSize - 1;
  } else {
    g_Mode = desiredACMode;
  }

  memset(outputBuffer, 0, obSize);
  snprintf(outputBuffer, obSize, "CMD: setting AC mode to %s", kACModes[g_Mode]);

  Serial.println(outputBuffer);
  ac.setMode(g_Mode);
  ac.send();
  okResponse(outputBuffer);
}

void rotateACMode() {
  return setACMode(g_Mode + 1);
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
    return server.requestAuthentication();
  }
  char message[] = "CMD: turning on full lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdFullOn, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdFullOn, kIRConRepeats);
  okResponse(message);
}

void lightsOff() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  char message[] = "CMD: turning off lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdOff, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdOff, kIRConRepeats);
  okResponse(message);
}

void lightsLessLight() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  char message[] = "CMD: lowering the lights";
  Serial.println(message);
  // IrSender.sendNEC(kIRConAddress, kIRConCmdLessLight, kIRConRepeats);
  sendNEC(GPIO_IR_LED, kIRConAddress, kIRConCmdLessLight, kIRConRepeats);
  okResponse(message);
}

void lightsMoreLight() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
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
      return server.requestAuthentication();
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
