# Author: Isabella Fernandes
# This is where the dispatch parameters and location for attachment on the RSO
# are calculated before dispatching.
# Assuming camera has 30 FPS, maybe 60 (TODO)

import cv2
import numpy as np
import time  # Import time module
from src import config

def init_camera():
    cap = cv2.VideoCapture(camera_index=config.CAMERA_INDEX)  # this is if we want to test with the camera
    if not cap.isOpened():
        raise Exception("ERROR: Camera unable to be initialized.")
    return cap


# This is where we process the frame from the camera and detect where the RSO is
# Should return a set of velocity and location
# Uses OpenCV to detect the frame of the RSO and the middle where mite should be placed
# Key assumptions: camera feed has no noise (duh space), RSO has distinguishable shape
def processFrame(frame, previous_location=None, previous_time=None):
    """
    Processes the camera frame to detect the RSO and calculate its location and velocity.

    Parameters:
        frame (numpy.ndarray): The current frame from the camera.
        previous_location (tuple): The (x, y) location of the RSO in the previous frame.
        previous_time (float): The timestamp of the previous frame for time difference calculation.

    Returns:
        tuple: (velocity, location) where:
            - velocity is a tuple (vx, vy) representing velocity in x and y directions.
            - location is a tuple (x, y) representing the RSO's current location.
    """
    current_time = time.time()

    dt = (current_time - previous_time) if previous_time is not None else 1 / 30  # Default 30 FPS
    
    print(f"Time elapsed: {dt:.6f} seconds")  # Debugging time step

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    _, threshold = cv2.threshold(blurred, 50, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(threshold, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)


    if contours:
        largest_contour = max(contours, key=cv2.contourArea)

        M = cv2.moments(largest_contour)
        if M['m00'] > 0:
            cx = int(M['m10'] / M['m00'])
            cy = int(M['m01'] / M['m00'])
            location = (cx, cy)

            # Draw the selected largest contour in green
            cv2.drawContours(frame, [largest_contour], -1, (0, 255, 0), 2)  

            # Draw a red dot at the center of the detected RSO
            cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)  

            cv2.imshow("Tracked Object", frame)
            cv2.waitKey(500)

            if previous_location:
                vx = (location[0] - previous_location[0]) / dt
                vy = (location[1] - previous_location[1]) / dt

                print(f"Previous Location: {previous_location}, Current Location: {location}")  # Debugging locations
                print(f"Velocity: ({vx:.2f}, {vy:.2f}) pixels/sec")  # Debugging velocity

                velocity = (vx, vy)
            else:
                velocity = (0, 0)

            return velocity, location, current_time

    return None, None, current_time