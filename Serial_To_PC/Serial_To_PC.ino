#define BITDELAY 100 // Microseconds to wait between bits at 9600 baud
#define HALFBITDELAY 50 

const byte rPin = 2;
const byte tPin = 3;
byte receivedData;

void RS232Print(int data) {
  //SEND START BIT
  digitalWrite(tPin,LOW);
  delayMicroseconds(BITDELAY);

  //SEND DATA
  for (byte selector = 1; selector > 0; selector <<= 1) { // Iterates over each of the binary positions in the data
    if (data & selector){ // If the current bit position is one...
     digitalWrite(tPin,HIGH); // Send 1
    }
    else {
     digitalWrite(tPin,LOW); // Send 0
    }
    delayMicroseconds(BITDELAY); // Wait before sending next bit
  }
  
  //SEND STOP BIT
  digitalWrite(tPin, HIGH);
  delayMicroseconds(BITDELAY);
}

byte RS232Read() {
  byte data = 0;
  byte newBit = 0;

  // WAIT FOR START BIT
  while (digitalRead(rPin));

  // SEND DATA
  delayMicroseconds(HALFBITDELAY); // I don't know why this is here, but without it the data gets messed up on the way out
    
  for (int offset = 0; offset < 8; offset++) {
   delayMicroseconds(BITDELAY);
   newBit = digitalRead(rPin) << offset; // Read the next bit and make a number with it in the correct position
   data |= newBit; // This will change the relevant position of our total data to whatever the new bit has
  }
  
  // WAIT FOR STOP BIT
  delayMicroseconds(BITDELAY);

  // (EXTRA WAIT JUST IN CASE)
  //delayMicroseconds(BITDELAY);
  
  return data;
}

void RS232PrintString(char string[], byte stringLength) {
  for (byte i = 0; i < stringLength; i++) {
    RS232Print(string[i]);
  }
}

void setup() {
  pinMode(rPin,INPUT);
  pinMode(tPin,OUTPUT);
  digitalWrite(tPin,HIGH);
  delay(2);
  digitalWrite(13,HIGH); //turn on debugging LED
  RS232Print('h'); //debugging hello
  RS232Print('i');
  RS232PrintString("This is a test. 123456789\n", 26);
}

void loop() {
    receivedData = RS232Read(); 
    RS232Print(toupper(receivedData));
}
