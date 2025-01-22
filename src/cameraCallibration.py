# @Author: Isabella Fernandes
# This callibrates the camera used in the prototype. 

import cv2
import os
import numpy as np

# Arrays to store object points and image points from all the given images.
objPoints3D = []
imgPoints2D = []

# termination criteria
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001) # TODO


