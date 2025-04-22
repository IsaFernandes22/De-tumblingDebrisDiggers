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
FRAME_LIMIT = 50
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
frame_counter = 0
last_velocity = (0.0, 0.0)
last_location = (0, 0)

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
    global previous_location, previous_time, frame_counter, last_velocity, last_location

    if frame_counter >= FRAME_LIMIT:
        # Show final values after processing is done
        velocity_label.config(text=f"Velocity: ({last_velocity[0]:.2f}, {last_velocity[1]:.2f})")
        location_label.config(text=f"Location: ({last_location[0]}, {last_location[1]})")
        return  # Stop updating

    ret, frame = cap.read()
    if not ret:
        print("Early end of video.")
        on_close()
        return

    velocity, location, previous_time = dispatch.processFrame(frame, previous_location, previous_time)
    previous_location = location
    frame_counter += 1
    last_velocity = velocity
    last_location = location

    # Display current frame
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img = Image.fromarray(frame_rgb)
    imgtk = ImageTk.PhotoImage(image=img)
    video_label.imgtk = imgtk
    video_label.configure(image=imgtk)

    # (Optional) Send final result to Arduino only once at frame 50
    if frame_counter == FRAME_LIMIT:
        try:
            int_vx = int(velocity[0] * 100)
            int_vy = int(velocity[1] * 100)
            rfdevice.tx_code(int_vx)
            time.sleep(0.05)
            rfdevice.tx_code(int_vy)
        except:
            pass

    # Schedule next frame
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
