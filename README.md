## Notes
1. The Arduino can function as both a DCE and DTE unit. The diagram below is set to a DTE configuration, but can be made into a DCE configuration if the cables that go to pins 
2. The PDA36A photodetector should be set to a gain of 30dB. Otherwise, the output may be saturated for high input intensities and the threshold for correcting polarization may become detuned.
3. pPinLo, pPinHi, txPin and rxPin may all be changed in the Arduino code.
4. Do not confuse txPin and rxPin for pins 0 and 1 on the Arduino, which are labeled RX and TX.


![Wiring diagram](/polarization_corrector_pin_diagram.svg)
