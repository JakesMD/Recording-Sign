#include <Adafruit_NeoPixel.h>
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
const uint8_t LED_PIN = 25;

// The GPIO pin that the red connector of the RGB LED / strip is wired to.
const uint8_t RGB_RED_PIN = 26;

// The GPIO pin that the green connector of the RGB LED / strip is wired to.
const uint8_t RGB_GREEN_PIN = 27;

// The GPIO pin that the blue connector of the RGB LED / strip is wired to.
const uint8_t RGB_BLUE_PIN = 14;

// The GPIO pin that the relay is wired to.
const uint8_t RELAY_PIN = 12;

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
const uint16_t MIDI_SESSION_TIMOUT = 65;  // in seconds.

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
// Hardware control
// ----------------------------------------------------------------------------

SetupType setupType;
uint16_t pixelCount;

// We have to call this here or we will encounter errors. So I just set the pixel num to 1000 of which not many will be used.
Adafruit_NeoPixel neopixelStrip = Adafruit_NeoPixel(1000, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Whether the LED isn currently on.
bool isLEDOn = false;

// What color the RGB LED / strip is set to.
uint8_t currentRGBRedColor = 0;
uint8_t currentRGBGreenColor = 0;
uint8_t currentRGBBlueColor = 0;

// Fades the LED on or off.
void toggleLED(bool on) {
    if (on != isLEDOn) {
        for (uint8_t i = 0; i < 255; i++) {
            if (on) {
                analogWrite(LED_PIN, i);
            } else {
                analogWrite(LED_PIN, 254 - i);
            }
            delay(1);
        }
    }
    isLEDOn = on;
}

// Fades the RGB LED / strip to the given color.
void setRGBLEDColor(uint8_t red, uint8_t green, uint8_t blue) {
    for (uint8_t i = 0; i < 255; i++) {
        analogWrite(RGB_RED_PIN, map(i, 0, 254, currentRGBRedColor, red));
        analogWrite(RGB_GREEN_PIN, map(i, 0, 254, currentRGBGreenColor, green));
        analogWrite(RGB_BLUE_PIN, map(i, 0, 254, currentRGBBlueColor, blue));
        delay(1);
    }
    currentRGBRedColor = red;
    currentRGBGreenColor = green;
    currentRGBBlueColor = blue;
}

// Wipes the NeoPixel strip in the given color.
void setNeopixelColor(uint8_t red, uint8_t green, uint8_t blue) {
    for (uint16_t i = 0; i < pixelCount; i++) {
        neopixelStrip.setPixelColor(i, red, green, blue);
        neopixelStrip.show();
        delay(15);
    }
}

// Changes to color of the sign.
void setColor(uint8_t red, uint8_t green, uint8_t blue, bool isRecording = false) {
    AM_DBG(F("[MAIN] Setting color"), red, green, blue, F("..."));
    switch (setupType) {
        case LED:
            toggleLED(isRecording);
            break;
        case RGB:
            setRGBLEDColor(red, green, blue);
            break;
        case RELAY:
            digitalWrite(RELAY_PIN, isRecording);
            break;
        case NEOPIXEL:
            setNeopixelColor(red, green, blue);
            break;
    }
    AM_DBG(F("[MAIN] Color set."));
}

// Sets the color, waits for 500 milliseconds and then sets the color to 0.
void flashColor(uint8_t red, uint8_t green, uint8_t blue) {
    setColor(red, green, blue);
    delay(500);
    setColor(0, 0, 0);
}

// Called when the ESP32 goes into AP mode.
void inAPMode() {
    setColor(255, 165, 0);  // Orange
}

// Called when the ESP32 connects to the WiFi.
void wifiConnected() {
    flashColor(0, 0, 255);  // Blue
}

// Called when the ESP32 connects to the MIDI network session.
void midiConnected() {
    flashColor(255, 0, 255);  // Purple
}

// Called when the "start recording" MIDI message is received.
void startRecording() {
    setColor(255, 0, 0, true);  // Red
}

// Called when the "stop recording" MIDI message is received.
void stopRecording() {
    setColor(0, 0, 0);  // Off
}

// ----------------------------------------------------------------------------
// ESP32 power controls
// ----------------------------------------------------------------------------

// Restarts the ESP32.
void restart() {
    flashColor(0, 255, 255);  // Blue-Green

    AM_DBG(F("[ESP] Restarting...\n\n"));
    ESP.restart();
}

// Sends the ESP32 into a very deep sleep to save power. Once the sleep duration is up the ESP32 will restart.
void hibernate() {
    flashColor(255, 255, 0);  // Yellow

    // Turn off unnecessary modules when sleeping.
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

    // Go to sleep and wake up after the sleep duration.
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000);

    AM_DBG(F("[ESP] Going to sleep for"), SLEEP_DURATION, F("seconds...\n\n"));
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
// Initialization
// ----------------------------------------------------------------------------

// This is the first function that is called when the ESP32 restarts.
void setup() {
    // Start the serial (used to print logs to the serial monitor).
    AM_DBG_SETUP(115200);

    // Start the PrefsService.
    prefs.begin();

    // Setup the hardware.
    setupType = prefs.getSetupType();
    pixelCount = prefs.getPixelCount();

    switch (setupType) {
        case LED:
            pinMode(LED_PIN, OUTPUT);
            break;
        case RGB:
            pinMode(RGB_RED_PIN, OUTPUT);
            pinMode(RGB_GREEN_PIN, OUTPUT);
            pinMode(RGB_BLUE_PIN, OUTPUT);
            break;
        case RELAY:
            pinMode(RELAY_PIN, OUTPUT);
            break;
        case NEOPIXEL:
            neopixelStrip.begin();
            neopixelStrip.clear();
            break;
    }

    flashColor(0, 255, 0);  // Green

    // Check if the button is pressed.
    pinMode(START_AP_BTN_PIN, INPUT_PULLUP);

    bool startInAPMode = false;

    if (digitalRead(START_AP_BTN_PIN) == true) {
        startInAPMode = true;

        // Turn the sign on.
        inAPMode();

        AM_DBG(F("[MAIN] Start AP button pressed."));
    } else {
        AM_DBG(F("[MAIN] Start AP button not pressed."));
    }

    // Configure the WiFiService.
    wifiSvc.config(prefs.getSSID(),
                   prefs.getPassword(),
                   prefs.getIPAddress(),
                   prefs.getDeviceName(),
                   AP_TIMEOUT,
                   CAPTIVE_PORTAL_TIMEOUT,
                   STA_TIMEOUT);

    // Start the WiFiService.
    wifiSvc.begin(startInAPMode, &saveSettingsFromCaptivePortal, &hibernate, &restart);

    // Flash the sign.
    wifiConnected();

    // Setup and connect to the MIDI network session.
    midiSvc.setup(prefs.getDeviceName(), MIDI_ON_VALUE, &midiConnected, &restart, &startRecording, &stopRecording);
    midiSvc.begin(MIDI_SESSION_TIMOUT, DAW_RESPONSE_TIMEOUT, &hibernate, &hibernate);
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    midiSvc.run(PING_INTERVAL, DAW_RESPONSE_TIMEOUT, &hibernate);
}