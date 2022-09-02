#include <Arduino.h>

#include "PrefsService.h"
#include "WiFiService.h"

PrefsService prefs;
WiFiService wifiSvc;

void setup() {
    Serial.begin(115200);  // Start the serial.
    prefs.begin();         // Start the PrefsService.

    // Configure the WiFiService.
    wifiSvc.wifiConfig(prefs.getSSID(), prefs.getPassword(), prefs.getIPAddress(), prefs.getDeviceName());
    wifiSvc.settingsConfig(prefs.getMIDIControlNum(), prefs.getControlType(), prefs.getGPIOPin(), prefs.getPixelCount());

    wifiSvc.begin();  // Start the WiFiService.
}

void loop() {}