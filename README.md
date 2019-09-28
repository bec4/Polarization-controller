# Automatic polarization controller
This repository contains the Arduino sketches needed to control the polarization of a laser beam using a half- and quarter-wave plate in Newport SM100 rotating mounts. The Arduino controls the motor controllers via RS-232 serial. As input, it takes the light signal of a photodiode that's in the rejected port of a beam splitter. The Arduino has some arbitrary threshold, below which it doesn't do anything, and above which it minimizes the light on the photodiode. 

To use the Arduino for RS-232 communication, we largely follow [this Arduino tutorial](https://www.arduino.cc/en/Tutorial/ArduinoSoftwareRS232). It requires a separate chip which can be powered using the 5V out on the Arduino (MAX3323E). See below for the wiring diagram.

## Notes
1. The Arduino can function as both a DCE (receiving) and DTE (transmitting) unit. The diagram below is set to a DTE configuration, but can be made into a DCE configuration if the cables that go to pins 
2. The PDA36A photodetector should be set to a gain of 30dB. Otherwise, the output may be saturated for high input intensities and the threshold for correcting polarization may become detuned.
3. pPinLo, pPinHi, txPin and rxPin may all be changed in the Arduino code.
4. Do not confuse txPin and rxPin for pins 0 and 1 on the Arduino, which are labeled RX and TX.


![Wiring diagram](/polarization_corrector_pin_diagram.svg)


## Wishlist
* Currently, the threshold above which the Arduino starts the minimum search is hardcoded in the sketch (`pVolThr`). This makes it hard to just drop in the polarization corrector and expect it to work. Ideally, the Arduino would do a full rotation of both plates at when it initializes, and map out the minimum. Then it should set the threshold to something like 5% above this minimum.