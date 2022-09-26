#ifndef MIDIService_h
#define MIDIService_h

#include <Arduino.h>

// The service that handles connecting to the MIDI network session and handling MIDI messages.
class MIDIService {
   public:
    MIDIService();

    void setup(const char* deviceName,
               const uint8_t midiOnValue,
               void (*onConnected)(),
               void (*onDisconnected)(),
               void (*onRecordingStart)(),
               void (*onRecordingStop)());
    void begin(uint16_t sessionTimeout, uint16_t dawResponseTimeout, void (*onFailedToConnect)(), void (*onDAWNotResponding)());
    void run(uint16_t pingInterval, uint16_t dawResponseTimeout, void (*onDAWNotResponding)());

   private:
    uint32_t _lastPinged;
    bool _isConnected = false;
    bool _isDAWRunning = false;

    bool _getIsConnected();
    bool _getIsDAWRunning();
    bool _getFalse();

    void _read();
    void _pingDaw(uint16_t dawResponseTimeout, void (*onDAWNotResponding)());
    bool _timer(uint16_t durationInSecs, bool (MIDIService::*isFinished)(), void (MIDIService::*toDo)());
};

#endif