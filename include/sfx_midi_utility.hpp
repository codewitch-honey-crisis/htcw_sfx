#ifndef HTCW_SFX_MIDI_UTILITY
#define HTCW_SFX_MIDI_UTILITY
#include "sfx_core.hpp"

namespace sfx {
// provides utility methods for MIDI
struct midi_utility {
    // decodes a variable length number
    static const uint8_t* decode_varlen(const uint8_t* in, int32_t* out_value);
    // decodes a variable length number
    static size_t decode_varlen(stream* in, int32_t* out_value);
    // converts a MIDI microtempo to a tempo in beats per minute
    inline static double microtempo_to_tempo(int32_t microtempo) {
        return 60000000 / ((double)microtempo);
    }
    // converts a tempo in beats per minute to a MIDI microtempo
    inline static int32_t tempo_to_microtempo(double tempo) {
        if (tempo != tempo || 0.0 >= tempo) {
            return 0;
        }
        return (int32_t)(500000 * (120.0 / tempo));
    }
    // encodes a variable length integer to a memory buffer
    static uint8_t* encode_varlen(int32_t value, uint8_t* out);
    // encodes a variable length integer to a stream
    static size_t encode_varlen(int32_t value, stream* out);
    // gets the number of microseconds per MIDI tick
    inline static double usec_per_tick(int32_t microtempo, int16_t timebase) {
        return microtempo / (double)timebase;
    }
    // gets the number of seconds per MIDI tick
    inline static double sec_per_tick(int32_t microtempo, int16_t timebase) {
        return usec_per_tick(microtempo, timebase) / 1000000.0;
    }
};
}  // namespace sfx
#endif  // HTCW_SFX_MIDI_UTILITY