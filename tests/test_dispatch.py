#this is for trial testing OpenCV and Supervision for launch params

import unittest
from src import comms
from src import dispatch
from src import main

class TestDispatch(unittest.TestCase):

    def setUp(self):
        return super().setUp()
    
    # Test if the camera is initialized correctly
    def test_init_camera(self):
        cap = comms.init_camera()
        self.assertTrue(cap.isOpened())
        cap.release()
    
     # Test if the frame is processed correctly (location and velocity are returned)
    def test_processFrame(self):
        cap = comms.init_camera()
        ret, frame = cap.read()
        previous_location = None
        dt = 1
        velocity, location = comms.processFrame(frame, previous_location, dt)
        self.assertIsInstance(velocity, tuple)
        self.assertIsInstance(location, tuple)
        cap.release()

    # Test if the calculated location and velocity are correct
    def test_processFrame_values(self):
        cap = comms.init_camera()
        ret, frame = cap.read()
        previous_location = None
        dt = 1
        velocity, location = comms.processFrame(frame, previous_location, dt)
        self.assertGreaterEqual(location[0], 0) # location should be positive
        self.assertGreaterEqual(location[1], 0) # TODO change these values to the actual values
        cap.release()
    
    def tearDown(self):
        return super().tearDown()