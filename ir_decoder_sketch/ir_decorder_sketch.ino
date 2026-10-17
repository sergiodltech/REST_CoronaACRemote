#define DECODE_NEC
#include <IRremote.hpp>

const int IR_RECEIVE_PIN = D5;

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, true, LED_BUILTIN);

//  pinMode(D2, OUTPUT);
//  digitalWrite(D2, LOW);
//  pinMode(D3, OUTPUT);
//  digitalWrite(D3, LOW);
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.printIRResultRawFormatted(&Serial, true);
    IrReceiver.printIRResultShort(&Serial);
    //switch (IrReceiver.decodedIRData.decodedRawData) {
    //  case 0xF8077F80:
    //    digitalWrite(D2, HIGH);
    //    break;
    //  case 0xF40B7F80:
    //    digitalWrite(D2, LOW);
    //    break;
    //  case 0xF9067F80:
    //    digitalWrite(D3, HIGH);
    //    break;
    //  case 0xF50A7F80:
    //    digitalWrite(D3, LOW);
    //    break;
    //}
    IrReceiver.resume();
  }
}