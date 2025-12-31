#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

#include <WiFi.h>

#define WIFI_TIMEOUT_MS 15000
#define WIFI_RETRY_INTERVAL 5000

class WiFiHelper {
  private:
    String ssid, password;
    unsigned long lastAttempt = 0;
  public:
    WiFiHelper(String s, String p): ssid(s), password(p) {}

    void connect() {
      Serial.println("[WiFi] Connecting...");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());

      unsigned long startAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
      }

      if (WiFi.status() == WL_CONNECTED)
        Serial.println("\n[WiFi] Connected to " + ssid);
      else
        Serial.println("\n[WiFi] Connection Failed!");
    }

    void maintain() {
      if (WiFi.status() != WL_CONNECTED && millis() - lastAttempt > WIFI_RETRY_INTERVAL) {
        Serial.println("[WiFi] Reconnecting...");
        lastAttempt = millis();
        connect();
      }
    }

    bool isConnected() {
      return WiFi.status() == WL_CONNECTED;
    }
};

#endif
