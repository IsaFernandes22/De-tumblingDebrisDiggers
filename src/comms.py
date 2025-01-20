# Author: Isabella Fernandes 
# this is for handling communication to ground station and to the arduino
import socket
import config

# Send log to ground station (target ip), returns void
def send_log(message):
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
def listen_to_arduino(arduino):
    """
    Listens to the Arduino for a signal.

    Parameters:
        arduino (serial.Serial): The serial connection to the Arduino.

    Returns:
        str: The message received from the Arduino.
    """
    if arduino.in_waiting > 0:
        return arduino.readline().decode('utf-8').strip()
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