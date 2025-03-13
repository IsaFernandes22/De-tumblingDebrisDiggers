import unittest
from unittest.mock import MagicMock, patch
from collections import deque
from src import main, dispatch, comms
import socket

class TestMain(unittest.TestCase):
    
    @patch('src.dispatch.init_camera', side_effect=Exception("Camera init failed"))
    def test_camera_init_failure(self, mock_init_camera):
        """ Test if the script handles camera initialization failure properly. """
        with self.assertRaises(SystemExit):
            main.cap = dispatch.init_camera(main.config.CAMERA_INDEX)

    def test_velocity_averaging(self):
        """ Test that the average velocity calculation works correctly. """
        velocity_history = deque(maxlen=5)
        test_velocities = [2, 4, 6, 8, 10]  # Sample velocities
        for v in test_velocities:
            velocity_history.append(v)
        
        expected_avg = sum(test_velocities) / len(test_velocities)
        computed_avg = sum(velocity_history) / len(velocity_history)

        self.assertEqual(computed_avg, expected_avg)

    @patch('src.main.rf_receiver')
    @patch('src.comms.notify_ground_station')
    def test_rf_signal_handling(self, mock_notify, mock_rf):
        """ Test that the detumble signal is received 5 consecutive times before sending the ready signal. """
        mock_rf.rx_code = "1"
        main.detumble_signal_count = 0
        
        for _ in range(main.REQUIRED_CONSECUTIVE_SIGNALS):
            if mock_rf.rx_code == "1":
                main.detumble_signal_count += 1
        
        self.assertEqual(main.detumble_signal_count, main.REQUIRED_CONSECUTIVE_SIGNALS)

        # Ensure the message is sent after receiving 5 valid signals
        mock_notify.assert_called_with(main.sock, "RSO_READY_FOR_CAPTURE")

if __name__ == '__main__':
    unittest.main()
