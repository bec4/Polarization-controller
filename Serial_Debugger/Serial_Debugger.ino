#include <SoftwareSerial.h>

const byte rxPin = 2;
const byte txPin = 3;

byte data;

SoftwareSerial RS232Serial(rxPin, txPin);

void setup() {
  Serial.begin(57600);
  RS232Serial.begin(57600);
}

void loop() {
  if (Serial.available() > 0) {
    data = Serial.read();
    
    Serial.write("1TS\r\n");
    RS232Serial.write(data);
  }
}
