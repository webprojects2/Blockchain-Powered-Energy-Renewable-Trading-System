#ifndef FIREBASE_HELPER_H
#define FIREBASE_HELPER_H

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"

class FirebaseHelper {
  private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    String deviceID;
  public:
    FirebaseHelper(String id, String apiKey, String email, String pass, String projectID): deviceID(id) {
      config.api_key = apiKey;
      auth.user.email = email;
      auth.user.password = pass;
      config.database_url = "https://" + projectID + ".firebaseio.com/";
    }

    void begin() {
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
    }

    void sendRealtimeData(float voltage, float current, float power, float energy) {
      FirebaseJson json;
      json.set("voltage", voltage);
      json.set("current", current);
      json.set("power", power);
      json.set("energy_generated", energy);
      json.set("timestamp", millis());

      String path = "/devices/" + deviceID + "/data";
      if (!Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
        Serial.println("[Firebase] Upload failed: " + fbdo.errorReason());
      } else {
        Serial.println("[Firebase] Data sent successfully");
      }
    }

    void listenForTransactions() {
      Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCallback);
      Firebase.RTDB.beginStream(&fbdo, "/transactions/" + deviceID);
    }

    static void streamCallback(FirebaseStream data) {
      Serial.printf("[Stream] Event: %s | Data: %s\n", data.eventType().c_str(), data.stringData().c_str());
    }

    static void streamTimeoutCallback(bool timeout) {
      if (timeout) Serial.println("[Stream] Timeout, reconnecting...");
    }
};

#endif
