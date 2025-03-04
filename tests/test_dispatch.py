#this is for trial testing OpenCV and Supervision for launch params
import unittest
from src import comms
from src import dispatch
from unittest.mock import patch

class TestDispatch(unittest.TestCase):

    def setUp(self):
        self.cap = dispatch.init_camera()
    
    # Test if the camera initializes correctly
    def test_init_camera(self):
        self.assertTrue(self.cap.isOpened())

    # Test if the frame is processed correctly (location and velocity are returned)
    def test_processFrame(self):
        ret, frame = self.cap.read()
        previous_location = None
        dt = 1
        velocity, location = dispatch.processFrame(frame, previous_location, dt)
        self.assertIsInstance(velocity, tuple)
        self.assertIsInstance(location, tuple)

    # Test if location and velocity are reasonable values
    def test_processFrame_values(self):
        ret, frame = self.cap.read()
        previous_location = None
        dt = 1
        velocity, location = dispatch.processFrame(frame, previous_location, dt)

        self.assertGreaterEqual(location[0], 0)  # Location X should be positive
        self.assertGreaterEqual(location[1], 0)  # Location Y should be positive

    # Test if `dispatch` properly handles the processed frame data
    @patch("src.dispatch.send_data")
    def test_dispatch_receives_correct_data(self, mock_send_data):
        ret, frame = self.cap.read()
        previous_location = None
        dt = 1
        velocity, location = dispatch.processFrame(frame, previous_location, dt)
        
        # Call the function that sends data to dispatch
        dispatch.send_data({"velocity": velocity, "location": location})

        # Check if dispatch.send_data was called with correct values
        mock_send_data.assert_called_with({"velocity": velocity, "location": location})

    def tearDown(self):
        self.cap.release()
