#include "WiFiService.h"

#include <DNSServer.h>
#include <WiFi.h>

// This stops the code editor from changing the order of the imports.
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "htmlFiles.h"

DNSServer dnsServer;
AsyncWebServer server(80);

// The class that handles requests from the captive portal.
class CaptiveRequestHandler : public AsyncWebHandler {
   public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
        return true;
    }

    // Opens the captive portal when a device connects to the AP.
    void handleRequest(AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", indexHTML);
        Serial.print(F("\n[WIFI] Client connected."));
    }
};

WiFiService::WiFiService() {}

// Sets the wifi credentials, ip address and device name.
void WiFiService::wifiConfig(const char *newSSID, const char *newPassword, const IPAddress newIPAddress, const char *newDeviceName) {
    const char *ssid = newSSID;
    const char *password = newPassword;
    IPAddress ipAddress = newIPAddress;
    const char *deviceName = newDeviceName;
}

// Sets the MIDI control number, control type, GPIO pin and number of pixels on the led strip.
void WiFiService::settingsConfig(const uint8_t newMIDIControlNum, const ControlType newControlType, const uint8_t newGPIOPin, const uint16_t newPixelCount) {
    uint8_t midiControlNum = newMIDIControlNum;
    ControlType controlType = newControlType;
    uint8_t gpioPin = newGPIOPin;
    uint16_t pixelCount = newPixelCount;
}

// Configures the WiFi settings and server callbacks for the captive portal mode.
void WiFiService::_setupServer() {
    Serial.print(F("\n[WIFI] Setting up server..."));

    // Configure the WiFi settings.
    WiFi.mode(WIFI_AP);
    WiFi.softAP(deviceName);
    // The IP address 8.8.8.8 is needed for Android devices to detect the captive portal.
    WiFi.softAPConfig(IPAddress(8, 8, 8, 8), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    // Callback for when a device connects to the IP address via a browser.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", indexHTML);
        Serial.print(F("\n[WIFI] Client connected."));
    });

    // Callback for when a device taps the submit button on the captive portal.
    server.on("/get", HTTP_GET, [&](AsyncWebServerRequest *request) {
        Serial.print(F("\n[WIFI] User has submitted."));
        if (request->hasParam("ssid")) {
            ssid = request->getParam("ssid")->value().c_str();
            Serial.print(F("\n[WIFI] SSID: "));
            Serial.print(ssid);
        }
        if (request->hasParam("password")) {
            password = request->getParam("password")->value().c_str();
            Serial.print(F("\n[WIFI] Password: "));
            Serial.print(password);
        }
        if (request->hasParam("deviceName")) {
            deviceName = request->getParam("deviceName")->value().c_str();
            Serial.print(F("\n[WIFI] Device name: "));
            Serial.print(deviceName);
        }
        if (request->hasParam("ipAddress")) {
            ipAddress = ipAddressFromChar(request->getParam("ipAddress")->value().c_str());
            Serial.print(F("\n[WIFI] IP address: "));
            Serial.print(request->getParam("ipAddress")->value().c_str());
        }
        if (request->hasParam("midiControlNum")) {
            midiControlNum = atoi(request->getParam("midiControlNum")->value().c_str());
            Serial.print(F("\n[WIFI] MIDI control number: "));
            Serial.print(midiControlNum);
        }
        if (request->hasParam("controlType")) {
            controlType = controlTypeFromChar(request->getParam("controlType")->value().c_str());
            Serial.print(F("\n[WIFI] Control type: "));
            Serial.print(request->getParam("controlType")->value().c_str());
        }
        if (request->hasParam("gpioPin")) {
            gpioPin = atoi(request->getParam("gpioPin")->value().c_str());
            Serial.print(F("\n[WIFI] GPIO pin: "));
            Serial.print(gpioPin);
        }
        if (request->hasParam("pixelCount")) {
            pixelCount = strtol(request->getParam("pixelCount")->value().c_str(), NULL, 0);
            Serial.print(F("\n[WIFI] Pixel count: "));
            Serial.print(pixelCount);
        }
        request->send_P(200, "text/html", infoHTML);
        _onSave();
    });

    Serial.print(F("\n[WIFI] Set up server."));
    Serial.print(F("\n[WIFI] AP IP address: "));
    Serial.print(WiFi.softAPIP());
}

// Starts the server in captive portal mode.
void WiFiService::_startServer() {
    Serial.print(F("\n[WIFI] Starting DNS Server..."));

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    server.begin();

    Serial.print(F("\n[WIFI] Started DNS Server."));
}

// Connects to the wifi network or starts the captive portal AP.
void WiFiService::begin(void (*newOnSave)()) {
    _onSave = newOnSave;

    _setupServer();
    _startServer();

    while (true) {
        dnsServer.processNextRequest();
    }
}