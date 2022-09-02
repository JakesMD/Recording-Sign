#ifndef PrefsService_h
#define PrefsService_h

#include <Arduino.h>
#include <Preferences.h>

#include "WiFiService.h"
#include "helpers.h"

// The class that handles getting and saving settings.
class PrefsService {
   public:
    PrefsService();
    void begin();

    const char* getSSID();
    void saveSSID(const char* ssid);

    const char* getPassword();
    void savePassword(const char* password);

    IPAddress getIPAddress();
    void saveIPAddress(const IPAddress ipAddress);

    const char* getDeviceName();
    void saveDeviceName(const char* deviceName);

    uint8_t getMIDIControlNum();
    void saveMIDIControlNum(const uint8_t getMIDIControlNum);

    ControlType getControlType();
    void saveControlType(const ControlType controlType);

    uint8_t getGPIOPin();
    void saveGPIOPin(const uint8_t gpioPin);

    uint16_t getPixelCount();
    void savePixelCount(const uint16_t pixelCount);

   private:
    void _openSpace(const bool readOnly = false);
    void _closeSpace();

    void _reset();

    template <typename T>
    void _printReadLog(const char* key, T value);

    template <typename T>
    void _printWriteLog(const char* key, T value);

    uint8_t _readUInt8(const char* key, const uint8_t defaultValue);
    void _writeUInt8(const char* key, const uint8_t value);
    uint16_t _readUInt16(const char* key, const uint16_t defaultValue);
    void _writeUInt16(const char* key, const uint16_t value);
    const char* _readConstChar(const char* key, const char* defaultValue);
    void _writeConstChar(const char* key, const char* value);

    Preferences _prefs;
};

#endif