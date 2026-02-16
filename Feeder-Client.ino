#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Servo.h>
#include <time.h>
#include "secrets.h"
#include "NetworkManager.h"

// --- Hardware Pins ---
const int SERVO_PIN = D2;
const int SWITCH_PIN = D1;  // D1 and GND connection

Servo feederServo;
NetworkManager netBox;

// --- System Settings ---
String Status = "IDLE";
unsigned long lastPacketTime = 0;

// --- Servo Settings ---
const int STOP_VAL = 90;
const int START_VAL = 120;
const int REVERSE_VAL = 60;
const int SPEED_LIMIT = 115;

// --- Time Settings ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600;

void setup() {

  netBox.begin();
  Serial1.begin(115200);

  configTime(gmtOffset_sec, 0, ntpServer);

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  feederServo.attach(SERVO_PIN);
  while (digitalRead(SWITCH_PIN) == LOW) {
    feederServo.write(REVERSE_VAL);
    yield();
  }
  feederServo.write(STOP_VAL);  // Servo Initial State
  Status = "IDLE";
  netBox.broadcastUDP(Status);
}

void dispensePortion() {
  Status = "PENDING";
  netBox.broadcastUDP(Status);
  feederServo.write(START_VAL);

  delay(2000);  // Wait to clear the switch area

  unsigned long timeout = millis();
  int slowFact = 0;
  
  while (digitalRead(SWITCH_PIN) == LOW) {
    if (millis() - timeout > 50000) {  // 50 second safety timeout
      Serial1.println("ERROR: Switch not triggered!");
      Status = "ERROR_HARDWARE";
      break;
    }
    feederServo.write(START_VAL - slowFact);
    slowFact++;
    if (START_VAL - slowFact <= SPEED_LIMIT) slowFact = START_VAL - SPEED_LIMIT;
    yield();
  }

  feederServo.write(STOP_VAL);  // Always stop

  if (Status == "PENDING") {
    Status = "SUCCESS";
  }

  netBox.broadcastUDP(Status);
  netBox.updateFirebase(Status);
  Serial1.print("Dispense finished with status: ");
  Serial1.println(Status);
}

void loop() {
  netBox.handleOTA();
  netBox.handleNetwork(Status, lastPacketTime);

  if (Status == "SUCCESS" && (millis() - lastPacketTime > 600000)) {
    Status = "IDLE";
    netBox.broadcastUDP("SYSTEM_READY_IDLE");
    Serial1.println("Cooldown finished. System is IDLE again.");
  }
}