# Author: Isabella Fernandes
# This is the communication between the main satellite and the de-tumbling mite.
# The Raspberry Pi (on the main satellite) communicates with the Arduino (on the mite)
# via RF instead of a serial connection. The RasPi also communicates with the 
# main satellite's control and camera.

import cv2
from src import config
from src import dispatch
from src import comms
import socket
from rpi_rf import RFDevice
import time
from collections import deque  # For storing velocity history

# --- RF Communication Setup ---
rf_receiver = RFDevice(config.RF_GPIO)  # Set up RF receiver on the defined GPIO pin
rf_receiver.enable_receive()  # Start listening for RF signals

# --- Computer-to-Computer Communication Setup ---
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP Socket

# Initialize Camera
try:
    cap = dispatch.init_camera(config.CAMERA_INDEX)
except Exception as e:
    print(e)
    exit()

print("Camera and RF receiver initialized. Press 'q' to quit.")

velocity_history = deque(maxlen=5)  # Store the last 5 velocity values
previous_location = None
dt = 1  # Assume a fixed time delta for simplicity

detumble_signal_count = 0  # Counter for consecutive "detumbled" signals
REQUIRED_CONSECUTIVE_SIGNALS = 5  # Number of times the signal must be received consecutively

try:
    while True:
        # Process camera frame for dispatch parameters
        ret, frame = cap.read()
        if not ret:
            print("Failed to read frame from camera.")
            break

        velocity, location = dispatch.processFrame(frame, previous_location, dt)

        if location:
            velocity_history.append(velocity)  # Store velocity in the history queue
            previous_location = location  # Update location

            # Calculate average velocity over the last N frames
            avg_velocity = sum(velocity_history) / len(velocity_history) #TODO test this

            print(f"Detected RSO at {location} with avg velocity {avg_velocity}")

            # Send averaged velocity to the ground station
            dispatch_message = f"RSO detected at {location} with avg velocity {avg_velocity}"
            comms.notify_ground_station(sock, dispatch_message)
        
        # --- Check if there's data from the RF Receiver ---
        # TODO might need to be changed if we receive more than just a de-tumble signal
        if rf_receiver.rx_code is not None:
            received_signal = str(rf_receiver.rx_code)  # Convert received RF signal to a string

            if received_signal == "1":  # '1' indicates detumbled
                detumble_signal_count += 1
                print(f"Detumble signal received {detumble_signal_count}/{REQUIRED_CONSECUTIVE_SIGNALS}")
                
                if detumble_signal_count >= REQUIRED_CONSECUTIVE_SIGNALS:
                    print("RSO detumbled. Sending ready signal to ground station.")
                    comms.notify_ground_station(sock, "RSO_READY_FOR_CAPTURE")
                    break  # Exit after confirming detumbled status
            else:
                detumble_signal_count = 0  # Reset count if a different signal is received

        time.sleep(0.1)  # Small delay to avoid overwhelming the CPU

except KeyboardInterrupt:
    comms.send_log("Interrupted by user. Exiting...")

finally:
    cap.release()  # Release the camera
    cv2.destroyAllWindows()  # Close OpenCV windows
    rf_receiver.cleanup()  # Disable RF receiver
    sock.close()
    comms.send_log("Done releasing resources. Script finished.")
