# Author: Isabella Fernandes
# This is the communication between the main satellite and the de-tumbling mite.
# This communication happens with a raspberryPi and camera (on the main satellite) 
# with an arduino (on the mite). The rasPi also communicates with the main satellite's
# control.

import cv2
import config
import serial
import time
import dispatch
import comms
import socket

#set up serial communication with arduino
arduino = serial.Serial(config.COM_PORT, config.BAUD_RATE, timeout=1)
time.sleep(2)  # Allow Arduino to initialize

# --- Computer-to-Computer Communication Setup ---
# Hardcoded IP and port of the target computer
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP Socket

# Initialize Camera
try:
    cap = dispatch.init_camera(config.CAMERA_INDEX)
except Exception as e:
    print(e)
    exit()

print("Camera and serial initialized. Press 'q' to quit.")

# Analyze the dispatch stuff

velocity, location = None, None
previous_location = None
dt = 1  # Assume a fixed time delta for simplicity

try:
    while True:
        # Process camera frame for dispatch parameters
        ret, frame = cap.read()
        if not ret:
            print("Failed to read frame from camera.")
            break

        velocity, location = dispatch.processFrame(frame, previous_location, dt)
        if location:
            print(f"Detected RSO at {location} with velocity {velocity}")
            previous_location = location

            # Optionally, send RSO parameters to the ground station
            dispatch_message = f"RSO detected at {location} with velocity {velocity}"
            comms.notify_ground_station(sock, dispatch_message)
        
        # Check if there's data from the Arduino
        if arduino.in_waiting > 0:
            detumble_signal = arduino.readline().decode('utf-8').strip()

        if detumble_signal == "1":  # '1' indicates detumbled
                print("RSO detumbled. Sending ready signal to ground station.")
                comms.notify_ground_station(sock, "RSO_READY_FOR_CAPTURE")
                break  # Exit after sending the detumbled signal
        break
        
except KeyboardInterrupt:
    comms.send_log("Interrupted by user. Exiting...")

finally:
    cap.release()  # Release the camera
    cv2.destroyAllWindows()  # Close OpenCV windows
    arduino.close()
    sock.close()
    comms.send_log("Done releasing resources. Script finished.")