#include <Arduino.h>

#include "PrefsService.h"
#include "WiFiService.h"

PrefsService prefs;
WiFiService wifiSvc;

// Saves the settings from the captive portal to Preferences.
void saveSettingsFromCaptivePortal() {
    prefs.saveSSID(wifiSvc.ssid);
    prefs.savePassword(wifiSvc.password);
    prefs.saveIPAddress(wifiSvc.ipAddress);
    prefs.saveDeviceName(wifiSvc.deviceName);
    prefs.saveMIDIControlNum(wifiSvc.midiControlNum);
    prefs.saveControlType(wifiSvc.controlType);
    prefs.saveGPIOPin(wifiSvc.gpioPin);
    prefs.savePixelCount(wifiSvc.pixelCount);
}

void setup() {
    Serial.begin(115200);  // Start the serial.
    prefs.begin();         // Start the PrefsService.

    // Configure the WiFiService.
    wifiSvc.wifiConfig(prefs.getSSID(), prefs.getPassword(), prefs.getIPAddress(), prefs.getDeviceName());
    wifiSvc.settingsConfig(prefs.getMIDIControlNum(), prefs.getControlType(), prefs.getGPIOPin(), prefs.getPixelCount());

    wifiSvc.begin(&saveSettingsFromCaptivePortal);  // Start the WiFiService.
}

void loop() {}