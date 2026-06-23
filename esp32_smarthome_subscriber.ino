#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ---------- WLAN ----------
const char* ssid = "alcoholusdehydrogenatus";
const char* password = "123456788";

// ---------- MQTT ----------
const char* mqtt_server = "192.168.137.46";
const int mqtt_port = 1883;

const char* topic_prefix = "smarthome_teamBugBuddies/";

String t_light_set    = String(topic_prefix) + "home/livingroom/light/set";
String t_light_state  = String(topic_prefix) + "home/livingroom/light/state";
String t_blinds_set   = String(topic_prefix) + "home/livingroom/blinds/set";
String t_blinds_state = String(topic_prefix) + "home/livingroom/blinds/state";
String t_temperature  = String(topic_prefix) + "home/kitchen/temperature";
String t_humidity     = String(topic_prefix) + "home/kitchen/humidity";
String t_pir          = String(topic_prefix) + "home/bedroom/pir";
String t_button       = String(topic_prefix) + "home/button";

// ---------- Pins ----------
#define BUTTON_PIN 19
#define LED_PIN    22
#define DHT_PIN    5
#define SERVO_PIN  21
#define PIR_PIN    4
#define DHT_TYPE   DHT22

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHT_TYPE);
Servo blindsServo;

// ---------- STATES ----------
bool lightState = false;
bool blindsOpen = false;

// ---------- BUTTON ----------
bool buttonState = HIGH;
bool lastReading = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Heartbeat
unsigned long lastButtonPublish = 0;
const unsigned long buttonInterval = 1000;

// ---------- PIR ----------
bool lastPirState = false;

// ---------- SENSOR ----------
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 3000;

// =====================================================

void setLight(bool on) {
  lightState = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);

  Serial.print("[LIGHT] ");
  Serial.println(on ? "ON" : "OFF");

  client.publish(t_light_state.c_str(), on ? "ON" : "OFF", true);
}

void setBlinds(bool open) {
  blindsOpen = open;

  blindsServo.write(open ? 90 : 0);

  Serial.print("[BLINDS] ");
  Serial.println(open ? "OPEN" : "CLOSED");

  client.publish(t_blinds_state.c_str(), open ? "OPEN" : "CLOSED", true);
}

// =====================================================

void publishButtonState() {
  client.publish(
    t_button.c_str(),
    buttonState == LOW ? "PRESSED" : "RELEASED",
    true
  );
}

// =====================================================

void setup_wifi() {
  Serial.print("WLAN...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWLAN OK");
  Serial.println(WiFi.localIP());
}

// =====================================================

void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  String t = String(topic);

  if (t == t_light_set) {
    setLight(message == "ON");
  }
  else if (t == t_blinds_set) {
    setBlinds(message == "OPEN");
  }
}

// =====================================================

void reconnect() {
  while (!client.connected()) {

    String clientId = "ESP32-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      client.subscribe(t_light_set.c_str());
      client.subscribe(t_blinds_set.c_str());

      setLight(lightState);
      setBlinds(blindsOpen);
    } else {
      delay(5000);
    }
  }
}

// =====================================================

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  dht.begin();
  blindsServo.attach(SERVO_PIN);
  blindsServo.write(0);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// =====================================================

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ================= BUTTON =================
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;

      Serial.print("[BUTTON] Zustand geändert: ");
      Serial.println(buttonState == LOW ? "PRESSED" : "RELEASED");

      publishButtonState();
    }
  }

  lastReading = reading;

  // HEARTBEAT (1s)
  if (millis() - lastButtonPublish > buttonInterval) {

    lastButtonPublish = millis();

    publishButtonState();

    Serial.print("[BUTTON HEARTBEAT] ");
    Serial.println(buttonState == LOW ? "PRESSED" : "RELEASED");
  }

  // ================= PIR =================
  bool pirState = digitalRead(PIR_PIN);

  if (pirState != lastPirState) {
    client.publish(t_pir.c_str(), pirState ? "MOTION" : "CLEAR", true);
    lastPirState = pirState;
  }

  // ================= DHT =================
  if (millis() - lastSensorRead > sensorInterval) {

    lastSensorRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      client.publish(t_temperature.c_str(), String(t, 1).c_str(), true);
      client.publish(t_humidity.c_str(), String(h, 1).c_str(), true);
    }
  }
}