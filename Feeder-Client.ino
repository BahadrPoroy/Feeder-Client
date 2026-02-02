#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Servo.h>
#include <time.h>
#include "secrets.h"

// --- Hardware Pins ---
const int SERVO_PIN = D2;
const int SWITCH_PIN = D1;  // D1 and GND connection

// --- UDP Settings ---
WiFiUDP udp;
unsigned int INCOMING_PORT = 4210;  //Listening port for Client
unsigned int OUTGOING_PORT = 4211;  //Sending port for Client
int failCounter = 0;
const int MAX_RETRIES = 3;
bool isLocalMode = true;
unsigned long lastLocalCheck = 0;
const unsigned long LOCAL_CHECK_INTERVAL = 300000;  // 5 minutes

// --- Feed Settings ---
Servo feederServo;
bool isFed = false;
const int FEED_HOUR = 13;
const int FEED_MINUTE = 0;
const int STOP_VAL = 90;
const int START_VAL = 120;
const int REVERSE_VAL = 60;
unsigned long previousMillis = 0;
const long interval = 1000;

// --- Time Settings ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600;
struct tm timeinfo;

void setup() {

  ArduinoOTA.setHostname("Feeder-Client");
  ArduinoOTA.setPassword(YOUR_OTA_PASS);
  Serial1.begin(115200);

  WiFi.begin(YOUR_SSID, YOUR_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }
  Serial1.println("\nWiFi Baglandi! IP: " + WiFi.localIP().toString());

  ArduinoOTA.begin();
  udp.begin(INCOMING_PORT);

  configTime(3 * 3600, 0, "pool.ntp.org");

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  feederServo.attach(SERVO_PIN);
  while (digitalRead(SWITCH_PIN) == LOW) {
    feederServo.write(REVERSE_VAL);
    yield();
  }
  feederServo.write(STOP_VAL);  // Servo Initial State

  sendUdpStatus("SISTEM_HAZIR");
}

void sendUdpStatus(String message) {
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255;
  udp.beginPacket(broadcastIP, OUTGOING_PORT);
  udp.write(message.c_str());
  udp.endPacket();
}

void dispensePortion() {
  sendUdpStatus("FEEDING_STARTED");
  feederServo.write(START_VAL);

  delay(2000);  // Wait to clear the switch area

  unsigned long timeout = millis();
  bool success = true;
  for (int i = 0; i <= 1; i++) {
    while (digitalRead(SWITCH_PIN) == LOW) {
      if (millis() - timeout > 50000) {  // 50 second safety timeout
        Serial1.println("ERROR: Switch not triggered!");
        success = false;
        break;
      }
      yield();
    }
  }

  feederServo.write(STOP_VAL);  // Always stop

  if (success) {
    sendUdpStatus("FEED_SUCCESS");  // Master will catch this and update Firebase
  } else {
    sendUdpStatus("FEED_ERROR");  // Master will send you a notification
  }
}

void loop() {
  ArduinoOTA.handle();

  // UDP Instruction Control
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buf[255];
    int len = udp.read(buf, 254);
    if (len > 0) buf[len] = 0;
    String req = String(buf);
    req.trim();

    Serial1.print("Paket yakalandi: ");
    Serial1.println(req);

    // Feeding Control
    if (req == "FEED_NOW") {
      Serial1.println("YEM VERILDI!");
      dispensePortion();
    }
  }
}
