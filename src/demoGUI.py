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
FPS_DELAY = 33  # ~30fps
# ============================

# --- RF Setup ---
rfdevice = RFDevice(RF_GPIO_PIN)
rfdevice.enable_tx()

# --- Video File Setup ---
cap = cv2.VideoCapture(VIDEO_PATH)
if not cap.isOpened():
    raise FileNotFoundError(f"Failed to open video: {VIDEO_PATH}")

# --- State ---
previous_location = None
previous_time = None

# --- Tkinter GUI Setup ---
root = tk.Tk()
root.title("De-Tumbling Tracker")

# Create a container for data labels
data_frame = Frame(root)
data_frame.pack(pady=10)

velocity_label = Label(data_frame, text="Velocity: (0.00, 0.00)", font=("Helvetica", 16), fg="blue")
velocity_label.pack(side="left", padx=20)

location_label = Label(data_frame, text="Location: (0, 0)", font=("Helvetica", 16), fg="green")
location_label.pack(side="left", padx=20)

# Video display
video_label = Label(root)
video_label.pack()

# --- Frame Update ---
def update_frame():
    global previous_location, previous_time

    ret, frame = cap.read()
    if not ret:
        on_close()
        return

    # Get processed data
    velocity, location, previous_time = dispatch.processFrame(frame, previous_location, previous_time)
    previous_location = location

    # Update GUI labels
    velocity_label.config(text=f"Velocity: ({velocity[0]:.2f}, {velocity[1]:.2f})")
    location_label.config(text=f"Location: ({location[0]}, {location[1]})")

    # Send data over RF
    try:
        int_vx = int(velocity[0] * 100)
        int_vy = int(velocity[1] * 100)
        rfdevice.tx_code(int_vx)
        time.sleep(0.05)
        rfdevice.tx_code(int_vy)
    except:
        pass  # Fail silently for now

    # Display video frame
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img = Image.fromarray(frame_rgb)
    imgtk = ImageTk.PhotoImage(image=img)
    video_label.imgtk = imgtk  # keep a reference!
    video_label.configure(image=imgtk)

    # Schedule next frame
    root.after(FPS_DELAY, update_frame)

# --- Clean Exit ---
def on_close():
    cap.release()
    rfdevice.cleanup()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)

# --- Start GUI Loop ---
update_frame()
root.mainloop()
