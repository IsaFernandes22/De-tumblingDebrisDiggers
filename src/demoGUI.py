import tkinter as tk
from tkinter import Label, Frame
import cv2
from PIL import Image, ImageTk
import time
from rpi_rf import RFDevice
from src import dispatch
import os

# ========== CONFIG ==========
VIDEO_PATH = os.path.join(os.path.dirname(__file__), "../tests/test_videos/adrasJ2.mp4")
VIDEO_PATH = os.path.abspath(VIDEO_PATH)
RF_GPIO_PIN = 17
# ============================

# --- RF Setup ---
rfdevice = RFDevice(RF_GPIO_PIN)
rfdevice.enable_tx()

# --- Video Setup ---
cap = cv2.VideoCapture(VIDEO_PATH)
if not cap.isOpened():
    raise FileNotFoundError(f"Failed to open video: {VIDEO_PATH}")

# --- State ---
previous_location = None
previous_time = None
stopped = False

# --- Tkinter GUI ---
root = tk.Tk()
root.title("De-Tumbling Tracker")

data_frame = Frame(root)
data_frame.pack(pady=10)

velocity_label = Label(data_frame, text="Velocity: Processing...", font=("Helvetica", 16), fg="blue")
velocity_label.pack(side="left", padx=20)

location_label = Label(data_frame, text="Location: Processing...", font=("Helvetica", 16), fg="green")
location_label.pack(side="left", padx=20)

video_label = Label(root)
video_label.pack()

# --- Frame Processing Loop ---
def update_frame():
    global previous_location, previous_time, stopped

    if stopped:
        return  # Do nothing after stopping

    ret, frame = cap.read()
    if not ret:
        print("End of video reached without non-zero velocity.")
        on_close()
        return

    velocity, location, previous_time = dispatch.processFrame(frame, previous_location, previous_time)
    previous_location = location

    vx, vy = velocity
    non_zero = abs(vx) > 0.01 or abs(vy) > 0.01  # small threshold to avoid noise

    # Show frame
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img = Image.fromarray(frame_rgb)
    imgtk = ImageTk.PhotoImage(image=img)
    video_label.imgtk = imgtk
    video_label.configure(image=imgtk)

    # Update GUI
    velocity_label.config(text=f"Velocity: ({vx:.2f}, {vy:.2f})")
    location_label.config(text=f"Location: ({location[0]}, {location[1]})")

    if non_zero:
        stopped = True  # stop future updates
        print("Velocity detected, stopping video.")

        # Send once via RF
        try:
            int_vx = int(vx * 100)
            int_vy = int(vy * 100)
            rfdevice.tx_code(int_vx)
            time.sleep(0.05)
            rfdevice.tx_code(int_vy)
        except:
            pass

        return  # stop here

    root.after(33, update_frame)

# --- Clean Exit ---
def on_close():
    cap.release()
    rfdevice.cleanup()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)

# --- Start GUI ---
update_frame()
root.mainloop()
