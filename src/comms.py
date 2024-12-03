# Author: Isabella Fernandes 
# this is for handling communication to ground station and to the arduino

# Send log to ground station (target ip), returns void
def send_log(message):
    """
    Sends a log message to the hardcoded computer.

    Parameters:
        message (str): The log message to send.
    """
    try:
        log_message = f"LOG: {message}"
        sock.sendto(log_message.encode('utf-8'), (TARGET_IP, TARGET_PORT))
        print(f"Sent log: {log_message}")
    except Exception as e:
        print(f"Failed to send log: {e}")

# this returns a string of the arduino data it received
def listen_to_arduino():

    return 

# this sens a message to ground station, returns void
def notify_ground_station(message):

    return