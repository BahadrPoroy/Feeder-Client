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
const int L_RED = D3;
const int L_GREEN = D5;
const int L_BLUE = D6;

Servo feederServo;
NetworkManager netBox;

// --- System Settings ---
String Status = "IDLE";
long int lastFirebaseSync = 0;
unsigned long lastPacketTime = 0;
int lastCheckDay = -1;
int lastCheckInterval = 0;
int dailyFeedCount = 10;
int dailyFeedLimit = 1;

// --- Servo Settings ---
const int STOP_VAL = 90;
const int START_VAL = 125;
const int REVERSE_VAL = 60;
const int SPEED_LIMIT = 110;

// --- Time Settings ---
struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600;


void setup() {
  pinMode(L_RED, OUTPUT);
  pinMode(L_GREEN, OUTPUT);
  pinMode(L_BLUE, OUTPUT);
  netBox.begin();
  Serial1.begin(115200);
  Serial.begin(115200);

  configTime(gmtOffset_sec, 0, ntpServer);

  pinMode(SWITCH_PIN, INPUT_PULLUP);

  feederServo.attach(SERVO_PIN);

  Restart();  //Gets the servo to home position

  netBox.readFeedLimit(dailyFeedLimit);
  netBox.readFeedCount(dailyFeedCount);
  if (dailyFeedCount == dailyFeedLimit)
    netBox.broadcastUDP("SUCCESS");
}

void dispensePortion() {
  digitalWrite(L_RED, LOW);
  digitalWrite(L_GREEN, LOW);
  digitalWrite(L_BLUE, HIGH);
  if (dailyFeedCount >= dailyFeedLimit) {
    netBox.broadcastUDP("SUCCESS");
    digitalWrite(L_RED, HIGH);
    digitalWrite(L_GREEN, LOW);
    digitalWrite(L_BLUE, HIGH);
    return;
  }
  Status = "PENDING";
  netBox.broadcastUDP(Status);

  // --- Clearing The Switch Area ---

  unsigned long timeout = millis();

  while (millis() - timeout <= 2000) {
    feederServo.write(START_VAL);
  }
  // --- ~ ~ ~ ---

  if (digitalRead(SWITCH_PIN) == HIGH) {  //Controls the system for any stucks
    feederServo.write(STOP_VAL);
    Status = "ERROR_HARDWARE(Servo/Switch_Error)";
    netBox.broadcastUDP(Status);
    netBox.updateFirebase(Status);
    digitalWrite(L_RED, LOW);
    digitalWrite(L_GREEN, HIGH);
    digitalWrite(L_BLUE, HIGH);
    return;
  }

  timeout = millis();
  int slowFact = 0;

  while (digitalRead(SWITCH_PIN) == LOW) {
    if (millis() - timeout > 10000) {  // 10 second safety timeout
      Serial1.println("ERROR: Switch not triggered!");
      Status = "ERROR_HARDWARE(Possible_Servo_Stuck)";
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
    dailyFeedCount++;
    netBox.updateFirebase(dailyFeedCount);
  }

  netBox.broadcastUDP(Status);
  netBox.updateFirebase(Status);
  Serial1.print("Dispense finished with status: ");
  Serial1.println(Status);
  if (Status.startsWith("ERROR")) {  //Red
    digitalWrite(L_RED, LOW);
    digitalWrite(L_GREEN, HIGH);
    digitalWrite(L_BLUE, HIGH);
  } else if (Status == "SUCCESS") {  // Green
    digitalWrite(L_RED, HIGH);
    digitalWrite(L_GREEN, LOW);
    digitalWrite(L_BLUE, HIGH);
  } else {  // Yellow
    digitalWrite(L_RED, LOW);
    digitalWrite(L_GREEN, LOW);
    digitalWrite(L_BLUE, HIGH);
  }
}

void loop() {
  netBox.handleOTA();
  netBox.handleNetwork(Status, lastPacketTime);
  if (millis() - lastCheckInterval >= 1000) {
    lastCheckInterval = millis();
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
  }
  if (Status == "SUCCESS" && (millis() - lastPacketTime > 600000)) {
    Status = "IDLE";
    digitalWrite(L_RED, HIGH);
    digitalWrite(L_GREEN, LOW);
    digitalWrite(L_BLUE, LOW);
    netBox.broadcastUDP(Status);
    Serial1.println("Cooldown finished. System is IDLE again.");
  }
  if (millis() - lastFirebaseSync >= 20000) {
    lastFirebaseSync = millis();
    netBox.readFeedLimit(dailyFeedLimit);
  }
  if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && lastCheckDay != timeinfo.tm_wday) {
    lastCheckDay = timeinfo.tm_wday;
    Status = "IDLE";
    dailyFeedCount = 0;
    netBox.broadcastUDP(Status);
    netBox.updateFirebase(Status);
    netBox.updateFirebase(dailyFeedCount);
    digitalWrite(L_RED, HIGH);
    digitalWrite(L_GREEN, HIGH);
    digitalWrite(L_BLUE, LOW);
  }
}

void Restart() {
  unsigned long timeout = millis();
  while (digitalRead(SWITCH_PIN) == LOW) {
    feederServo.write(REVERSE_VAL);
    if (millis() - timeout > 15000) {  // 10 second safety timeout
      Serial1.println("ERROR: Switch not triggered!");
      Status = "ERROR_HARDWARE(Possible_Servo_Stuck)";
      break;
    }
    yield();
  }
  feederServo.write(STOP_VAL);  // Servo Initial State
  Status = "IDLE";
  netBox.broadcastUDP(Status);
  netBox.updateFirebase(Status);
  if (Status.startsWith("ERROR")) {
    digitalWrite(L_RED, LOW);
    digitalWrite(L_GREEN, HIGH);
    digitalWrite(L_BLUE, HIGH);
  } else {
    digitalWrite(L_RED, HIGH);
    digitalWrite(L_GREEN, HIGH);
    digitalWrite(L_BLUE, LOW);
  }
}