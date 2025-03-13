# Author: Isa Fernandes
# Date: 03-13-2025
# Description: Test communication.py

import unittest
from unittest.mock import patch, MagicMock
import socket
from src.comms import CommunicationHandler

class TestComms(unittest.TestCase):

    @patch('socket.socket')
    def test_send_log(self, mock_socket):
        """ Test that logs are correctly sent to the ground station. """
        mock_sock = mock_socket.return_value
        CommunicationHandler.send_log("Test log", mock_sock)
        mock_sock.sendto.assert_called_with(b"LOG: Test log", (CommunicationHandler.config.TARGET_IP, CommunicationHandler.config.TARGET_PORT))

    @patch('src.comms.RFDevice')
    def test_listen_to_arduino(self, mock_rfdevice):
        """ Test that RF receiving correctly captures Arduino messages. """
        mock_rf = mock_rfdevice.return_value
        mock_rf.rx_code = "1"  # Simulate receiving signal "1"
        mock_rf.enable_receive.return_value = None

        received_data = CommunicationHandler.listen_to_arduino()
        self.assertEqual(received_data, "1")

    @patch('socket.socket')
    def test_notify_ground_station(self, mock_socket):
        """ Test that ground station notifications are sent correctly. """
        mock_sock = mock_socket.return_value
        CommunicationHandler.notify_ground_station(mock_sock, "RSO detected")
        mock_sock.sendto.assert_called_with(b"RSO detected", (CommunicationHandler.config.TARGET_IP, CommunicationHandler.config.TARGET_PORT))

if __name__ == '__main__':
    unittest.main()
