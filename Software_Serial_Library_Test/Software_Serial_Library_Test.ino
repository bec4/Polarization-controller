#include <SoftwareSerial.h>

const byte rPin = 2;
const byte tPin = 3;

SoftwareSerial RS232Serial(2,3);

void setup() {
  RS232Serial.begin(57600);
}

void loop() {
  RS232Serial.println("Hiya there!");
  delay(1000);
}
