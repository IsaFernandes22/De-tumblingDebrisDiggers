# @author: Isabella Fernandes
# This is the communication between the main satellite and the de-tumbling mite.
# This communication happens with a raspberryPi and camera (on the main satellite) 
# with an arduino (on the mite). The rasPi also communicates with the main satellite's
# control.

import cv2
import serial
import time
import dispatch
import socket

#set up serial communication with arduino
COM_PORT = '/dev/ttyUSB0'  # Adjust for setup with prototype (TODO)
BAUD_RATE = 9600
arduino = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Allow Arduino to initialize

# --- Computer-to-Computer Communication Setup ---
# Hardcoded IP and port of the target computer
TARGET_IP = "192.168.1.100"  # Replace with the IP address of the other computer (TODO)
TARGET_PORT = 5000           # Replace with the desired port (TODO)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP Socket

# Initialize Camera
try:
    cap = dispatch.init_camera()
except Exception as e:
    print(e)
    exit()

print("Camera and serial initialized. Press 'q' to quit.")

# TODO this is where the dispatch stuff will be analyzed


# TODO now lets focus on setting up communication with the main satellite

try:
    while True:
        # Check if there's data from the Arduino
        if arduino.in_waiting > 0:
            detumble_signal = arduino.readline().decode('utf-8').strip()

        if detumble_signal == "1":  # '1' indicates detumbled
            print("RSO detumbled. Sending ready signal to ground station.")

            # Notify the other computer
            message = "RSO_READY_FOR_CAPTURE"
            sock.sendto(message.encode('utf-8'), (TARGET_IP, TARGET_PORT))
            print(f"Message sent to {TARGET_IP}:{TARGET_PORT}") #logging message
                
        break
        
except KeyboardInterrupt:
    print("Interrupted by user. Exiting...")

finally:
    #clean up the space
    arduino.close()
    sock.close()
    print("Done releasing resources. Script finished.")

