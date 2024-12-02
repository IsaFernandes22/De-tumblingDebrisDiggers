# @author: Isabella Fernandes
# This is where the dispatch parameters and location for attachment on the RSO
# are calculated before dispatching.

import cv2

def init_camera():
    cap = cv2.VideoCapture(0) # this uses first camera (only camera connected to pi)
    if not cap.isOpened():
        raise Exception("ERROR: Camera unable to be initialized.")
    return cap


#this is where we process the frame from the camera and detect where the RSO is
#should return a set of velocity and location
def processFrame(frame):

    return