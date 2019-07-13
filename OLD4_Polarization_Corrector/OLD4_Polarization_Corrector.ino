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
const byte masterDelay = 6; // Time (in milliseconds) to wait before Arduino repeats loop
const int superAvgN = 60; // Determines the amount of measurements averaged to find the superAvg
const float stepAngle = 5; // Angle (in degrees) that the waveplates will rotate each step
const float degreeDuration = 30; // Amount of time (in milliseconds) it takes for waveplate to rotate 1 degree
const float pVolThr = 0.18; // Threshold (in volts) for deciding to correct the waveplate orientation

byte state = 0;
/* STATE GUIDE
   0 -> READY: Current detected intensity is within threshold
   1 -> SLOPING: Detecting which way to rotate waveplate in order to minimize detected intensity
   2 -> STEPPING: Moving towards the minimum of detected intensity
   3 -> SETTLING: Stopping at detected minimum
*/

bool superAvgDone; // Determines if we have finished taking the super average mentioned
bool stateReady = true; // A multi-purpose boolean used in the state machine to determine readiness for different actions within the state
bool skipMeasurement = false; // Used to determine if we want to re-run the state machine without having to take the super average inbetween
byte currID = 1; // Determines which waveplate is currently being moved. Half-waveplate is 1, quarter-waveplate is 2.
byte pVolListSize = 5; // See pVolList
int rotateDirection[] = {1,1}; // This determines what direction the waveplates should rotate in order for the ellipticity to be reduced. The program automatically finds the correct settings by trial-and-error. Possible values are 1 and -1. First value is for ID = 1, second for ID = 2.
int rotateTimer = 0; // Used to tell how long it's been since last rotation started
int rotateDuration = 0; // Used to tell if we are done with previous rotation
int avgCounter = 0; // Counts how many data points we have averaged for sVolSuperAvg
float pVolOut; // Stores the recorded voltage from the photodetector
float pVolList[] = {0,0,0,0,0}; // Remember to update vListSize below if you change element number
float pVolAvg; // Rolling average of the recorded voltage
float superAvg; // Average of many measurments of the recorded voltages. This number determines the "correctness" of our current waveplate orientations. 
float oldSuperAvg; // Previous value of superAvg. Used to compare.

SoftwareSerial RS232Serial(rxPin,txPin); // Instantiate serial object

void SMC100Begin(byte id) {
  /* This function brings the SMC100 with the given ID (address) to the 'ready' (green) state, preparing it for input.
   */

  String idString = String(id); // Change the byte into a String to be concatenated
  
  RS232Serial.print(idString + "RS\r\n"); // Send RS command (reset SCM100)
  delay(5000); // Give SMC100 time to restart
  RS232Serial.print(idString + "TS\r\n"); // Send TS command (step 1 in SMC100 connection)
  delay(700); // Give SMC100 time to execute (this is necessary!)
  RS232Serial.print(idString + "OR\r\n"); // Send OR command (step 2 in SMC100 connection)
  delay(10); // More time to execute
}

void Rotate(byte id, float angle, bool absolute = false) {
  /* This function will send a command to rotate a motor to the SMC100.
   * angle is in degrees.
   * absolute determines if the rotation will be relative to its current position or absolute.
   * Rotate starts a timer rotateTimer that can be used to get when the rotation has been finished.
   */
  
  rotateDuration = (int) abs(angle)*degreeDuration; // Define length of this rotation
  rotateTimer = millis(); // Start rotation timer
  
  String decider = "R"; // This stores an "R" or an "A", depending on whether the rotation is absolute or not
  if (absolute) {
    decider = "A";
  }
  
  String angleString = String(angle); // Change the float into a String to be concatenated
  String idString = String(id); // Change the byte into a String to be concatenated
  String instruction = idString + "P" + decider + angleString + "\r\n"; // Concatenate everything to form the coherent command
  RS232Serial.print(instruction); // Send PR or PA command
}

void UpdatePhotodetectorAverage(float pLo, float pHi) {
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

void UpdatePlot(float zoom, float val1, float val2, float val3, float val4) {
   /* This function will update the desired plotting values.
   * The values will be scaled up by a factor of zoom.
   */

  Serial.print(zoom*val1); Serial.print("\t"); // Plot val1
  Serial.print(zoom*val2); Serial.print("\t"); // Plot val2
  Serial.print(zoom*val3); Serial.print("\t"); // Plot val3
  Serial.println(zoom*val4); // Plot val4
}

void UpdateSuperAvg(float measurement) {
  /* This function both updates our value for the super average and checks if we are finished building this average.
   * If it detects the average is complete, we prepare to enter the state machie again.
   */
  if (avgCounter == 0) { // If we are just starting to take this new average
    oldSuperAvg = superAvg; // Set the current average as the old average
    superAvg = 0; // Reset superAvg
  }
  superAvgDone = false; // Say we are not done taking averages
  superAvg += measurement/superAvgN; // Update the super average
  avgCounter ++; // Update the counter
  if (avgCounter >= superAvgN) { // If we have finished making the super average
    superAvgDone = true; // Say we are done taking averages
    avgCounter = 0; // Reset the counter
  }
}

void setup() {
  Serial.begin(9600); // Open USB serial communications to PC, for debugging
  RS232Serial.begin(57600); // Open RS232 serial connection to SMC100 motor controllers.

  SMC100Begin(1); // Begin motor controller 1
  SMC100Begin(2); // Begin motor controller 2
}

void loop() {
  UpdatePhotodetectorAverage(analogRead(pPinLo), analogRead(pPinHi)); // Update our photodetector values
  UpdatePlot(10, pVolThr, pVolAvg, oldSuperAvg, float(state)); // Plot zoomed in graph with photodetector threshold, the photodetector rolling average, the previous super average the state. Zoom is set to an arbitrary number (10 right now).
if (millis() - rotateTimer > rotateDuration && !skipMeasurement) { // If we are finished rotating AND we don't want to skip this measurement
    UpdateSuperAvg(pVolOut); // Update our super average and determine if we are ready to go into the state machine again
  }
  
  if (superAvgDone || skipMeasurement) { // We will only enter the state machine if we are done taking the super average any previous action (rotating, observing) OR if we decided to skip the measurement last time
    // Below is the heart of the code: the state machine.
    // Depending on the current state, the program will execute different lines of code.

    skipMeasurement = false; // Reset skipMeasurement
    superAvgDone = false; // Reset superAvgDone
    
    switch(state) {
      case 0: // READY: Current detected intensity is within threshold
      if (superAvg >= pVolThr) { // If we detect that our system needs correcting
        state = 1; // Enter SLOPING state to begin correction
      }
      break; // Exit this case
  
      case 1: // SLOPING: Detecting which way to rotate waveplate in order to minimize detected intensity
      // Here, stateReady is used to determine if it's the first time in state 1
      if (stateReady) { // If it's the first time in state 1
        stateReady = false; // Say it is no longer the first time
        Rotate(currID, rotateDirection[currID - 1]*stepAngle); // Rotate in whatever preset direction we have
      }
      else if (oldSuperAvg > superAvg) { // If the rotation was in the right direction
        state = 2; // Move on to the STEPPING state
        stateReady = true; // Reset stateReady
      }
      else { // If the rotation was in the wrong direction
        rotateDirection[currID - 1] *= -1; // Change direction for this waveplate
        Rotate(currID, rotateDirection[currID - 1]*stepAngle); // Rotate in our new direction

        state = 2; // Move on to the STEPPING state
        stateReady = true; // Reset stateReady
      }
      break; // Exit this case
  
      case 2: // STEPPING: Moving towards the minimum of detected intensity
      // Here, stateReady is used to decide if we are in a rotation or measurement cycle. True -> rotate, False -> measurement.
      if (stateReady) { // If we want to rotate
        Rotate(currID, rotateDirection[currID - 1]*stepAngle); // Rotate in the direction we found in state 1
        stateReady = false; // Switch to measurement mode
      }
      else { // If we want to measure
        if (oldSuperAvg < superAvg) { // If we overshot the minimum
          state = 3; // Move on to SETTLING state
        }
        stateReady = true; // Reset stateReady/switch to rotation mode
        skipMeasurement = true; // Order measurement to be skipped and to go straight into rotation orders
      }      
      break; // Exit this case
  
      case 3: // SETTLING: Stopping at detected minimum
      // Here, stateReady is used to decide if it's the first time in state 3
      if (stateReady) { // If it's the first time in state 3
        
        stateReady = false; // Say it's no longer the first time
        Rotate(currID, -1*rotateDirection[currID - 1]*stepAngle); // Reverse the last rotation in state 2, which overshot
      }
      else { // If it's not the first time in state 3, and we have already corrected the rotation
        if (superAvg < pVolThr*0.95) { // If our new minimum is well below the threshold (0.95 is an arbitraty choice)
          state = 0; // We have corrected the polarization, and we return to our READY state
          stateReady = true; // Reset stateReady
        }
        else { // If our new minimum is not safely below the desired threshold
          currID = (byte) ((currID - 1.5)*-1 + 1.5); // Change our target waveplate (1 -> 2 and 2 -> 1)
          state = 1; // Re-start the minimization process with our new waveplate
          skipMeasurement = true; // Skip measuring before the next rotation
          stateReady = true; // Reset stateReady
        }
      }
      break; // Exit this case
      
    }
  }

  delay(masterDelay); // Give Arduino and sensors time to catch up
}
