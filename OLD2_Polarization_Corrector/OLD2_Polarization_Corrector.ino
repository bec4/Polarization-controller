/* 
 * VERY IMPORTANT: MAKE A README ABOUT THIS LATER
 * In order to serve as a DTE (transmitting/master), the transmit wire from the RS232 connector must go to pin 8 and the receive must go to pin 7.
 * In order to work as a DCE (connect to a PC/receiver), we must flip the two.
 * See pics in phone to clarify wire locations.
 */

#include <SoftwareSerial.h>

const byte rxPin = 2; // Serial receive pin (digital)
const byte txPin = 3; // Serial transmit pin (digital)
const byte pPinLo = 0; // Low (ground) photodetector pin (analog)
const byte pPinHi = 1; // High photodetector pin (analog)
const float pVolThr = 0.7; // Threshold for deciding to correct the waveplate orientation
const float changeDirectionThr = 0.5; // If we are rotating in one direction and make things worse by an amount equal to this threshold, we change directions
const float stepAngle = 2; // Angle (in degrees) that the waveplates will rotate each step
const float degreePeriod = 70; // Amount of time (in milliseconds) it takes for waveplate to rotate 1 degree
const int rotatePeriod = (int) stepAngle*degreePeriod; // Amount of time (in milliseconds) it takes for waveplate to rotate stepAngle

byte state = 0; // This variable determines which "mode" the algorithm is on. See guide:
/* STATE GUIDE
   0: Ready, no correction required
   1: Correcting
*/

int rotateTimer; // Timer to keep track of time since last rotation started
float pVolOut; // Stores the recorded voltage from the photodetector
float pVolList[] = {0,0,0,0,0}; // Remember to update vListSize below if you change element number
byte pVolListSize = 5; // See pVolList
float pVolAvg; // Stores the rolling average of the recorded voltage
float referenceValue; // When a correction becomes required, this variable stores the current voltage so that, after rotating a little bit, we can see if we are rotating in the right direction

bool rotatePositive = true; // This determines what direction the waveplates should rotate in order for the ellipticity to be reduced. The program automatically finds the correct setting by trial-and-error.
bool readyToRotate = true; // This determines if its time to rotate the waveplate again (i.e. if prev. rotation has finished)

SoftwareSerial RS232Serial(rxPin,txPin); // Instantiate serial object

void SMC100Begin() {
  /* This function brings the SMC100 to the 'ready' (green) state, preparing it for input.
   */
  
  RS232Serial.write("1RS\r\n"); // Send RS command (reset SCM100)
  delay(4000); // Give SMC100 time to restart
  RS232Serial.write("1TS\r\n"); // Send TS command (step 1 in SMC100 connection)
  delay(700); // Give SMC100 time to execute (this is necessary!)
  RS232Serial.write("1OR\r\n"); // Send OR command (step 2 in SMC100 connection)
  delay(10); // More time to execute
}

void Rotate(float angle, bool absolute = false) {
  /* This function will send a command to rotate a motor to the SMC100.
   * angle is in degrees.
   * absolute determines if the rotation will be relative to its current position or absolute
   */
  
  String decider = "R"; // This stores an "R" or an "A", depending on whether the rotation is absolute or not
  if (absolute) {
    decider = "A";
  }
  
  String angleString = String(angle); // Change the float into a String to be concatenated
  String instruction = "1P" + decider + angleString + "\r\n"; // Concatenate everything to form the coherent command
  RS232Serial.print(instruction); // Send PR or PA command
}

void UpdatePhotodiodeAverage(float pLo, float pHi) {
  /* This function will take the latest low and high pin measurements from the photodiode and calculate the next rolling average based on them. 
   * Note that it returns void. pVolOut and pVolAvg are updated automatically within the function.
   */
  
  // Calculate voltage from analog input
  pVolOut = (pHi - pLo)*5/1023.0;

  // Take rolling average
  pVolAvg = 0;
  for (int i = pVolListSize - 1; i > 0; i--) {
    pVolList[i] = pVolList[i - 1]; // Trickling values down the array
    pVolAvg += pVolList[i];
  }
  pVolList[0] = pVolOut; // Final step, add new data to begining of array
  pVolAvg += pVolOut; // ...and add new data to average
  pVolAvg /= pVolListSize; // Normalize average
}

void setup() {
  Serial.begin(9600); // Initialize USB serial connection to programming computer. For debugging purposes only.
  
  RS232Serial.begin(57600); // Initialize RS232 serial connection to SMC100 motor controller.

  Serial.println("Connecting to SMC100..."); // This is being sent via USB to programming computer, not to the SMC100
  SMC100Begin(); // Connect to the SMC100 and prepare it to receive orders
  Serial.println("DONE");

  UpdatePhotodiodeAverage(analogRead(pPinLo), analogRead(pPinHi)); // Update our photodiode values
  referenceValue = pVolOut;
}

void loop() { 
  UpdatePhotodiodeAverage(analogRead(pPinLo), analogRead(pPinHi)); // Update our photodiode values

  Serial.print(10*pVolAvg); Serial.print("\t"); // Plot a zoomed in 
  Serial.print(10*pVolOut); Serial.print("\t");
  Serial.println(10.0*state);
  
  // This is the heart of the algorithm. Depending on the state of the system we will execute different commands.
  if (state == 0) { // If system does not need correcting
    if (pVolOut >= pVolThr) { // If, now, the system needs correcting
      state = 1; // Switch into correction state
      referenceValue = pVolOut; // Set up a reference value. This value will be compared to the post-rotation value to see if we are rotating in the correct direction.
    }
  }

  else if (state == 1) { // If system needs correcting
    if (millis() - rotateTimer > rotatePeriod) { // If the previous rotation is over
      readyToRotate = true; // Prepare to start new rotation
    }
    
    if (readyToRotate) {
      readyToRotate = false; // This will stop the code from executing more unwanted rotations after this one
      rotateTimer = millis(); // Start rotation timer to know how long to wait
      
      if (rotatePositive) {
        Rotate(1.0*stepAngle); // Rotate the waveplate in the positive direction
      }
      else {
        Rotate(-1.0*stepAngle); // Rotate the waveplate in the negative direction
      }
    }

    // Now we check if we goofed up OR corrected
    if (pVolAvg - referenceValue > changeDirectionThr) { // If we are going too far in the wrong direction...
      rotatePositive = !rotatePositive; // ...change direction...
      referenceValue = pVolAvg; // ...and reset our reference value
    }
    else if (pVolAvg < 0.9*pVolThr) { // (NOTE: the 0.9 is an arbitrary choice) If we are back under the threshold (and a little further than that still)...
      state = 0; // ...go back to the "ready" state (stop rotating)
    }
  }
  
  delay(10); // Give Arduino and sensors time to catch up
}
