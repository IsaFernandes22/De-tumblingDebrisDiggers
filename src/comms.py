# Send log to ground station (target ip)
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