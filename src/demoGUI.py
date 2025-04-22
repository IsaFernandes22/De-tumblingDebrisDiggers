import tkinter as tk
from tkinter import Label
import cv2
from PIL import Image, ImageTk
import time
from rpi_rf import RFDevice
from src import dispatch  # Make sure your PYTHONPATH or sys.path allows this
import os

# --- RF Setup ---
rfdevice = RFDevice(17)
rfdevice.enable_tx()

# --- Tracking State ---
previous_location = None
previous_time = None

# --- Camera Setup ---
cap = cv2.VideoCapture(0)  # 0 = default camera

# --- Tkinter GUI Setup ---
root = tk.Tk()
root.title("De-Tumbling Tracker")

video_label = Label(root)
video_label.pack()

velocity_label = Label(root, text="Velocity: (0, 0)")
velocity_label.pack()

location_label = Label(root, text="Location: (0, 0)")
location_label.pack()

# --- Update Function ---
def update_frame():
    global previous_location, previous_time

    ret, frame = cap.read()
    if not ret:
        root.after(10, update_frame)
        return

    velocity, location, previous_time = dispatch.processFrame(frame, previous_location, previous_time)
    previous_location = location

    # Update labels
    velocity_text = f"Velocity: ({velocity[0]:.2f}, {velocity[1]:.2f})"
    location_text = f"Location: ({location[0]}, {location[1]})"

    velocity_label.config(text=velocity_text)
    location_label.config(text=location_text)

    # Send velocity as encoded integers over RF
    try:
        int_velocity_x = int(velocity[0] * 100)
        int_velocity_y = int(velocity[1] * 100)
        rfdevice.tx_code(int_velocity_x)
        time.sleep(0.05)  # slight pause between sends
        rfdevice.tx_code(int_velocity_y)
    except:
        pass

    # Convert frame for Tkinter display
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img = Image.fromarray(frame)
    imgtk = ImageTk.PhotoImage(image=img)
    video_label.imgtk = imgtk
    video_label.configure(image=imgtk)

    # Schedule next frame
    root.after(30, update_frame)

# --- Clean exit ---
def on_close():
    cap.release()
    rfdevice.cleanup()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)

# --- Start loop ---
update_frame()
root.mainloop()
