# @author: Isabella Fernandes
# This is the communication between the main satellite and the de-tumbling mite.
# This communication happens with a raspberryPi and camera (on the main satellite) 
# with an arduino (on the mite). The rasPi also communicates with the main satellite's
# control.

import cv2
import serial
import time
import launch

#set up serial communication with arduino
COM_PORT = '/dev/ttyUSB0'  # Adjust for setup with prototype (TODO)
BAUD_RATE = 9600
arduino = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Allow Arduino to initialize

#this is where we process the frame from the camera and detect where the RSO is
def processFrame(frame):

    return

