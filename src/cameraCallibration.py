# @Author: Isabella Fernandes
# Calibrates the camera using checkerboard images.

import cv2
import os
import numpy as np

# Arrays to store object points and image points
objPoints3D = []
imgPoints2D = []

# Termination criteria for corner refinement
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# Calibration data path
calib_data_path = "../calib_data"
if not os.path.isdir(calib_data_path):
    os.makedirs(calib_data_path)
    print("Directory created:", calib_data_path)

# Define the real-world checkerboard pattern size (9x6 inner corners)
CHECKERBOARD_SIZE = (9, 6)
SQUARE_SIZE = 2.4  # Adjust based on your actual checkerboard

# Prepare object points
obj3D = np.zeros((CHECKERBOARD_SIZE[0] * CHECKERBOARD_SIZE[1], 3), np.float32)
obj3D[:, :2] = np.mgrid[0:CHECKERBOARD_SIZE[0], 0:CHECKERBOARD_SIZE[1]].T.reshape(-1, 2)
obj3D *= SQUARE_SIZE  # Scale the points

# Check if the images directory exists
image_dir_path = "images"
if not os.path.exists(image_dir_path):
    raise Exception(f"ERROR: Image directory '{image_dir_path}' not found.")

files = os.listdir(image_dir_path)  # List all files in the directory

for file in files:
    imagePath = os.path.join(image_dir_path, file)
    image = cv2.imread(imagePath)

    if image is None:
        print(f"ERROR: Cannot read {file}. Skipping...")
        continue

    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # Detect chessboard corners
    ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD_SIZE, None)
    
    if not ret:
        print(f"Chessboard not found in {file}. Skipping...")
        continue

    # Refine corner detection
    corners2 = cv2.cornerSubPix(gray, corners, (3, 3), (-1, -1), criteria)
    objPoints3D.append(obj3D)
    imgPoints2D.append(corners2)

    # Draw detected corners
    cv2.drawChessboardCorners(image, CHECKERBOARD_SIZE, corners2, ret)
    cv2.imshow("Calibration Image", image)
    cv2.waitKey(500)  # Show image for 500ms
    cv2.destroyAllWindows()  # Ensure windows are closed after displaying

# Ensure we have valid points before calibrating
if not objPoints3D or not imgPoints2D:
    print("ERROR: No valid calibration images found. Check 'images/' directory.")
    exit(1)

# Perform camera calibration
ret, mtx, dist, rVecs, tVecs = cv2.calibrateCamera(objPoints3D, imgPoints2D, gray.shape[::-1], None, None)

print("Calibration Successful!")

# Save calibration data
print("Saving calibration data...")
np.savez(f"{calib_data_path}/CalibrationData", matrix=mtx, distCoef=dist, rVector=rVecs, tVector=tVecs)

print("Calibration data saved.")