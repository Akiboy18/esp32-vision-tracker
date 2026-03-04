import socket
import cv2
import mediapipe as mp
import time

# ---------------------------------------------------------
# 1. ESP32 TCP SETTINGS
# ---------------------------------------------------------
ESP32_IP = "10.15.185.176"   # 🔧 Replace with your ESP32 IP
ESP32_PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    sock.connect((ESP32_IP, ESP32_PORT))
    print(f"✅ Connected to ESP32 at {ESP32_IP}:{ESP32_PORT}")
except Exception as e:
    print(f"⚠️ Could not connect to ESP32: {e}")
    sock = None

# ---------------------------------------------------------
# 2. MEDIAPIPE SETUP
# ---------------------------------------------------------
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils

hands = mp_hands.Hands(
    max_num_hands=1,
    min_detection_confidence=0.6,
    min_tracking_confidence=0.5
)

# ---------------------------------------------------------
# 3. CAMERA SETTINGS
# ---------------------------------------------------------
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("❌ Could not open webcam.")
    exit()

H_FOV = 60  # degrees
V_FOV = 45  # degrees
prev_time = 0

# ---------------------------------------------------------
# 4. TRACKING LOOP
# ---------------------------------------------------------
while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb)

    height, width, _ = frame.shape
    cx_cam, cy_cam = width // 2, height // 2
    azimuth, elevation = 0, 0

    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

            # Use palm center (landmark 9)
            x = int(hand_landmarks.landmark[9].x * width)
            y = int(hand_landmarks.landmark[9].y * height)

            dx = x - cx_cam
            dy = cy_cam - y

            azimuth = (dx / (width / 2)) * (H_FOV / 2)
            elevation = (dy / (height / 2)) * (V_FOV / 2)

            # Draw feedback
            cv2.circle(frame, (x, y), 8, (0, 255, 255), -1)
            cv2.line(frame, (cx_cam, cy_cam), (x, y), (0, 255, 0), 2)

            # Send to ESP32 via TCP
            if sock:
                try:
                    msg = f"{azimuth:.2f},{elevation:.2f}\n"
                    sock.sendall(msg.encode())
                except Exception as e:
                    print(f"⚠️ TCP send failed: {e}")
                    sock = None

            break  # only one hand

    # Calculate FPS
    curr_time = time.time()
    fps = 1 / (curr_time - prev_time) if prev_time else 0
    prev_time = curr_time

    # Overlay text
    cv2.putText(frame, f"FPS: {fps:.1f}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)
    cv2.putText(frame, f"Az: {azimuth:+.2f}°  El: {elevation:+.2f}°", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)

    cv2.imshow("Mediapipe Hand Tracking (TCP → ESP32)", frame)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
if sock:
    sock.close()
