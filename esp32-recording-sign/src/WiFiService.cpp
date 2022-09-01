#include "WiFiService.h"

#include <DNSServer.h>
#include <WiFi.h>

// This stops the code editor from changing the order of the imports.
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "htmlFiles.cpp"

DNSServer dnsServer;
AsyncWebServer server(80);

// Returns an IPAddress from the given string, e.g. "192.168.4.1" to IPAddress(192,168,2,1).
IPAddress ipAddressFromChar(const char *ipStr) {
    IPAddress newIP;
    sscanf(ipStr, "%d.%d.%d.%d", &newIP[0], &newIP[1], &newIP[2], &newIP[3]);
    return newIP;
}

// Returns a string from the given IPAddress, e.g. IPAddress(192,168,2,1) to "192.168.4.1".
char *ipAddressToChar(IPAddress ip) {
    char *newIpStr;
    sprintf(newIpStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return newIpStr;
}

// Returns a ControlType from a String.
ControlType controlTypeFromChar(const char *value) {
    if (value == "rgbStrip") {
        return RGB_STRIP;
    } else if (value == "led") {
        return LED;
    } else if (value == "relay") {
        return RELAY;
    } else {
        return NEOPIXEL_STRIP;
    }
}

class CaptiveRequestHandler : public AsyncWebHandler {
   public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
        return true;
    }

    // Opens the captive portal when a device connects to the AP.
    void handleRequest(AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
        Serial.print(F("\nClient connected."));
    }
};

WiFiService::WiFiService() {}

// Sets the wifi credentials, ip address and device name.
void WiFiService::wifiConfig(const char *newSSID, const char *newPassword, IPAddress newIPAddress, const char *newDeviceName) {
    const char *ssid = newSSID;
    const char *password = newPassword;
    IPAddress ipAddress = newIPAddress;
    const char *deviceName = newDeviceName;
}

// Sets the MIDI control number, control type, GPIO pin and number of pixels on the led strip.
void WiFiService::settingsConfig(uint8_t newMidiControlNum, ControlType newControlType, uint8_t newGPIOPin, uint16_t newPixelCount) {
    uint8_t midiControlNum = newMidiControlNum;
    ControlType controlType = newControlType;
    uint8_t gpioPin = newGPIOPin;
    uint16_t pixelCount = newPixelCount;
}

// Configures the WiFi settings and server callbacks for the captive portal mode.
void WiFiService::_setupServer() {
    Serial.print(F("\nSetting up the server... "));

    // Configure the WiFi settings.
    WiFi.mode(WIFI_AP);
    WiFi.softAP(deviceName);
    // The IP address 8.8.8.8 is needed for Android devices to detect the captive portal.
    WiFi.softAPConfig(IPAddress(8, 8, 8, 8), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    // Callback for when a device connects to the IP address via a browser.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
        Serial.print(F("\nClient connected."));
    });

    // Callback for when a device taps the submit button on the captive portal.
    server.on("/get", HTTP_GET, [&](AsyncWebServerRequest *request) {
        Serial.print(F("\n\nUser has submitted."));
        if (request->hasParam("ssid")) {
            ssid = request->getParam("ssid")->value().c_str();
            Serial.print(F("\nSSID: "));
            Serial.print(ssid);
        }
        if (request->hasParam("password")) {
            password = request->getParam("password")->value().c_str();
            Serial.print(F("\nPassword: "));
            Serial.print(password);
        }
        if (request->hasParam("deviceName")) {
            deviceName = request->getParam("deviceName")->value().c_str();
            Serial.print(F("\nDevice name: "));
            Serial.print(deviceName);
        }
        if (request->hasParam("ipAddress")) {
            ipAddress = ipAddressFromChar(request->getParam("ipAddress")->value().c_str());
            Serial.print(F("\nIP address: "));
            Serial.print(request->getParam("ipAddress")->value().c_str());
        }
        if (request->hasParam("midiControlNum")) {
            midiControlNum = atoi(request->getParam("midiControlNum")->value().c_str());
            Serial.print(F("\nMIDI control number: "));
            Serial.print(midiControlNum);
        }
        if (request->hasParam("controlType")) {
            controlType = controlTypeFromChar(request->getParam("controlType")->value().c_str());
            Serial.print(F("\nControl type: "));
            Serial.print(request->getParam("controlType")->value().c_str());
        }
        if (request->hasParam("gpioPin")) {
            gpioPin = atoi(request->getParam("gpioPin")->value().c_str());
            Serial.print(F("\nGPIO pin: "));
            Serial.print(gpioPin);
        }
        if (request->hasParam("pixelCount")) {
            pixelCount = atoi(request->getParam("pixelCount")->value().c_str());
            Serial.print(F("\nPixel count: "));
            Serial.print(pixelCount);
        }
        request->send_P(200, "text/html", info_html);
        Serial.print(F("\n"));
    });

    Serial.print(F("done."));
    Serial.print(F("\nAP IP address: "));
    Serial.print(WiFi.softAPIP());
}

// Starts the server in captive portal mode.
void WiFiService::_startServer() {
    Serial.print(F("\nStarting DNS Server... "));

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    server.begin();

    Serial.print(F("done."));
}

// Connects to the wifi network or starts the captive portal AP.
void WiFiService::begin() {
    _setupServer();
    _startServer();

    while (true) {
        dnsServer.processNextRequest();
    }
}