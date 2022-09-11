#include "helpers.h"

// Returns an IPAddress from the given string, e.g. "192.168.4.1" to IPAddress(192,168,4,1).
IPAddress ipAddressFromChar(const char *ipStr) {
    IPAddress newIP;
    sscanf(ipStr, "%d.%d.%d.%d", &newIP[0], &newIP[1], &newIP[2], &newIP[3]);
    return newIP;
}

// Returns a string from the given IPAddress, e.g. IPAddress(192,168,4,1) to "192.168.4.1".
const char *ipAddressToChar(const IPAddress ip) {
    char *newIPStr = (char *)malloc(sizeof(char) * 16);
    sprintf(newIPStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    // Because we can't call free(newIPStr) here, this may lead to memory leaks.

    return newIPStr;
}

// Returns a ControlType from a string.
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

// Returns a string from a ControlType.
const char *controlTypeToChar(const ControlType value) {
    switch (value) {
        case NEOPIXEL_STRIP:
            return "neoPixelStrip";
        case RGB_STRIP:
            return "rgbStrip";
        case LED:
            return "led";
        case RELAY:
            return "relay";
        default:
            return "neoPixelStrip";
    }
}