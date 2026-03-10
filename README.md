# esp32-vision-tracker-_Real-Time Vision-Based Object Tracking using ESP32_

![Python](https://img.shields.io/badge/Python-3.11-blue)
![ESP32](https://img.shields.io/badge/Hardware-ESP32-red)
![Computer Vision](https://img.shields.io/badge/Computer%20Vision-MediaPipe-green)
![AI](https://img.shields.io/badge/AI-Hand%20Landmark%20Detection-orange)
![Communication](https://img.shields.io/badge/Communication-TCP%2FIP-yellow)
![License](https://img.shields.io/badge/License-MIT-lightgrey)

A real-time vision-guided pan–tilt tracking system that integrates **AI-based hand detection with embedded hardware control**. The system uses a laptop running a **MediaPipe hand landmark detection model** to identify hand position from a webcam feed and transmits azimuth and elevation coordinates to an **ESP32 microcontroller** over TCP. The ESP32 then controls two servo motors to physically track the detected hand.

This project demonstrates how **lightweight AI perception and embedded actuation can be integrated into a responsive closed-loop tracking system** without relying on cloud services or heavy onboard computation.

---

# Project Overview

Object tracking plays an important role in many modern applications such as:

- Robotics  
- Surveillance systems  
- Gesture-controlled interfaces  
- Human–computer interaction  
- Smart IoT devices  

Traditional control interfaces such as buttons, joysticks, or switches can be inconvenient, inaccessible for some users, and unsuitable for environments where hygiene or hands-free interaction is important.

This project implements a **vision-driven tracking system** where a user's hand acts as the control input. The detected hand position is translated into **azimuth and elevation angles**, which are transmitted to an ESP32 that drives a **pan–tilt servo mechanism**.

Additional feedback is provided through:

- **OLED display** showing hand quadrant location  
- **LED indicator** when the hand approaches the center of the screen  

The result is a **low-latency, real-time, distributed AI + embedded system**.

---

# System Architecture

The system is composed of two primary modules.

## AI Perception Module (Laptop)

Responsible for:

- Capturing webcam video  
- Detecting the hand using MediaPipe  
- Computing angular offsets  
- Sending data via TCP  

## Embedded Control Module (ESP32)

Responsible for:

- Receiving TCP data  
- Parsing azimuth and elevation values  
- Controlling servo motors  
- Updating OLED display  
- Driving LED feedback  

---

# Architecture Diagram

*(Insert system architecture diagram here)*

Example architecture:

```
Webcam
   │
   ▼
MediaPipe Hand Detection (Laptop)
   │
   ▼
Azimuth / Elevation Calculation
   │
   ▼
TCP Client (Laptop)
   │
WiFi Communication
   │
TCP Server (ESP32)
   │
   ▼
Servo Control (Pan-Tilt)
   │
   ├── OLED Display Feedback
   └── LED Center Indicator
```

---

# AI Model

The project uses **Google MediaPipe Hands**, a lightweight deep learning framework capable of detecting hands in real time.

MediaPipe uses a **two-stage neural network pipeline**.

---

## 1. Palm Detection Model (BlazePalm)

The palm detection model identifies the hand region in the image frame.

Characteristics:

- Based on **MobileNetV2 architecture**
- Uses **depthwise separable convolutions**
- Optimized for low-latency inference
- Efficient for CPU execution

Advantages:

- Fast detection  
- Robust to lighting conditions  
- Small model size  
- Suitable for real-time applications  

---

## 2. Hand Landmark Model

After the palm is detected, the landmark model predicts:

**21 three-dimensional hand landmarks**

Each landmark contains:

```
(x, y, z)
```

These represent finger joints, fingertips, and wrist position.

The landmarks form a **skeletal representation of the hand** which allows accurate motion tracking.

*(Insert MediaPipe landmark diagram here)*

---

# Direction Estimation

The **palm base (Landmark 0)** is used as the reference point.

Normalized coordinates are converted into angular deviations:

```
Azimuth   = (x - 0.5) × 60°
Elevation = -(y - 0.5) × 40°
```

These angles represent the horizontal and vertical displacement of the hand relative to the center of the camera frame.

Example transmitted data:

```
-15.2,10.6
```

---

# Communication Framework

Communication between the laptop and ESP32 is implemented using **WiFi and TCP/IP networking**.

### Network Roles

Laptop:

```
TCP Client
```

ESP32:

```
TCP Server
Port: 8080
```

---

# Data Packet Format

Each packet contains two floating point values separated by a comma.

```
<azimuth>,<elevation>\n
```

Example:

```
12.3,-8.5
```

Advantages of this format:

- Simple parsing  
- Low communication overhead  
- Reliable synchronization  

Average communication delay observed during testing:

```
20–30 ms
```

This delay is small enough to maintain real-time responsiveness.

---

# Embedded Control

The ESP32 receives angle data through the TCP socket and converts them into **servo motor commands**.

Servo mapping example:

```cpp
int az_servo = map(az, -30, 30, 35, 145);
int el_servo = map(el, -20, 20, 35, 145);

az_servo = constrain(az_servo, 45, 135);
el_servo = constrain(el_servo, 45, 135);

servoAzimuth.write(az_servo);
servoElevation.write(el_servo);
```

The **ESP32Servo library** simplifies servo motor control.

Unlike DC motors, servo motors contain **internal feedback control**, allowing precise positioning without implementing external PID controllers.

---

# System Feedback

## OLED Display

The OLED screen provides real-time information including:

- Azimuth angle  
- Elevation angle  
- Quadrant location of the hand  
- Motion direction indicator  

## LED Indicator

The LED turns **ON when the detected hand is near the center of the screen**.

Center condition:

```
|azimuth| < 3°
|elevation| < 3°
```

This confirms correct alignment of the tracking mechanism.

---

# Working Principle

The system operates continuously through the following steps:

1. Webcam captures live video  
2. MediaPipe detects the hand  
3. Hand coordinates are extracted  
4. Coordinates are converted into azimuth and elevation angles  
5. Angles are transmitted to the ESP32 via TCP over WiFi  
6. ESP32 parses the received data  
7. Servo motors adjust their positions accordingly  
8. OLED and LED provide visual feedback  
9. The loop repeats continuously for real-time tracking  

This creates a **closed-loop tracking system**.

---

# Hardware Components

- ESP32 Microcontroller  
- 2 × Servo Motors (Pan and Tilt)  
- OLED Display  
- LED Indicator  
- Webcam  
- Laptop / PC  

---

# Software Stack

- Python  
- OpenCV  
- MediaPipe  
- ESP32 Arduino Framework  
- ESP32Servo Library  
- TCP/IP Networking  

---

# Applications

Potential applications include:

- Gesture-controlled robotics  
- Smart surveillance systems  
- Human–computer interaction interfaces  
- Contactless machine control  
- Object-following robots  
- Assistive technologies  

---

# Future Improvements

Possible enhancements include:

- Running TinyML models directly on ESP32-S3  
- Multi-object tracking  
- Gesture recognition  
- Autonomous robotic camera systems  
- Edge AI optimization  

---

# Contributors

This project was collaboratively developed by:

- **Akhilesh R**
- **Navin Pragaash G**
- **Prajna Ranganath**

---

# License

This project is licensed under the **MIT License**.

---

# Acknowledgements

- Google MediaPipe Team  
- ESP32 Development Community  
- OpenCV Contributors  

---
