#ifndef BACKEND_HELPER_H
#define BACKEND_HELPER_H

#include <HTTPClient.h>

class BackendHelper {
  private:
    String backendURL;
  public:
    BackendHelper(String url): backendURL(url) {}

    void sendToBackend(String deviceID, float voltage, float current, float power, float energy) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(backendURL);
        http.addHeader("Content-Type", "application/json");

        String payload = "{\"device_id\":\"" + deviceID + "\",\"voltage\":" + String(voltage) +
                         ",\"current\":" + String(current) + ",\"power\":" + String(power) +
                         ",\"energy_generated\":" + String(energy) + "}";

        int code = http.POST(payload);
        if (code > 0)
          Serial.printf("[Backend] Response code: %d\n", code);
        else
          Serial.printf("[Backend] Error: %s\n", http.errorToString(code).c_str());
        http.end();
      }
    }
};

#endif
