#include "WiFiService.h"

#define SerialMon Serial
#include <AppleMIDI.h>  // Needed for AM_DGB serial.
#include <DNSServer.h>
#include <WiFi.h>

// This stops the code editor from changing the order of the imports.

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "constants/htmlFiles.h"

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
void WiFiService::config(const char *newSSID,
                         const char *newPassword,
                         const IPAddress newIPAddress,
                         const char *newDeviceName,
                         const uint16_t newAPTimeout,
                         const uint16_t newCaptivePortalTimeout,
                         const uint16_t newSTATimeout) {
    ssid = newSSID;
    password = newPassword;
    ipAddress = newIPAddress;
    deviceName = newDeviceName;
    _apTimeout = newAPTimeout;
    _captivePortalTimeout = newCaptivePortalTimeout;
    _staTimeout = newSTATimeout;
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
                AM_DBG(F("[WIFI][TIMER]"), (this->*durationInSecs)() - elapsedSecs, F("seconds left."));
                hasPrintedTime = true;
            }
        } else {
            hasPrintedTime = false;
        }
    }

    if ((this->*isFinished)() == false) {
        AM_DBG(F("[WIFI][TIMER] Timed out."));
    } else {
        AM_DBG(F("[WIFI][TIMER] Stopped."));
    }
}

// ----------------------------------------------------------------------------
// AP mode and captive portal
// ----------------------------------------------------------------------------

// Configures the WiFi settings and server callbacks for the captive portal mode.
void WiFiService::_setupAP() {
    AM_DBG(F("[WIFI][AP] Setting up AP..."));

    // Configure the WiFi settings.
    WiFi.mode(WIFI_AP);
    WiFi.softAP(deviceName);

    // The IP address 8.8.8.8 is needed for Android devices to detect the captive portal.
    WiFi.softAPConfig(IPAddress(8, 8, 8, 8), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    // Prelong the AP timeout when a client connects.
    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
        _apTimeout = _captivePortalTimeout;
        AM_DBG(F("[WIFI][AP] Client connected."));
    },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);

    // Open the index page if the client connects via the browser.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", indexHTML);
        AM_DBG(F("[WIFI][AP] Client connected."));
    });

    // Callback for when a device taps the submit button on the captive portal.
    server.on("/get", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AM_DBG(F("[WIFI][AP] User has submitted."));

        // Fetch the values from the index page.
        if (request->hasParam("ssid")) {
            const char *value = request->getParam("ssid")->value().c_str();
            char *newValue = new char[strlen(value) + 1];
            strcpy(newValue, value);

            ssid = newValue;
            AM_DBG(F("[WIFI][AP] SSID:"), ssid);
        }
        if (request->hasParam("password")) {
            const char *value = request->getParam("password")->value().c_str();
            char *newValue = new char[strlen(value) + 1];
            strcpy(newValue, value);

            password = newValue;
            AM_DBG(F("[WIFI][AP] Password:"), password);
        }
        if (request->hasParam("deviceName")) {
            const char *value = request->getParam("deviceName")->value().c_str();
            char *newValue = new char[strlen(value) + 1];
            strcpy(newValue, value);

            deviceName = newValue;
            AM_DBG(F("[WIFI][AP] Device name:"), deviceName);
        }
        if (request->hasParam("ipAddress")) {
            const char *value = request->getParam("ipAddress")->value().c_str();
            char *newValue = new char[strlen(value) + 1];
            strcpy(newValue, value);

            ipAddress = ipAddressFromChar(newValue);
            AM_DBG(F("[WIFI][AP] IP address:"), ipAddressToChar(ipAddress));
        }
        if (request->hasParam("setupType")) {
            const char *value = request->getParam("setupType")->value().c_str();
            char *newValue = new char[strlen(value) + 1];
            strcpy(newValue, value);

            setupType = setupTypeFromChar(newValue);
            AM_DBG(F("[WIFI][AP] Setup type:"), setupTypeToChar(setupType));
        }
        if (request->hasParam("pixelCount")) {
            const char *value = request->getParam("pixelCount")->value().c_str();
            char *newValue = new char[strlen(value) + 1];
            strcpy(newValue, value);

            pixelCount = strtol(newValue, NULL, 0);
            AM_DBG(F("[WIFI][AP] Pixel count:"), pixelCount);
        }

        // Open the info page.
        request->send_P(200, "text/html", infoHTML);

        // Update the submit status. This will stop the timer.
        _hasSubmitted = true;
    });

    AM_DBG(F("[WIFI][AP] Set up AP."));
    AM_DBG(F("[WIFI][AP] AP IP address:"), WiFi.softAPIP());
}

// Processes an AP request.
void WiFiService::_processAPRequest() {
    dnsServer.processNextRequest();
}

// Starts the captive portal AP.
void WiFiService::_startAP() {
    AM_DBG(F("[WIFI][AP] Starting AP..."));

    dnsServer.start(53, "*", WiFi.softAPIP());

    // Open the index page if the client connects via the captive portal.
    // It is important to call this after the dnsServer has started!
    server.addHandler(new CaptiveRequestHandler).setFilter(ON_AP_FILTER);
    server.begin();

    AM_DBG(F("[WIFI][AP] Started AP."));

    _timer(&WiFiService::_getAPTimeout, &WiFiService::_getHasSubmitted, &WiFiService::_processAPRequest);
}

// Stops the captive portal AP.
void WiFiService::_stopAP() {
    AM_DBG(F("[WIFI][AP] Stopping AP..."));

    server.end();
    dnsServer.stop();
    WiFi.softAPdisconnect();

    AM_DBG(F("[WIFI][AP] Stopped AP."));
}

// ----------------------------------------------------------------------------
// STA mode
// ----------------------------------------------------------------------------

void WiFiService::_startSTA(void (*onDisconnected)()) {
    AM_DBG(F("[WIFI][STA] Starting STA..."));

    // Set the WiFi mode.
    WiFi.mode(WIFI_MODE_STA);

    // Set a custom static IP address.
    WiFi.config(ipAddress, IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    // Start the WiFi.
    WiFi.begin(ssid, password);

    AM_DBG(F("[WIFI][STA] Started STA."));
    AM_DBG(F("[WIFI][STA] Connecting to"), ssid, F("..."));

    _timer(&WiFiService::_getSTATimeout, &WiFiService::_getIsWiFiConnected, &WiFiService::_emptyFunction);

    if (!_getIsWiFiConnected()) {
        AM_DBG(F("[WIFI][STA] Failed to connect to"), ssid, F("."));
    } else {
        AM_DBG(F("[WIFI][STA] Connected to"), ssid, F("."));
        AM_DBG(F("[WIFI] All done!"));

        // Call onDisconnected when disconnected from the WiFi.
        // This must be called after already connected to the WiFi.
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
            AM_DBG(F("[WIFI][STA] Disconnected."));
            onDisconnected();
        },
                     WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    }
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

// Connects to the wifi network or starts the captive portal AP.
void WiFiService::begin(const bool startInAPMode, void (*onSave)(), void (*onFailedToConnect)(), void (*onDisconnected)()) {
    // For starting in AP mode.
    if (startInAPMode) {
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