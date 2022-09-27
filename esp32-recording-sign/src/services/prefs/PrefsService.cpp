#include "PrefsService.h"

#define SerialMon Serial
#include <AppleMIDI.h>  // Needed for AM_DGB serial.

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

PrefsService::PrefsService() {}

// Initializes the Preferences and resets the space if used for the first time.
void PrefsService::begin() {
    AM_DBG(F("[PREFS] Starting..."));

    Preferences _prefs;
    _reset();

    AM_DBG(F("[PREFS] Started."));
}

// ----------------------------------------------------------------------------
// Opening and closing the space
// ----------------------------------------------------------------------------

// Opens the Preferences space.
void PrefsService::_openSpace(const bool readOnly) {
    _prefs.begin("prefs", readOnly);
}

// Closes the Preferences space.
void PrefsService::_closeSpace() {
    _prefs.end();
}

// ----------------------------------------------------------------------------
// Resetting the space
// ----------------------------------------------------------------------------

// Resets the space to the default values if used for the first time.
void PrefsService::_reset() {
    _openSpace(false);

    if (!_prefs.isKey("ssid")) {
        AM_DBG(F("[PREFS] Empty. Resetting..."));

        _prefs.clear();

        saveSSID("");
        savePassword("");
        saveIPAddress(IPAddress(0, 0, 0, 0));
        saveDeviceName("Recording Sign");
        saveSetupType(NEOPIXEL);
        savePixelCount(16);

        AM_DBG(F("[PREFS] Reset."));
    }

    _closeSpace();
}

// ----------------------------------------------------------------------------
// Printing read and write log
// ----------------------------------------------------------------------------

// Prints a log when reading from the space.
template <typename T>
void PrefsService::_printReadLog(const char* key, T value, const bool obscureValue) {
    if (!obscureValue) {
        AM_DBG(F("[PREFS] Read"), key, F("="), value);
    } else {
        AM_DBG(F("[PREFS] Read"), key, F("="), F("(obscured)"));
    }
}

// Prints a log when writing to the space.
template <typename T>
void PrefsService::_printWriteLog(const char* key, T value, const bool obscureValue) {
    if (!obscureValue) {
        AM_DBG(F("[PREFS] Wrote"), key, F("="), value);
    } else {
        AM_DBG(F("[PREFS] Wrote"), key, F("="), F("(obscured)"));
    }
}

// ----------------------------------------------------------------------------
// Reading and writing to the space
// ----------------------------------------------------------------------------

// Reads a uint8_t from the space.
uint8_t PrefsService::_readUInt8(const char* key, const uint8_t defaultValue) {
    _openSpace(true);
    uint8_t value = _prefs.getUChar(key, defaultValue);

    _printReadLog(key, value);

    _closeSpace();
    return value;
}

// Writes a uint8_t to the space.
void PrefsService::_writeUInt8(const char* key, const uint8_t value) {
    _openSpace(false);
    if (_prefs.getUChar(key) != value) {
        _prefs.putUChar(key, value);
        _printWriteLog(key, value);
    }
    _closeSpace();
}

// Reads a uint16_t from the space.
uint16_t PrefsService::_readUInt16(const char* key, const uint16_t defaultValue) {
    _openSpace(true);
    uint16_t value = _prefs.getUShort(key, defaultValue);

    _printReadLog(key, value);

    _closeSpace();
    return value;
}

// Writes a uint16_t to the space.
void PrefsService::_writeUInt16(const char* key, const uint16_t value) {
    _openSpace(false);
    if (_prefs.getUShort(key) != value) {
        _prefs.putUShort(key, value);
        _printWriteLog(key, value);
    }
    _closeSpace();
}

// Reads a const char* from the space.
const char* PrefsService::_readConstChar(const char* key, const char* defaultValue, const bool obscureValue) {
    _openSpace(true);

    const String valueStr = _prefs.getString(key, defaultValue);
    char* value = new char[valueStr.length() + 1];
    strcpy(value, valueStr.c_str());

    _printReadLog(key, value, obscureValue);

    _closeSpace();
    return value;
}

// Writes a const char* to the space.
void PrefsService::_writeConstChar(const char* key, const char* value, const bool obscureValue) {
    _openSpace(false);

    char* oldValue;

    if (_prefs.isKey(key)) {
        const String oldValueStr = _prefs.getString(key, "null");
        oldValue = new char[oldValueStr.length() + 1];
        strcpy(oldValue, oldValueStr.c_str());
    }

    char* newValue = new char[strlen(value) + 1];
    strcpy(newValue, value);

    if (!_prefs.isKey(key) || oldValue != newValue) {
        _prefs.putString(key, newValue);
        _printWriteLog(key, newValue, obscureValue);
    }

    _closeSpace();
}

// ----------------------------------------------------------------------------
// Fetching and saving settings from and to the space.
// ----------------------------------------------------------------------------

// Reads the ssid from the space.
const char* PrefsService::getSSID() {
    return _readConstChar("ssid", "");
};

// Saves the ssid to the space.
void PrefsService::saveSSID(const char* ssid) {
    if (sizeof(ssid) / sizeof(char) < 50) {
        _writeConstChar("ssid", ssid);
    } else {
        AM_DBG(F("[PREFS][WARNING] Save ssid declined."));
    }
}

// Reads the password from the space.
const char* PrefsService::getPassword() {
    return _readConstChar("password", "");
};

// Writes the password to the space.
void PrefsService::savePassword(const char* password) {
    if (sizeof(password) / sizeof(char) < 50) {
        _writeConstChar("password", password);
    } else {
        AM_DBG(F("[PREFS][WARNING] Save password declined."));
    }
}

// Reads the IP address from the space.
IPAddress PrefsService::getIPAddress() {
    IPAddress newIP;
    newIP[0] = _readUInt8("ipAddress0", 0);
    newIP[1] = _readUInt8("ipAddress1", 1);
    newIP[2] = _readUInt8("ipAddress2", 2);
    newIP[3] = _readUInt8("ipAddress3", 3);
    return newIP;
};

// Writes the IP address to the space.
void PrefsService::saveIPAddress(const IPAddress ipAddress) {
    _writeUInt8("ipAddress0", ipAddress[0]);
    _writeUInt8("ipAddress1", ipAddress[1]);
    _writeUInt8("ipAddress2", ipAddress[2]);
    _writeUInt8("ipAddress3", ipAddress[3]);
}

// Reads the device name from the space.
const char* PrefsService::getDeviceName() {
    return _readConstChar("deviceName", "Recording Sign");
};

// Writes the device name to the space.
void PrefsService::saveDeviceName(const char* deviceName) {
    if (sizeof(deviceName) / sizeof(char) < 30) {
        _writeConstChar("deviceName", deviceName);
    } else {
        AM_DBG(F("[PREFS][WARNING] Save deviceName declined."));
    }
}

// Reads the setup type from the space.
SetupType PrefsService::getSetupType() {
    return setupTypeFromChar(_readConstChar("setupType", "LED"));
};

// Writes the setup type to the space.
void PrefsService::saveSetupType(const SetupType setupType) {
    _writeConstChar("setupType", setupTypeToChar(setupType));
}

// Reads the number of pixels from the space.
uint16_t PrefsService::getPixelCount() {
    return _readUInt16("pixelCount", 16);
};

// Writes the number of pixels to the space.
void PrefsService::savePixelCount(const uint16_t pixelCount) {
    if (pixelCount >= 0 && pixelCount <= 1000) {
        _writeUInt16("pixelCount", pixelCount);
    } else {
        AM_DBG(F("[PREFS][WARNING] Save pixelCount declined."));
    }
}