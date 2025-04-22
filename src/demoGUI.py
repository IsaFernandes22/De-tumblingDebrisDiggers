

import tkinter as tk
from tkinter import Label
import cv2
from PIL import Image, ImageTk
import time
from rpi_rf import RFDevice
from src import dispatch  # Make sure __init__.py exists in src/
import os

# ========== CONFIG ==========
VIDEO_PATH = "/tests/test_videos/adrasJ2.mp4"  # Change if needed
RF_GPIO_PIN = 17  # GPIO pin connected to 433 MHz transmitter
FPS_DELAY = 33    # ~30fps
# ============================

# --- RF Setup ---
rfdevice = RFDevice(RF_GPIO_PIN)
rfdevice.enable_tx()

# --- Video File Setup ---
cap = cv2.VideoCapture(VIDEO_PATH)
if not cap.isOpened():
    print(f"Failed to open video: {VIDEO_PATH}")
    exit()

# --- State Variables ---
previous_location = None
previous_time = None

# --- Tkinter GUI Setup ---
root = tk.Tk()
root.title("De-Tumbling Tracker")

video_label = Label(root)
video_label.pack()

velocity_label = Label(root, text="Velocity: (0.00, 0.00)", font=("Helvetica", 14))
velocity_label.pack()

location_label = Label(root, text="Location: (0, 0)", font=("Helvetica", 14))
location_label.pack()

# --- Frame Update Loop ---
def update_frame():
    global previous_location, previous_time

    ret, frame = cap.read()
    if not ret:
        print("Video finished or failed to read.")
        on_close()
        return

    # Process the frame
    velocity, location, previous_time = dispatch.processFrame(
        frame, previous_location, previous_time
    )
    previous_location = location

    # Update GUI labels
    velocity_label.config(text=f"Velocity: ({velocity[0]:.2f}, {velocity[1]:.2f})")
    location_label.config(text=f"Location: ({location[0]}, {location[1]})")

    # Send over RF
    try:
        int_vx = int(velocity[0] * 100)
        int_vy = int(velocity[1] * 100)
        rfdevice.tx_code(int_vx)
        time.sleep(0.05)
        rfdevice.tx_code(int_vy)
    except Exception as e:
        print(f"RF send error: {e}")

    # Display frame in Tkinter
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img = Image.fromarray(frame_rgb)
    imgtk = ImageTk.PhotoImage(image=img)
    video_label.imgtk = imgtk  # prevent garbage collection
    video_label.configure(image=imgtk)

    # Schedule next frame
    root.after(FPS_DELAY, update_frame)

# --- Clean Shutdown ---
def on_close():
    print("Closing...")
    cap.release()
    rfdevice.cleanup()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)

# --- Start GUI ---
update_frame()
root.mainloop()
