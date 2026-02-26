#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "secrets.h"

extern void dispensePortion();
extern void Restart();

class NetworkManager {
private:
  FirebaseData firebaseData;
  FirebaseConfig config;
  FirebaseAuth auth;
  WiFiUDP udp;

  //Seperated ports for diffrent purposes
  unsigned int INCOMING_PORT = 4210;  //Listening port for Client
  unsigned int OUTGOING_PORT = 4211;  //Sending port for Client
                                      /**
   * @brief Fetches weather data from OpenWeather API using stream parsing to save RAM
   */

public:

  void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(YOUR_SSID, YOUR_PASS);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(10);
    }
    Serial.println("\nWiFi Connected");

    config.host = YOUR_URL;
    config.signer.tokens.legacy_token = YOUR_DATABASE_SECRET_KEY;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    ArduinoOTA.setHostname("Feeder_Client");
    ArduinoOTA.setPassword(YOUR_OTA_PASS);

    ArduinoOTA.begin();
    udp.begin(INCOMING_PORT);
  }

  void updateFirebase(const String &Status) {
    Firebase.setString(firebaseData, "/FeederStatus/Status", Status);
  }

  void readFirebase(bool &isFed) {
    if (Firebase.getBool(firebaseData, "/NetTime/isFed")) {
      isFed = firebaseData.boolData();
    }
  }

  void broadcastUDP(String message) {
    IPAddress broadcastIP = WiFi.localIP();
    broadcastIP[3] = 255;
    udp.beginPacket(broadcastIP, OUTGOING_PORT);  //Updated
    udp.write(message.c_str());
    udp.endPacket();
  }

  void handleOTA() {
    ArduinoOTA.handle();
  }

  void handleNetwork(const bool &isFed, String &Status, unsigned long &lastPacketTime) {
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
        unsigned long currentTime = millis();
        if (isFed) {
          Status = "SUCCESS";
          broadcastUDP(Status);
        } else if (Status == "ERROR_HARDWARE") {
          Serial1.println("BLOCK: Hardware error persistent. Manual reset required.");
          broadcastUDP(Status);
        } else if (Status == "PENDING" || Status == "SUCCESS" || (currentTime - lastPacketTime < 2000)) {
          Serial1.print("Protection Triggered: Skipping duplicate/invalid request. Status: ");
          Serial1.println(Status);
          lastPacketTime = currentTime;
        } else {
          // Only turn the servo if the status is "IDLE" or "SW/NETWORK_ERROR"
          lastPacketTime = currentTime;
          dispensePortion();
        }
      }
      if(req == "RESTART"){
        Restart();
      }
    }
  }
};
#endif