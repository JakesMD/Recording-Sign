#include <Arduino.h>

#define SerialMon Serial
#include "AppleMIDI.h"  // Needed for AM_DGB serial.
#include "services/midi/MIDIService.h"
#include "services/prefs/PrefsService.h"
#include "services/wifi/WiFiService.h"

// ----------------------------------------------------------------------------
// General settings
// ----------------------------------------------------------------------------

// The GPIO pin that the button for starting in AP mode is wired to.
const uint8_t START_AP_BTN_PIN = 35;

// The GPIO pin that the LED is wired to.
const uint8_t LED_PIN = 13;

// The GPIO pin that the relay is wired to.
const uint8_t RELAY_PIN = 13;

// The GPIO pin that the red connector of the RGB LED is wired to.
const uint8_t LED_R_PIN = 35;

// The GPIO pin that the green connector of the RGB LED is wired to.
const uint8_t LED_G_PIN = 35;

// The GPIO pin that the blue connector of the RGB LED is wired to.
const uint8_t LED_B_PIN = 35;

// The GPIO pin that the data-in connection of the NeoPixel strip is wired to.
const uint8_t NEOPIXEL_PIN = 13;

// How long before the AP mode times out when no client connects.
const uint16_t AP_TIMEOUT = 60;  // in seconds.

// How much to prelong the AP mode for when a client connects.
const uint16_t CAPTIVE_PORTAL_TIMEOUT = 120;  // in seconds.

// How long before connecting to the WiFI times out when not connected.
const uint16_t STA_TIMEOUT = 20;  // in seconds.

// How long to sleep for.
const uint32_t SLEEP_DURATION = 600;  // in seconds.

// The MIDI control change value that will turn the recording sign on.
const uint8_t MIDI_ON_VALUE = 127;  // (0 - 127).

// How long before the connecting to the MIDI session times out when not connected.
const uint16_t MIDI_SESSION_TIMOUT = 90;  // in seconds.

// The duration between pinging the DAW.
const uint16_t PING_INTERVAL = 60;  // in seconds.

// How long before waiting for a response from the DAW times out.
const uint16_t DAW_RESPONSE_TIMEOUT = 5;  // in seconds.

// ----------------------------------------------------------------------------
// Definition of the services
// ----------------------------------------------------------------------------

PrefsService prefs;   // Handles storing data on the ESP32.
WiFiService wifiSvc;  // Handles the WiFi connection and captive portal.
MIDIService midiSvc;  // Handles connecting to the MIDI network session and handling messages.

// ----------------------------------------------------------------------------
// ESP32 power controls
// ----------------------------------------------------------------------------

// Restarts the ESP32.
void restart() {
    AM_DBG(F("[ESP] Restarting...\n\n"));
    ESP.restart();
}

// Sends the ESP32 into a very deep sleep to save power. Once the sleep duration is up the ESP32 will restart.
void hibernate() {
    AM_DBG(F("[ESP] Going to sleep for"), SLEEP_DURATION, F("seconds...\n\n"));

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
    prefs.saveSetupType(wifiSvc.setupType);
    prefs.savePixelCount(wifiSvc.pixelCount);

    // Restart the ESP32.
    restart();
}

// ----------------------------------------------------------------------------
// Control
// ----------------------------------------------------------------------------

SetupType setupType;

// Called when the ESP32 connects to the MIDI network session.
void midiConnected() {}

// Called when the ESP32 disconnects from the MIDI network session.
void midiDisconnected() {
    restart();
}

// Called when the "start recording" MIDI message is received.
void startRecording() {
    switch (setupType) {
        case LED:
            break;
        case RGB_LED:
            break;
        case RELAY:
            break;
        default:
            break;
    }
}

// Called when the "stop recording" MIDI message is received.
void stopRecording() {
    switch (setupType) {
        case LED:
            break;
        case RGB_LED:
            break;
        case RELAY:
            break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

// This is the first function that is called when the ESP32 restarts.
void setup() {
    // Start the serial (used to print logs to the serial monitor).
    AM_DBG_SETUP(115200);

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
    wifiSvc.settingsConfig(prefs.getSetupType(), prefs.getPixelCount());

    // Start the WiFiService.
    wifiSvc.begin(&saveSettingsFromCaptivePortal, &hibernate, &restart);

    // Setup and connect to the MIDI network session.
    midiSvc.setup(prefs.getDeviceName(), MIDI_ON_VALUE, &midiConnected, &midiDisconnected, &startRecording, &stopRecording);
    midiSvc.begin(MIDI_SESSION_TIMOUT, DAW_RESPONSE_TIMEOUT, &hibernate, &hibernate);

    setupType = prefs.getSetupType();

    switch (setupType) {
        case LED:
            break;
        case RGB_LED:
            break;
        case RELAY:
            break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    midiSvc.run(PING_INTERVAL, DAW_RESPONSE_TIMEOUT, &hibernate);
}