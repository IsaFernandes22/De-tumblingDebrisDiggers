# Author: Isabella Fernandes 
# this is for handling communication to ground station and to the arduino
from src import config
from rpi_rf import RFDevice
import time

class CommunicationHandler:
    # Send log to ground station (target ip), returns void
    def send_log(message, sock):
        """
        Sends a log message to the hardcoded computer.

        Parameters:
            message (str): The log message to send.
        """
        try:
            log_message = f"LOG: {message}"
            sock.sendto(log_message.encode('utf-8'), (config.TARGET_IP, config.TARGET_PORT))
            print(f"Sent log: {log_message}")
        except Exception as e:
            print(f"Failed to send log: {e}")

    # this returns a string of the arduino data it received
    @staticmethod
    def listen_to_arduino():
        """
        Listens for data from the Arduino via 433MHz RF.

        Returns:
            str: The received message.
        """
        rf_receiver = RFDevice(config.RF_GPIO)  # RF GPIO pin is defined in config.py
        rf_receiver.enable_receive()

        try:
            while True:
                if rf_receiver.rx_code:
                    print(f"Received: {rf_receiver.rx_code}")
                    return str(rf_receiver.rx_code)
                time.sleep(0.1)  # to prevent CPU overuse
        except Exception as e:
            print(f"RF receive error: {e}")
        finally:
            rf_receiver.cleanup()

        return None

    # this sens a message to ground station, returns void
    def notify_ground_station(sock, message):
        """
        Sends a message to the ground station.

        Parameters:
            sock (socket.socket): The socket connection.
            message (str): The message to send.
            target_ip (str): The IP address of the target computer.
            target_port (int): The port of the target computer.
        """
        try:
            sock.sendto(message.encode('utf-8'), (config.TARGET_IP, config.TARGET_PORT))
            print(f"Message sent to {config.TARGET_IP}:{config.TARGET_PORT}")
        except Exception as e:
            print(f"Failed to notify ground station: {e}")