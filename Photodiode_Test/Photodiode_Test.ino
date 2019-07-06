/*This is a test for the PDA36A photodiode.
Wire photodiode high and ground on either side of a 2.2MOhm resistor.
Connect high to A0 and ground to A1.
Set internal gain on the PDA36A to 30dB - this should produce a max output voltage near 3.5V*/

int p_0;
int p_1;

float vOut = 0;
float vList[] = {0,0,0,0,0}; // Remember to update vListSize below if you change element number 
float vAvg;

int vListSize = 5;

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read high and low pin
  p_0 = analogRead(A0);
  p_1 = analogRead(A1);
  
  // Calculate voltage from analog input
  vOut = (p_0 - p_1)*5/1023.0;

  // Take rolling average
  vAvg = 0;
  for (int i = 0; i < vListSize; i++) {
    int curr = vListSize - i;
    vList[curr] = vList[curr - 1]; // Trickling values down the array
    vAvg += vList[curr];
  }
  vList[0] = vOut; // Final step, add new data to begining of array
  vAvg += vOut; // ...and add new data to average
  vAvg /= vListSize; // Normalize average

  Serial.println(10*vAvg); // Print a "zoomed in" average
  
  delay(50);
}
