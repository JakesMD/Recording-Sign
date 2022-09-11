#include <Arduino.h>

#include "PrefsService.h"
#include "WiFiService.h"

// ----------------------------------------------------------------------------
// General settings
// ----------------------------------------------------------------------------

// The GPIO pin that the button for starting in AP mode is wired to.
const uint8_t START_AP_BTN_PIN = 35;

// How long before the AP mode times out when no client connects.
const uint16_t AP_TIMEOUT = 60;  // in seconds

// How much to prelong the AP mode for when a client connects.
const uint16_t CAPTIVE_PORTAL_TIMEOUT = 120;  // in seconds

// How long before connecting to the WiFI times out when not connected.
const uint16_t STA_TIMEOUT = 20;  // in seconds

// How long to sleep for.
const uint64_t SLEEP_DURATION = 300;  // in seconds

// ----------------------------------------------------------------------------
// Definition of the services
// ----------------------------------------------------------------------------

PrefsService prefs;   // Handles storing data on the ESP32.
WiFiService wifiSvc;  // Handles the WiFi connection and captive portal.

// ----------------------------------------------------------------------------
// ESP32 power controls
// ----------------------------------------------------------------------------

// Restarts the ESP32.
void restart() {
    Serial.print(F("\n[ESP] Restarting...\n\n"));
    ESP.restart();
}

// Sends the ESP32 into a very deep sleep to save power. Once the sleep duration is up the ESP32 will restart.
void hibernate() {
    Serial.print(F("\n[ESP] Going to sleep for "));
    Serial.print(SLEEP_DURATION);
    Serial.print(F(" seconds...\n\n"));

    // Turn off unnecessary modules when sleeping.
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

    // Go to sleep and wake up after the sleep duration.
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000);
    esp_deep_sleep_start();
}

// ----------------------------------------------------------------------------
// Saving data
// ----------------------------------------------------------------------------

// Saves the settings from the captive portal to Preferences and restarts the ESP32.
void saveSettingsFromCaptivePortal() {
    // Save the settings.
    prefs.saveSSID(wifiSvc.ssid);
    prefs.savePassword(wifiSvc.password);
    prefs.saveIPAddress(wifiSvc.ipAddress);
    prefs.saveDeviceName(wifiSvc.deviceName);
    prefs.saveMIDIControlNum(wifiSvc.midiControlNum);
    prefs.saveControlType(wifiSvc.controlType);
    prefs.savePixelCount(wifiSvc.pixelCount);

    // Restart the ESP32.
    restart();
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

// This is the first function that is called when the ESP32 restarts.
void setup() {
    // Start the serial (used to print logs to the serial monitor).
    Serial.begin(115200);

    // Start the PrefsService.
    prefs.begin();

    // Configure the WiFiService.
    wifiSvc.wifiConfig(prefs.getSSID(),
                       prefs.getPassword(),
                       prefs.getIPAddress(),
                       prefs.getDeviceName(),
                       START_AP_BTN_PIN,
                       AP_TIMEOUT,
                       CAPTIVE_PORTAL_TIMEOUT,
                       STA_TIMEOUT);
    wifiSvc.settingsConfig(prefs.getMIDIControlNum(), prefs.getControlType(), prefs.getPixelCount());

    // Start the WiFiService.
    wifiSvc.begin(&saveSettingsFromCaptivePortal, &hibernate, &restart);
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {}