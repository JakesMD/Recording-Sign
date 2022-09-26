#ifndef WiFiService_h
#define WiFiService_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "helpers/helpers.h"

// The service that handles the WiFi connection and the captive portal where the settings are set.
class WiFiService {
   public:
    WiFiService();
    void wifiConfig(const char* ssid,
                    const char* password,
                    const IPAddress ipAddress,
                    const char* deviceName,
                    const uint8_t startAPBtnPin,
                    const uint16_t apTimeout,
                    const uint16_t captivePortalTimeout,
                    const uint16_t staTimeout);

    void settingsConfig(const SetupType setupType, const uint16_t pixelCount);

    void begin(void (*onSave)(), void (*onFailedToConnect)(), void (*onDisconnected)());

    const char* ssid;
    const char* password;
    const char* deviceName;
    IPAddress ipAddress;
    uint8_t startAPBtnPin;
    SetupType setupType;
    uint16_t pixelCount;

   private:
    bool _getHasSubmitted();
    bool _getIsWiFiConnected();
    void _emptyFunction();
    uint16_t _getAPTimeout();
    uint16_t _getSTATimeout();

    void _timer(uint16_t (WiFiService::*durationInSecs)(), bool (WiFiService::*isFinished)(), void (WiFiService::*toDo)());

    void _setupAP();
    void _processAPRequest();
    void _startAP();
    void _stopAP();

    void _startSTA(void (*onDisconnected)());

    bool _hasSubmitted = false;

    uint16_t _apTimeout;
    uint16_t _captivePortalTimeout;
    uint16_t _staTimeout;
};

#endif