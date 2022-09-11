#include "WiFiService.h"

#include <DNSServer.h>
#include <WiFi.h>

// This stops the code editor from changing the order of the imports.
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "htmlFiles.h"

// ----------------------------------------------------------------------------
// Definition of the servers
// ----------------------------------------------------------------------------

DNSServer dnsServer;
AsyncWebServer server(80);

// ----------------------------------------------------------------------------
// Captive portal request handling
// ----------------------------------------------------------------------------

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
    }
};

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

WiFiService::WiFiService() {}

// Sets the settings specifically for the wifi.
void WiFiService::wifiConfig(const char *newSSID,
                             const char *newPassword,
                             const IPAddress newIPAddress,
                             const char *newDeviceName,
                             const uint8_t newStartAPBtnPin,
                             const uint16_t newAPTimeout,
                             const uint16_t newCaptivePortalTimeout,
                             const uint16_t newSTATimeout) {
    ssid = newSSID;
    password = newPassword;
    ipAddress = newIPAddress;
    deviceName = newDeviceName;
    startAPBtnPin = newStartAPBtnPin;
    _apTimeout = newAPTimeout;
    _captivePortalTimeout = newCaptivePortalTimeout;
    _staTimeout = newSTATimeout;
}

// Sets the user settings for the recording sign in general.
void WiFiService::settingsConfig(const uint8_t newMIDIControlNum, const ControlType newControlType, const uint16_t newPixelCount) {
    uint8_t midiControlNum = newMIDIControlNum;
    ControlType controlType = newControlType;
    uint16_t pixelCount = newPixelCount;
}

// ----------------------------------------------------------------------------
// Timer
// ----------------------------------------------------------------------------

// Returns the submit status.
bool WiFiService::_getHasSubmitted() {
    return _hasSubmitted;
}

// Returns true when the WiFi is connected to an AP.
bool WiFiService::_getIsWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// An empty function for when the timer doesn't need to do anything.
void WiFiService::_emptyFunction() {}

// Returns the AP timeout.
uint16_t WiFiService::_getAPTimeout() {
    return _apTimeout;
}

// Returns the STA timeout.
uint16_t WiFiService::_getSTATimeout() {
    return _staTimeout;
}

// A countdown timer that runs toDo in an infinate loop until the time runs out or isFinished is true.
void WiFiService::_timer(uint16_t (WiFiService::*durationInSecs)(), bool (WiFiService::*isFinished)(), void (WiFiService::*toDo)()) {
    uint16_t elapsedSecs = 0;
    uint32_t start = millis();
    bool hasPrintedTime = false;

    while ((this->*isFinished)() == false && elapsedSecs < (this->*durationInSecs)()) {
        (this->*toDo)();

        elapsedSecs = (millis() - start) / 1000;
        if (elapsedSecs % 5 == 0) {
            if (!hasPrintedTime) {
                Serial.print(F("\n[WIFI][TIMER] "));
                Serial.print((this->*durationInSecs)() - elapsedSecs);
                Serial.print(F(" seconds left."));
                hasPrintedTime = true;
            }
        } else {
            hasPrintedTime = false;
        }
    }

    if ((this->*isFinished)() == false) {
        Serial.print(F("\n[WIFI][TIMER] Timed out."));
    } else {
        Serial.print(F("\n[WIFI][TIMER] Stopped."));
    }
}

// ----------------------------------------------------------------------------
// AP mode and captive portal
// ----------------------------------------------------------------------------

// Configures the WiFi settings and server callbacks for the captive portal mode.
void WiFiService::_setupAP() {
    Serial.print(F("\n[WIFI][AP] Setting up AP..."));

    // Configure the WiFi settings.
    WiFi.mode(WIFI_AP);
    WiFi.softAP(deviceName);

    // The IP address 8.8.8.8 is needed for Android devices to detect the captive portal.
    WiFi.softAPConfig(IPAddress(8, 8, 8, 8), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    // Prelong the AP timeout when a client connects.
    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
        _apTimeout = _captivePortalTimeout;
        Serial.print(F("\n[WIFI][AP] Client connected."));
    },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);

    // Open the index page if the client connects via the browser.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", indexHTML);
        Serial.print(F("\n[WIFI][AP] Client connected."));
    });

    // Callback for when a device taps the submit button on the captive portal.
    server.on("/get", HTTP_GET, [&](AsyncWebServerRequest *request) {
        Serial.print(F("\n[WIFI][AP] User has submitted."));

        // Fetch the values from the index page.
        if (request->hasParam("ssid")) {
            ssid = request->getParam("ssid")->value().c_str();
            Serial.print(F("\n[WIFI][AP] SSID: "));
            Serial.print(ssid);
        }
        if (request->hasParam("password")) {
            password = request->getParam("password")->value().c_str();
            Serial.print(F("\n[WIFI][AP] Password: "));
            Serial.print(password);
        }
        if (request->hasParam("deviceName")) {
            deviceName = request->getParam("deviceName")->value().c_str();
            Serial.print(F("\n[WIFI][AP] Device name: "));
            Serial.print(deviceName);
        }
        if (request->hasParam("ipAddress")) {
            ipAddress = ipAddressFromChar(request->getParam("ipAddress")->value().c_str());
            Serial.print(F("\n[WIFI][AP] IP address: "));
            Serial.print(request->getParam("ipAddress")->value().c_str());
        }
        if (request->hasParam("startAPBtnPin")) {
            startAPBtnPin = atoi(request->getParam("startAPBtnPin")->value().c_str());
            Serial.print(F("\n[WIFI][AP] Start AP button pin: "));
            Serial.print(startAPBtnPin);
        }
        if (request->hasParam("midiControlNum")) {
            midiControlNum = atoi(request->getParam("midiControlNum")->value().c_str());
            Serial.print(F("\n[WIFI][AP] MIDI control number: "));
            Serial.print(midiControlNum);
        }
        if (request->hasParam("controlType")) {
            controlType = controlTypeFromChar(request->getParam("controlType")->value().c_str());
            Serial.print(F("\n[WIFI][AP] Control type: "));
            Serial.print(request->getParam("controlType")->value().c_str());
        }
        if (request->hasParam("pixelCount")) {
            pixelCount = strtol(request->getParam("pixelCount")->value().c_str(), NULL, 0);
            Serial.print(F("\n[WIFI][AP] Pixel count: "));
            Serial.print(pixelCount);
        }

        // Open the info page.
        request->send_P(200, "text/html", infoHTML);

        // Update the submit status. This will stop the timer.
        _hasSubmitted = true;
    });

    Serial.print(F("\n[WIFI][AP] Set up AP."));
    Serial.print(F("\n[WIFI][AP] AP IP address: "));
    Serial.print(WiFi.softAPIP());
}

// Processes an AP request.
void WiFiService::_processAPRequest() {
    dnsServer.processNextRequest();
}

// Starts the captive portal AP.
void WiFiService::_startAP() {
    Serial.print(F("\n[WIFI][AP] Starting AP..."));

    dnsServer.start(53, "*", WiFi.softAPIP());

    // Open the index page if the client connects via the captive portal.
    // It is important to call this after the dnsServer has started!
    server.addHandler(new CaptiveRequestHandler).setFilter(ON_AP_FILTER);
    server.begin();

    Serial.print(F("\n[WIFI][AP] Started AP."));

    _timer(&WiFiService::_getAPTimeout, &WiFiService::_getHasSubmitted, &WiFiService::_processAPRequest);
}

// Stops the captive portal AP.
void WiFiService::_stopAP() {
    Serial.print(F("\n[WIFI][AP] Stopping AP..."));

    server.end();
    dnsServer.stop();
    WiFi.softAPdisconnect();

    Serial.print(F("\n[WIFI][AP] Stopped AP."));
}

// ----------------------------------------------------------------------------
// STA mode
// ----------------------------------------------------------------------------

void WiFiService::_startSTA(void (*onDisconnected)()) {
    Serial.print(F("\n[WIFI][STA] Starting STA..."));

    // Set the WiFi mode.
    WiFi.mode(WIFI_MODE_STA);

    // Set a custom static IP address.
    WiFi.config(ipAddress, IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    // Start the WiFi.
    WiFi.begin(ssid, password);

    Serial.print(F("\n[WIFI][STA] Started STA."));
    Serial.print(F("\n[WIFI][STA] Connecting to "));
    Serial.print(ssid);
    Serial.print(F("..."));

    _timer(&WiFiService::_getSTATimeout, &WiFiService::_getIsWiFiConnected, &WiFiService::_emptyFunction);

    if (!_getIsWiFiConnected()) {
        Serial.print(F("\n[WIFI][STA] Failed to connect to "));
        Serial.print(ssid);
        Serial.print(F("."));
    } else {
        Serial.print(F("\n[WIFI][STA] Connected to "));
        Serial.print(ssid);
        Serial.print(F("."));
        Serial.print(F("\n[WIFI] All done!"));

        // Call onDisconnected when disconnected from the WiFi.
        // This must be called after already connected to the WiFi.
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
            Serial.print(F("\n[WIFI][STA] Disconnected."));
            onDisconnected();
        },
                     WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    }
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

// Connects to the wifi network or starts the captive portal AP.
void WiFiService::begin(void (*onSave)(), void (*onFailedToConnect)(), void (*onDisconnected)()) {
    // Setup the button.
    pinMode(startAPBtnPin, INPUT_PULLUP);

    // For starting in AP mode.
    if (ssid == "" || digitalRead(startAPBtnPin) == HIGH) {
        _setupAP();
        _startAP();
        _stopAP();

        if (_hasSubmitted) {
            onSave();
        }
    }

    // For starting in STA mode.
    _startSTA(onDisconnected);

    if (!_getIsWiFiConnected()) {
        onFailedToConnect();
    }
}