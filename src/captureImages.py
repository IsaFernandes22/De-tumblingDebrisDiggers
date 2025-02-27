# @Author: Isabella Fernandes
# This file helps the camera callibration by capturing 10 images and saving them to the images directory.
#
# This file is not used in the final implementation of the project.
import cv2
import os

# Ensure the images directory exists
image_dir = "images"
os.makedirs(image_dir, exist_ok=True)

cam = cv2.VideoCapture(0)  # 0 for external USB camera

for i in range(10):  # Capture 10 images
    ret, frame = cam.read()
    if not ret:
        print(f"Failed to grab frame {i}")
        break

    cv2.imshow("Frame", frame)
    cv2.waitKey(500)  # Show frame for 500ms

    img_path = os.path.join(image_dir, f"calib_{i}.jpg")
    cv2.imwrite(img_path, frame)
    print(f"Saved: {img_path}")

cam.release()
cv2.destroyAllWindows()
