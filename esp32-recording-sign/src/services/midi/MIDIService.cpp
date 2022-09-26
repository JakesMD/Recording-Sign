#include "MIDIService.h"

#define SerialMon Serial
#include <AppleMIDI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "helpers/helpers.h"

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// The service that handles connecting to the MIDI network session and handling MIDI messages.
MIDIService::MIDIService() {}

// Configures the MIDI settings.
void MIDIService::setup(const char* deviceName,
                        const uint8_t midiOnValue,
                        void (*onConnected)(),
                        void (*onDisconnected)(),
                        void (*onRecordingStart)(),
                        void (*onRecordingStop)()) {
    AM_DBG(F("[MIDI] Setting up..."));

    // Start the MIDI.
    AppleMIDI.setName(deviceName);
    MIDI.begin(MIDI_CHANNEL_OMNI);

    AM_DBG(F("[MIDI] Address:"), WiFi.localIP(), F("- Port:"), AppleMIDI.getPort(), F("- Name:"), AppleMIDI.getName());

    // Handle connections and messages.

    // Called when the ESP32 connects to a MIDI session.
    AppleMIDI.setHandleConnected(lambda_to_pointer([onConnected, this](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, const char* name) {
        AM_DBG(F("[MIDI] Connected to session:"), ssrc, name);

        // This will stop the connection timeout timer which will return true.
        _isConnected = true;

        // Potentially light up the sign in a certain color.
        onConnected();
    }));

    // Called when the ESP32 disconnects from a MIDI session.
    AppleMIDI.setHandleDisconnected(lambda_to_pointer([onDisconnected](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
        AM_DBG(F("[MIDI] Disonnected from session:"), ssrc);
        AppleMIDI.end();

        // Potentially restart the ESP32.
        onDisconnected();
    }));

    // Called when a control change message is received.
    MIDI.setHandleControlChange(lambda_to_pointer([midiOnValue, onRecordingStart, onRecordingStop](byte channel, byte number, byte value) {
        AM_DBG(F("[MIDI] Received control change: channel"), channel, F("- number"), number, F("- value"), value);

        // Turns lights up the recording sign if correct message is received.
        if (value == midiOnValue) {
            AM_DBG(F("[MIDI] Recording."));
            onRecordingStart();
        } else {
            AM_DBG(F("[MIDI] Stopped recording."));
            onRecordingStop();
        }
    }));

    // Called when a note on message is received.
    MIDI.setHandleNoteOn(lambda_to_pointer([this](byte channel, byte note, byte velocity) {
        AM_DBG(F("[MIDI] Received note on: channel"), channel, F("- note"), note, "- velocity", velocity);

        // This will stop the daw response timer which will return true.
        _isDAWRunning = true;
    }));

    AM_DBG(F("[MIDI] Setup."));
}

// ----------------------------------------------------------------------------
// Timer
// ----------------------------------------------------------------------------

// Returns _isConnected. Required for _timer method.
bool MIDIService::_getIsConnected() {
    return _isConnected;
}

// Returns _isDAWRunning. Required for _timer method.
bool MIDIService::_getIsDAWRunning() {
    return _isDAWRunning;
}

// Returns false. Required for _timer method.
bool MIDIService::_getFalse() {
    return false;
}

// A countdown timer that that runs toDo() in a loop until the time runs out or isFinished is true.
bool MIDIService::_timer(uint16_t durationInSecs, bool (MIDIService::*isFinished)(), void (MIDIService::*toDo)()) {
    uint16_t elapsedSecs = 0;
    uint32_t start = millis();
    bool hasPrintedTime = false;

    while ((this->*isFinished)() == false && elapsedSecs < durationInSecs) {
        (this->*toDo)();
        elapsedSecs = (millis() - start) / 1000;
        if (elapsedSecs % 5 == 0) {
            if (!hasPrintedTime) {
                AM_DBG(F("[MIDI][TIMER]"), durationInSecs - elapsedSecs, F("seconds left."));
                hasPrintedTime = true;
            }
        } else {
            hasPrintedTime = false;
        }
    }

    if ((this->*isFinished)() == false) {
        AM_DBG(F("[MIDI][TIMER] Timed out."));
        return false;
    }
    AM_DBG(F("[MIDI][TIMER] Stopped."));
    return true;
}

// ----------------------------------------------------------------------------
// Begin
// ----------------------------------------------------------------------------

// Waits for the ESP32 to connect to the MIDI session and then pings the DAW to check that it is running.
void MIDIService::begin(uint16_t sessionTimeout, uint16_t dawResponseTimeout, void (*onFailedToConnect)(), void (*onDAWNotResponding)()) {
    bool result;

    // Wait for the ESP32 to connect to the MIDI session.
    result = _timer(sessionTimeout, &MIDIService::_getIsConnected, &MIDIService::_read);

    // Called if the connection timed out.
    if (result == false) {
        AM_DBG(F("[MIDI] Failed to connect to session."));
        AppleMIDI.end();

        // Potentially puts the ESP32 to sleep.
        onFailedToConnect();
        return;
    }

    // Delay the ping by 10 seconds.
    AM_DBG(F("[MIDI] Waiting 10 seconds before pinging DAW..."));
    _timer(10, &MIDIService::_getFalse, &MIDIService::_read);

    // Ping the DAW.
    _pingDaw(dawResponseTimeout, onDAWNotResponding);
}

// ----------------------------------------------------------------------------
// DAW Pinging
// ----------------------------------------------------------------------------

// Pings the DAW.
// If the DAW has a MIDI track with the input and output set to "Recording Sign"
// we should receive the same message back.
void MIDIService::_pingDaw(uint16_t dawResponseTimeout, void (*onDAWNotResponding)()) {
    AM_DBG(F("[MIDI][PING] Pinging DAW..."));

    // Update last pinged.
    // Calling this here means it will ping the DAW exactly on the ping interval.
    _lastPinged = millis();

    // Reset _isDawRunning;
    _isDAWRunning = false;

    // Ping the DAW.
    MIDI.sendNoteOn(0, 127, 1);

    // Wait for the message to be received.
    bool result = _timer(dawResponseTimeout, &MIDIService::_getIsDAWRunning, &MIDIService::_read);

    // Called if the ping timed out.
    if (result == false) {
        AM_DBG(F("[MIDI][PING] DAW didn't respond."));
        AppleMIDI.end();

        // Potentially puts the ESP32 to sleep.
        onDAWNotResponding();
        return;
    }

    AM_DBG(F("[MIDI][PING] DAW responded."));
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

// Reads incoming messages.
void MIDIService::_read() {
    MIDI.read();
}

// Run in an infinate loop in main().
void MIDIService::run(uint16_t pingInterval, uint16_t dawResponseTimeout, void (*onDAWNotResponding)()) {
    _read();

    if (((millis() - _lastPinged) / 1000) > pingInterval) {
        _pingDaw(dawResponseTimeout, onDAWNotResponding);
    }
}