# Author: Isa Fernandes
# Date: 04-25-2021
# Description: Test communication.py

import unittest
import cv2
import time
from src import dispatch

class TestDispatch(unittest.TestCase):

    def setUp(self):
        """Load a test video instead of initializing a live camera."""
        self.video_path = "tests/test_videos/Andras-J video.mov"  # Using test videos
        self.cap = cv2.VideoCapture(self.video_path)
        if not self.cap.isOpened():
            self.fail(f"Could not open test video: {self.video_path}")
        self.previous_location = None
        self.previous_time = None

    def test_video_opens(self):
        """Ensure the video file opens correctly."""
        self.assertTrue(self.cap.isOpened())

    def test_processFrame(self):
        """Test if processFrame returns valid velocity and location, considering time."""
        ret, frame = self.cap.read()
        self.assertTrue(ret, "Failed to read a frame from video")

        # First frame, use None for previous location and time
        velocity, location, self.previous_time = dispatch.processFrame(frame, self.previous_location, self.previous_time)

        self.assertIsInstance(velocity, tuple, "Velocity should be a tuple")
        self.assertIsInstance(location, tuple, "Location should be a tuple")

    def test_processFrame_with_time(self):
        """Test if processFrame properly calculates velocity based on time."""
        ret, frame = self.cap.read()
        self.assertTrue(ret, "Failed to read a frame from video")

        # First frame, use None for previous location and time
        velocity, location, self.previous_time = dispatch.processFrame(frame, self.previous_location, self.previous_time)
        
        # Store the previous location for the next frame
        previous_location = location

        # Ensure that velocity is calculated considering the time difference (dt)
        time.sleep(1/30)  # Sleep to simulate time between frames (assuming 30 FPS)

        # Read the next frame
        ret, frame = self.cap.read()
        self.assertTrue(ret, "Failed to read a second frame from video")

        # Process the second frame
        velocity, location, self.previous_time = dispatch.processFrame(frame, previous_location, self.previous_time)

        # Assert velocity is a tuple and it's not zero (indicating motion was detected)
        self.assertIsInstance(velocity, tuple, "Velocity should be a tuple")
        self.assertNotEqual(velocity, (0, 0), "Velocity should not be (0, 0) if there was motion between frames")

    def test_processFrame_values(self):
        """Ensure that the detected location is within frame bounds and velocity is non-zero."""
        ret, frame = self.cap.read()
        self.assertTrue(ret, "Failed to read a frame from video")

        # First frame, use None for previous location and time
        velocity, location, self.previous_time = dispatch.processFrame(frame, self.previous_location, self.previous_time)

        height, width, _ = frame.shape
        self.assertGreaterEqual(location[0], 0, "X location should be non-negative")
        self.assertGreaterEqual(location[1], 0, "Y location should be non-negative")
        self.assertLess(location[0], width, "X location should be within frame width")
        self.assertLess(location[1], height, "Y location should be within frame height")
        self.assertNotEqual(velocity, (0, 0), "Velocity should not be (0, 0) if there is motion")

    def tearDown(self):
        """Release the video capture object."""
        self.cap.release()

if __name__ == "__main__":
    unittest.main()