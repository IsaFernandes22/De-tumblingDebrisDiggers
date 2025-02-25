# **De-tumbling Debris Diggers 🚀**  

## 📌 **Overview**  
**De-tumbling Debris Diggers** is a space debris removal system designed to capture and stabilize defunct satellites, specifically targeting **Nimbus 6**. The system relies on vision-based tracking, control moment gyroscopes for stabilization, and efficient communication protocols.  

This project is developed as part of a **senior capstone project at Colorado School of Mines**.  

## 🎯 **Project Goals**  
- **Detect and track space debris** using computer vision.  
- **Estimate velocity and position** for accurate capture.  
- **De-tumble the debris** using a CMG.  
- **Establish reliable communication** between the ground station, Raspberry Pi, and Arduino.  

## 📚 **Project Structure**  
```
📾 De-tumblingDebrisDiggers
 ├📂 images/                # Calibration and vision test images
 ├📂 calib_data/            # Camera calibration results
 ├📄 config.py              # Configuration variables (IP addresses, camera settings, etc.)
 ├📄 comms.py               # Handles communication between the ground station and Arduino
 ├📄 dispatch.py            # Processes frames and estimates debris velocity/location
 ├📄 calibrate.py           # Camera calibration script
 ├📄 requirements.txt       # Dependencies (OpenCV, NumPy, etc.)
 ├📄 README.md              # This file
```

## 🚀 **Getting Started**  

### 1️⃣ **Install Dependencies**  
Ensure you have Python installed, then run:  
```sh
pip install -r requirements.txt
```

### 2️⃣ **Camera Calibration** (One-time setup)  
- Place **9×6 checkerboard images** in `images/`.  
- Run the calibration script:  
  ```sh
  python calibrate.py
  ```
- This will generate `calib_data/CalibrationData.npz` with the camera matrix and distortion coefficients.

### 3️⃣ **Run Object Detection & Tracking**  
To track and estimate debris movement:  
```sh
python dispatch.py
```
This uses OpenCV to detect and analyze debris movement in real time.

### 4️⃣ **Send Commands & Log Data**  
To test communication with the ground station:  
```sh
python comms.py
```

## 🛠 **Technology Stack**  
- **Programming Languages:** Python, C++ (Arduino)  
- **Libraries & Tools:**  
  - OpenCV (Computer Vision)  
  - NumPy (Data Processing)  
  - PySerial (Arduino Communication)  
- **Hardware:**  
  - Raspberry Pi  
  - Arduino  
  - Camera Module  
  - Control Moment Gyroscope  

## 👥 **Team Members**  
- **Isabella Fernandes** (Lead Developer, Communications & Controls)  
- **Quinn Hejmanowski** (Lead Engineer, Controls)
  

## 📌 **Future Improvements**  
- Fine-tuning object tracking for different lighting conditions.  
- Enhancing communication protocols for real-time command execution.  
- Implementing machine learning for better debris detection.  