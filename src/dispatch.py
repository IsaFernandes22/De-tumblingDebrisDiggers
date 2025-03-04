# Author: Isabella Fernandes
# This is where the dispatch parameters and location for attachment on the RSO
# are calculated before dispatching.
# Assuming camera has 30 FPS, maybe 60 (TODO)

import cv2
import numpy as np
from src import config

def init_camera():
    cap = cv2.VideoCapture(camera_index=config.CAMERA_INDEX)
    if not cap.isOpened():
        raise Exception("ERROR: Camera unable to be initialized.")
    return cap


#this is where we process the frame from the camera and detect where the RSO is
#should return a set of velocity and location
# uses open cv to detect the frame of the RSO and the middle where mite should be placed
# key assumptions: camera feed has no noise (duh space), RSO has distinguishable shape
def processFrame(frame, previous_location=None, dt=1):
    """
    Processes the camera frame to detect the RSO and calculate its location and velocity.

    Parameters:
        frame (numpy.ndarray): The current frame from the camera.
        previous_location (tuple): The (x, y) location of the RSO in the previous frame.
        dt (float): Time difference between frames, used for velocity calculation.

    Returns:
        tuple: (velocity, location) where:
            - velocity is a tuple (vx, vy) representing velocity in x and y directions.
            - location is a tuple (x, y) representing the RSO's current location.
    """
    # Convert the frame to grayscale for processing
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Apply GaussianBlur to reduce noise and make object detection more reliable
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)

    # Use thresholding to isolate the RSO (assumes high contrast)
    _, threshold = cv2.threshold(blurred, 50, 255, cv2.THRESH_BINARY)

    # Find contours in the thresholded image
    contours, _ = cv2.findContours(threshold, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Filter the largest contour, assuming it's the RSO
    if contours:
        largest_contour = max(contours, key=cv2.contourArea)
        # Calculate the RSO's centroid
        M = cv2.moments(largest_contour)
        if M['m00'] > 0:
            cx = int(M['m10'] / M['m00'])  # X-coordinate of the centroid
            cy = int(M['m01'] / M['m00'])  # Y-coordinate of the centroid
            location = (cx, cy)

            # Calculate velocity if the previous location is known
            if previous_location:
                vx = (location[0] - previous_location[0]) / dt
                vy = (location[1] - previous_location[1]) / dt
                velocity = (vx, vy)
            else:
                velocity = (0, 0)  # No movement if no previous location

            # Draw the RSO and its center on the frame
            cv2.drawContours(frame, [largest_contour], -1, (0, 255, 0), 2)
            cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)

            return velocity, location

    # If no RSO is detected, return None for velocity and location
    return None, None