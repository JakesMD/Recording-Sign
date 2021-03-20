# Recording-Sign
A wifi recording sign that lights up when recording in a DAW (digital audio workstation).

![](images/recording-sign.gif)

## How it works
This uses rtpMIDI (with the help of Tobias Erichsen's [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) and Lathoub's [Arduino AppleMIDI library](https://github.com/lathoub/Arduino-AppleMIDI-Library)) to send MIDI messages from the DAW to an ESP32 over a local wifi network when the recording starts and stops. Based on the message received, the ESP32 lights up a NeoPixel strip in a certain color (with the help of Adafruits's [Arduino NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel)).

## Getting Started
- Visit [medium.com](https://jakesmd.medium.com/how-to-make-a-wifi-recording-sign-for-your-home-studio-f672b03b75f9) or [instructables.com](https://www.instructables.com/Wifi-Recording-Sign-Controlled-by-Your-DAW/) for a detailed tutorial on how to make your own for less than $20.
- Visit [thingiverse.com](https://www.thingiverse.com/thing:4800637) for more info on 3D printing an awesome case for it.
