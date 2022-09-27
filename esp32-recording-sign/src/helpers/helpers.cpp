#include "helpers.h"

// ----------------------------------------------------------------------------
// IPAddress conversion
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
// SetupType conversion
// ----------------------------------------------------------------------------

// Returns a SetupType from a string.
SetupType setupTypeFromChar(const char *value) {
    if (strcmp(value, "NEOPIXEL") == 0) {
        return NEOPIXEL;
    } else if (strcmp(value, "RELAY") == 0) {
        return RELAY;
    } else if (strcmp(value, "RGB") == 0) {
        return RGB;
    } else {
        return LED;
    }
}

// Returns a string from a SetupType.
const char *setupTypeToChar(const SetupType value) {
    switch (value) {
        case LED:
            return "LED";
        case RGB:
            return "RGB";
        case RELAY:
            return "RELAY";
        case NEOPIXEL:
            return "NEOPIXEL";
    }
}