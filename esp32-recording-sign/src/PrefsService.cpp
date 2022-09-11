#include "PrefsService.h"

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

PrefsService::PrefsService() {}

// Initializes the Preferences and resets the space if used for the first time.
void PrefsService::begin() {
    Serial.print(F("\n[PREFS] Starting..."));

    Preferences _prefs;
    _reset();

    Serial.print(F("\n[PREFS] Started."));
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
        Serial.print(F("\n[PREFS] Empty. Resetting..."));

        _prefs.clear();

        saveSSID("");
        savePassword("");
        saveIPAddress(IPAddress(0, 0, 0, 0));
        saveDeviceName("Recording Sign");
        saveMIDIControlNum(1);
        saveControlType(NEOPIXEL_STRIP);
        savePixelCount(16);

        Serial.print(F("\n[PREFS] Reset."));
    }

    _closeSpace();
}

// ----------------------------------------------------------------------------
// Printing read and write log
// ----------------------------------------------------------------------------

// Prints a log when reading from the space.
template <typename T>
void PrefsService::_printReadLog(const char* key, T value, const bool obscureValue) {
    Serial.print(F("\n[PREFS] Read "));
    Serial.print(key);
    Serial.print(F(": "));
    if (!obscureValue) {
        Serial.print(value);
    } else {
        Serial.print(F("(obscured)"));
    }
}

// Prints a log when writing to the space.
template <typename T>
void PrefsService::_printWriteLog(const char* key, T value, const bool obscureValue) {
    Serial.print(F("\n[PREFS] Wrote "));
    Serial.print(key);
    Serial.print(F(": "));
    if (!obscureValue) {
        Serial.print(value);
    } else {
        Serial.print(F("(obscured)"));
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
    if (!_prefs.isKey(key) || _prefs.getUChar(key) != value) {
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
    if (!_prefs.isKey(key) || _prefs.getUShort(key) != value) {
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
    if (!_prefs.isKey(key) || _prefs.getString(key).c_str() != value) {
        _prefs.putString(key, value);
        _printWriteLog(key, value, obscureValue);
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
    if (sizeof(ssid) / sizeof(char) < 30) {
        _writeConstChar("ssid", ssid);
    } else {
        Serial.print(F("\n[PREFS][WARNING] Save ssid declined."));
    }
}

// Reads the password from the space.
const char* PrefsService::getPassword() {
    return _readConstChar("password", "", true);
};

// Writes the password to the space.
void PrefsService::savePassword(const char* password) {
    if (sizeof(password) / sizeof(char) < 30) {
        _writeConstChar("password", password, true);
    } else {
        Serial.print(F("\n[PREFS][WARNING] Save password declined."));
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
        Serial.print(F("\n[PREFS][WARNING] Save deviceName declined."));
    }
}

// Reads the MIDI control number from the space.
uint8_t PrefsService::getMIDIControlNum() {
    return _readUInt8("midiControlNum", 1);
};

// Writes the MIDI control number to the space.
void PrefsService::saveMIDIControlNum(const uint8_t midiControlNum) {
    if (midiControlNum >= 0 && midiControlNum <= 127) {
        _writeUInt8("midiControlNum", midiControlNum);
    } else {
        Serial.print(F("\n[PREFS][WARNING] Save midiControlNum declined."));
    }
}

// Reads the control type from the space.
ControlType PrefsService::getControlType() {
    return controlTypeFromChar(_readConstChar("controlType", "neoPixel"));
};

// Writes the control type to the space.
void PrefsService::saveControlType(const ControlType controlType) {
    _writeConstChar("controlType", controlTypeToChar(controlType));
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
        Serial.print(F("\n[PREFS][WARNING] Save pixelCount declined."));
    }
}