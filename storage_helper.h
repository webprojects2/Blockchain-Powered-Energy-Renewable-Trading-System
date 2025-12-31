#ifndef STORAGE_HELPER_H
#define STORAGE_HELPER_H

#include <Preferences.h>

class StorageHelper {
  private:
    Preferences prefs;
  public:
    void begin() {
      prefs.begin("energyData", false);
    }

    void saveEnergy(float totalEnergy) {
      prefs.putFloat("energy_generated", totalEnergy);
      Serial.printf("[Storage] Energy saved: %.3f kWh\n", totalEnergy);
    }

    float loadEnergy() {
      float val = prefs.getFloat("energy_generated", 0.0);
      Serial.printf("[Storage] Loaded energy: %.3f kWh\n", val);
      return val;
    }

    void clear() {
      prefs.clear();
      Serial.println("[Storage] Cleared stored data");
    }
};

#endif
