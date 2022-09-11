#ifndef helpers_h
#define helpers_h

#include <Arduino.h>

typedef void (*voidFunctionCall)();
typedef bool (*boolFunctionCall)();

IPAddress ipAddressFromChar(const char *ipStr);
const char *ipAddressToChar(const IPAddress ip);

// The different setups for the recording sign.
enum ControlType { NEOPIXEL_STRIP,
                   RGB_STRIP,
                   LED,
                   RELAY };

ControlType controlTypeFromChar(const char *value);
const char *controlTypeToChar(const ControlType value);

#endif