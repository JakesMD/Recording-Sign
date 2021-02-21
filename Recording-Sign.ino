#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

#define NUM_LEDS 7 // replace with the number of NeoPixel LEDs you're using.
#define PIN 13

#define CONTROL_NUMBER 1 // replace with the control number this is mapped to in your DAW
#define ON_VALUE 127 // replace with the value when the record button is turned on in your DAW

char ssid[] = "ssid"; // your network SSID (name)
char pass[] = "password";  // your network password (use for WPA, or use as key for WEP)

char midi_name[] = "Recording Sign ESP32"; // replace with any name you fancy

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);


void setup(){
  DBG_SETUP(115200);
  strip.begin();
  

  WiFi.begin(ssid, pass);
  WiFi.setAutoConnect(true);
  
  DBG("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    colorWipe(255,0,255);
    colorWipe(0,0,0);
  }
  DBG("Connected to WiFi.");

  AppleMIDI.setName(midi_name);
  
  DBG("Address:", WiFi.localIP(), "Port:", AppleMIDI.getPort(), "Name:", AppleMIDI.getName());

  MIDI.begin(MIDI_CHANNEL_OMNI);

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    DBG("Connected to session", ssrc, name);
    MIDI.sendControlChange(CONTROL_NUMBER, 0, 1);
    colorWipe(0,0,255);
    colorWipe(0,0,0);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    DBG("Disconnected from session", ssrc);
    colorWipe(0,0,0);
  });
  
  MIDI.setHandleControlChange([](byte channel, byte number, byte value) {
    DBG("Control change - Channel:", channel, "Number:", number, "Value", value);

    if (number==CONTROL_NUMBER) {
      if (value==ON_VALUE) {
        colorWipe(255,0,0);
      } else {
        colorWipe(0,255,0);
      }
    }
  });
}


void loop(){
  MIDI.read();
}

void colorWipe(int red, int green, int blue) {
  for (int i = 0; i<NUM_LEDS/2+1; i++) {
    strip.setPixelColor(i, red, green, blue);
    strip.setPixelColor(NUM_LEDS-i-1, red, green, blue);
    strip.show();
    delay(70);
  }
}
