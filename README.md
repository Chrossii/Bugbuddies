# BugBuddys – Smart Home mit Gestensteuerung

Berührungslose Steuerung eines Smart-Home-Prototyps per Handgeste. Erkennung über MediaPipe, Kommunikation über MQTT, Hardware auf einem ESP32, Visualisierung über ein Web-Dashboard.

## Systemarchitektur

```
MediaPipe (Gestenerkennung)  →  MQTT-Broker (lokal, Mosquitto)  →  ESP32 (Sensoren/Aktoren)
                                          ↕
                                     Dashboard
```

Alle Komponenten kommunizieren ausschließlich über MQTT. Der Broker läuft lokal auf einem Rechner im selben WLAN wie der ESP32.

## Projektstruktur

| Datei | Funktion |
|---|---|
| `gesture_mqtt_publisher.py` | Erkennt Handgesten per Webcam (MediaPipe), publiziert sie als MQTT-Nachricht |
| `esp32_smarthome_subscriber.ino` | Liest Sensoren aus, steuert Aktoren, reagiert auf MQTT-Nachrichten |
| `dashboard.html` | Web-Dashboard zur Echtzeit-Visualisierung und Steuerung |
| `start_broker.bat` | Startet den lokalen Mosquitto-Broker mit Konsolenausgabe |

## Hardware

ESP32 DevKit V1, folgende Pin-Belegung:

| Bauteil | Pin |
|---|---|
| Taster | D19 |
| LED | D22 |
| DHT22 (Temperatur/Luftfeuchte) | D5 |
| Servo SG90 (Rollo) | D21 |
| PIR-Bewegungsmelder | D4 |

## Gestensteuerung

| Geste | Aktion |
|---|---|
| 🖐️ `open_hand` | Licht AN |
| ✊ `fist` | Licht AUS |
| 👍 `thumbs_up` | Rollo ÖFFNEN |
| ✌️ `peace` | Rollo SCHLIESSEN |

## MQTT-Topic-Struktur

Präfix: `smarthome_teamBugBuddies/`

| Topic | Richtung | Inhalt |
|---|---|---|
| `home/livingroom/light/set` | Dashboard/Gesture → ESP32 | `ON` / `OFF` |
| `home/livingroom/light/state` | ESP32 → Dashboard | `ON` / `OFF` (retained) |
| `home/livingroom/blinds/set` | Dashboard/Gesture → ESP32 | `OPEN` / `CLOSED` |
| `home/livingroom/blinds/state` | ESP32 → Dashboard | `OPEN` / `CLOSED` (retained) |
| `home/kitchen/temperature` | ESP32 → Dashboard | Float, °C (retained) |
| `home/kitchen/humidity` | ESP32 → Dashboard | Float, % (retained) |
| `home/bedroom/pir` | ESP32 → Dashboard | `MOTION` / `CLEAR` (retained) |
| `home/button` | ESP32 → Dashboard | `PRESSED` / `RELEASED` |
| `home/gesture` | MediaPipe → ESP32/Dashboard | Gestenname, z. B. `open_hand` |

## Setup

**Broker (Mosquitto)**
- `mosquitto.conf` braucht zwei Listener: `listener 1883` (für ESP32/Python) und `listener 9001` mit `protocol websockets` (für Dashboard) + `allow_anonymous true`
- Statische IP für den Rechner mit dem Broker einrichten
- Start über `start_broker.bat` (Adminrechte nötig, falls `mosquitto.conf` unter `Program Files` liegt)

**Python-Umgebung**
- Python 3.11 erforderlich (MediaPipe unterstützt aktuell keine neueren Versionen)
- `py -3.11 -m pip install opencv-python mediapipe paho-mqtt`
- Broker-IP und `TOPIC_PREFIX` in `gesture_mqtt_publisher.py` anpassen

**ESP32**
- Libraries: `PubSubClient`, `DHT sensor library`, `Adafruit Unified Sensor`, `ESP32Servo`
- WLAN-SSID, Passwort und Broker-IP im Sketch eintragen
- Gleicher `topic_prefix` wie im Python-Skript

**Dashboard**
- `dashboard.html` im Browser öffnen
- `PREFIX`-Variable im `<script>`-Teil muss exakt mit ESP32 und Python übereinstimmen

⚠️ **Wichtig:** Broker-IP und Topic-Präfix müssen in allen drei Komponenten exakt identisch sein, sonst sehen sich Publisher, Subscriber und Dashboard nicht.

## Team

- **Person A** – MediaPipe, Gestenerkennung, MQTT-Publisher
- **Person B** – ESP32, Sensoren, Aktoren, MQTT-Subscriber
- **Gemeinsam** – Dashboard, Tests, Dokumentation, Präsentation

## Status

Abschlusspräsentation: 07.07.2026
