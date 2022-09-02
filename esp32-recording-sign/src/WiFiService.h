#ifndef WiFiService_h
#define WiFiService_h

#include <Arduino.h>

#include "helpers.h"

// typedef void (*vFunctionCall)();

// The service that handles the wifi connection and the captive portal where the settings are set.
class WiFiService {
   public:
    WiFiService();
    void wifiConfig(const char* ssid, const char* password, const IPAddress ipAddress, const char* deviceName);
    void settingsConfig(const uint8_t midiControlNum, const ControlType controlType, const uint8_t gpioPin, const uint16_t pixelCount);
    void begin(void (*onSave)());

    const char* ssid;
    const char* password;
    const char* deviceName;
    IPAddress ipAddress;
    uint8_t midiControlNum;
    ControlType controlType;
    uint8_t gpioPin;
    uint16_t pixelCount;

   private:
    void _setupServer();
    void _startServer();

    void (*_onSave)();
};

#endif