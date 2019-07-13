import serial  # Import the serial library for communicaing over the computer's ports serially
import time

def rotate(deg, ser, absolute = False):  # float deg -> degrees to rotate by, serial ser -> serial object being acted on, bool absolute -> is rotation absolute (True) or relative (False)?
    # This function will automatically write the data to rotate the PR50 waveplate. Note it can only take one command at a time, until rotation is finished
    if absolute:
        decider = 'A'
    else:
        decider = 'R'
    string = '2P' + decider + str(deg) + '\r\n'
    return ser.write(str.encode(string))

# Instantiate my serial object
ser = serial.Serial('COM5', 57600, xonxoff = False, timeout = 5)

 # Send command to rotate
ser.write(b'1TS\r\n')
time.sleep(3)
ser.write(b'1OR\r\n')
time.sleep(3)
rotate(70, ser)

# Close serial connection
ser.close()
