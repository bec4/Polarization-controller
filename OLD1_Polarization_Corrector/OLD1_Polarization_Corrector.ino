#define BITDELAY 100 // Microseconds to wait between bits at 9600 baud
#define HALFBITDELAY 50

const byte rPin = 2; // Receive pin on the Arduino
const byte tPin = 3; // Transmit pin on the Arduino

byte receivedData; // For test code at start of loop
byte state = 0;
/* STATE GUIDE
   0: Ready, no correction required
   1: Correcting, moving ccw
   2: Correcting, moving cw
*/

void RS232Print(byte data) {
  /* This function sends one single byte of data through the RS323 connection.
   *
   * Note that the size of a packet is STRICTLY 8 bits (1 byte). 
   * As such, it cannot send strings because these do not map to ASCII and require more than 8 bits.
   * To send a string, please use the RS232PrintString() function.
   */
  
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
  /* This function receives one single byte of data through the RS232 connection.
   * 
   * Note that the size of a packet is STRICTLY 8 bits (1 byte). 
   * As such, it cannot send strings because these do not map to ASCII and require more than 8 bits.
   */
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
  /* This function sends an entire ASCII string through the RS232 connection.
   * 
   * This is a BLOCKING function, meaning the program will not advance until the entire message is sent.
   * Message sending time is roughly (no. of characters)*(100 microseconds).
   */

  // We iterate through the input string and send each character one by one
  for (byte i = 0; i < stringLength; i++) {
    RS232Print(string[i]);
  }
}

void SMC100Begin() {
  /* This function brings the SMC100 to the 'ready' (green) state, preparing it for input.
   */
  
  RS232PrintString("1TS\r\n", 5); // Send TS command (step 1 in SMC100 connection)
  delay(10); // Give SMC100 time to execute. Note for future testing: check if this is necessary
  RS232PrintString("1OR\r\n", 5); // Send OR command (step 2 in SMC100 connection)
  delay(10); // More time to execute
}

void Rotate(float angle, bool absolute) {
  String decider = "R";
  if (absolute) {
    decider = "A";
  }
  String angleString = String(angle);
  String instruction = "1P" + decider + angleString + "\r\n";
  for (byte i = 0; i < instruction.length(); i++) {
    RS232Print(instruction.charAt(i));
  }
  //RS232Print(1PA/Rdeg\r\n); \\TS COMMAND TURNS LIGHT FROM RED TO ORANGE. OR TURNS FROM ORANGE TO GREEN.
}

void setup() {
  Serial.begin(9600); // Engage USB serial connection to programming computer. For debugging purposes only.

  pinMode(rPin,INPUT); // Prepare receiver pin
  pinMode(tPin,OUTPUT); // Prepare transmitter pin

  digitalWrite(tPin,HIGH); // This is the correct starting bit for a byte string. Without this the first received message is incorrect. 
  delay(2); // Give the system ample time to flush the starting bit into every connection

  Serial.println("Connecting to SMC100...");
  SMC100Begin(); // Connect to the SMC100 and prepare it to receive orders
  Serial.println("DONE");
  //delay(3000);

  Serial.println("Rotating cw");
  Rotate(90, false);
  //delay(5000);

  Serial.println("Rotating ccw");
  Rotate(-90, false);
}

void loop() {
  // Test code to debug connection:
    receivedData = RS232Read(); 
    RS232Print(toupper(receivedData));
  
  if (state == 0) {
    
  }
  
  else if (state == 1) {
    
  }

  else if (state == 2) {
    
  }
}
