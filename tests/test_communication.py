# Author: Isa Fernandes

import unittest
from src import comms
from src import dispatch
from src import main
from comms import CommunicationHandler

class TestCommunication(unittest.TestCase):

    def setUp(self):
        return super().setUp()
    
    #TODO need to make these test more robust

    # Test logging message
    def test_send_log(self):
        message = "Test log message"
        sock = CommunicationHandler.send_log(message)
        self.assertTrue(sock)
    
    # Test listening to arduino
    def test_listen_to_arduino(self):
        arduino = CommunicationHandler.listen_to_arduino()
        self.assertTrue(arduino)
    
    # Test notifying ground station
    def test_notify_ground_station(self):
        message = "Test message"
        sock = CommunicationHandler.notify_ground_station(message)
        self.assertTrue(sock)