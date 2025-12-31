#include "wifi_helper.h"
#include "energy_sensor.h"
#include "firebase_helper.h"
#include "backend_helper.h"
#include "storage_helper.h"

WiFiHelper wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
EnergySensor sensor;
FirebaseHelper firebase("METER_001", "YOUR_FIREBASE_API_KEY", "device@grid.com", "pass123", "your-project-id");
BackendHelper backend("https://your-django-backend.com/api/update-energy/");
StorageHelper storage;

unsigned long lastSend = 0;
float totalEnergy = 0;

void setup() {
  Serial.begin(115200);
  wifi.connect();
  sensor.begin();
  firebase.begin();
  storage.begin();

  totalEnergy = storage.loadEnergy();
  firebase.listenForTransactions();
}

void loop() {
  wifi.maintain();

  if (millis() - lastSend > 5000) {
    lastSend = millis();

    float v, c, p, e;
    if (sensor.readData(v, c, p, e)) {
      totalEnergy += (p * 5.0 / 3600.0);
      storage.saveEnergy(totalEnergy);

      firebase.sendRealtimeData(v, c, p, totalEnergy);
      backend.sendToBackend("METER_001", v, c, p, totalEnergy);
    }
  }
}
