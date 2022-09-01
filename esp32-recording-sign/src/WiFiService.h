#ifndef WiFiService_h
#define WiFiService_h

#include <Arduino.h>

// The different setups for the recording sign.
enum ControlType { NEOPIXEL_STRIP,
                   RGB_STRIP,
                   LED,
                   RELAY };

// The service that handles the wifi connection and the captive portal where the settings are set.
class WiFiService {
   public:
    WiFiService();
    void wifiConfig(const char* ssid, const char* password, IPAddress ipAddress, const char* deviceName);
    void settingsConfig(uint8_t midiControlNum, ControlType controlType, uint8_t gpioPin, uint16_t pixelCount);
    void begin();

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
};

#endif