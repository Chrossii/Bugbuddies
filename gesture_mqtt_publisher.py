import cv2
import mediapipe as mp
import paho.mqtt.client as mqtt
import time

# --- Konfiguration ---
MQTT_BROKER = "192.168.178.24"   # lokaler broker, z.B. Mosquitto auf lokalen Rechner
MQTT_PORT = 1883

# Muss exakt mit dem Präfix im ESP32-Sketch übereinstimmen, sonst sehen
# sich Publisher und Subscriber auf dem öffentlichen Broker nicht!
TOPIC_PREFIX = "smarthome_teamBugBuddies/"
MQTT_TOPIC = TOPIC_PREFIX + "home/gesture"

PUBLISH_INTERVAL = 0.5      # Sekunden, verhindert Topic-Spam bei gleicher Geste

mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Verbunden mit {MQTT_BROKER}:{MQTT_PORT}")
    else:
        print(f"Verbindung fehlgeschlagen, rc={rc}")


client = mqtt.Client(client_id="gesture-publisher")
client.on_connect = on_connect
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_start()


def count_fingers(hand_landmarks):
    """Gibt eine Liste [Daumen, Zeige-, Mittel-, Ring-, kleiner Finger] zurück,
    1 = gestreckt, 0 = eingeklappt."""
    landmarks = hand_landmarks.landmark
    fingers = []

    # Daumen: seitliche Bewegung statt vertikal, daher x-Vergleich
    if landmarks[4].x < landmarks[3].x:
        fingers.append(1)
    else:
        fingers.append(0)

    # restliche vier Finger: Spitze über mittlerem Gelenk = gestreckt
    tip_ids = [8, 12, 16, 20]
    for tip_id in tip_ids:
        if landmarks[tip_id].y < landmarks[tip_id - 2].y:
            fingers.append(1)
        else:
            fingers.append(0)

    return fingers


def classify_gesture(fingers):
    total = sum(fingers)
    if total == 0:
        return "fist"
    elif total == 5:
        return "open_hand"
    elif fingers == [1, 0, 0, 0, 0]:
        return "thumbs_up"
    elif fingers == [0, 1, 1, 0, 0]:
        return "peace"
    elif fingers == [0, 1, 0, 0, 0]:
        return "pointing"
    else:
        return "unknown"


def main():
    last_gesture = None
    last_publish_time = 0
    cap = cv2.VideoCapture(0)

    with mp_hands.Hands(
        max_num_hands=1,
        min_detection_confidence=0.7,
        min_tracking_confidence=0.7,
    ) as hands:
        while cap.isOpened():
            success, frame = cap.read()
            if not success:
                continue

            frame = cv2.flip(frame, 1)
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = hands.process(rgb_frame)

            gesture = "none"

            if results.multi_hand_landmarks:
                hand_landmarks = results.multi_hand_landmarks[0]
                mp_drawing.draw_landmarks(
                    frame, hand_landmarks, mp_hands.HAND_CONNECTIONS
                )
                fingers = count_fingers(hand_landmarks)
                gesture = classify_gesture(fingers)

            now = time.time()
            if (
                gesture != last_gesture
                and gesture != "none"
                and (now - last_publish_time) > PUBLISH_INTERVAL
            ):
                client.publish(MQTT_TOPIC, gesture)
                print(f"Published to {MQTT_TOPIC}: {gesture}")
                last_gesture = gesture
                last_publish_time = now

            cv2.putText(
                frame, gesture, (10, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3
            )
            cv2.imshow("Gesture Recognition", frame)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                break

    cap.release()
    cv2.destroyAllWindows()
    client.loop_stop()
    client.disconnect()


if __name__ == "__main__":
    main()
