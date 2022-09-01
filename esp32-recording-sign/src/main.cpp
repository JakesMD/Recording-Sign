#include <Arduino.h>

#include "WiFiService.h"

WiFiService wifiSvc;

void setup() {
    Serial.begin(115200);  // Start the serial.

    wifiSvc.wifiConfig("", "", IPAddress(192, 168, 2, 178), "Recording Sign");
    wifiSvc.settingsConfig(5, NEOPIXEL_STRIP, 5, 16);

    wifiSvc.begin();
}

void loop() {}