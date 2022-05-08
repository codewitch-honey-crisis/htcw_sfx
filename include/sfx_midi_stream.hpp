#ifndef HTCW_SFX_MIDI_STREAM_HPP
#define HTCW_SFX_MIDI_STREAM_HPP
#include "sfx_core.hpp"
#include "sfx_midi_core.hpp"
namespace sfx {
// the MIDI event plus an absolute position within the stream
struct midi_stream_event final {
    // the absolute position in MIDI ticks
    unsigned long long absolute;
    // the delta from the last event in MIDI ticks
    int32_t delta;
    // the MIDI message
    midi_message message;
};
// represents a class for fetching MIDI messages out of a stream
class midi_stream final {
public:
    // decode the next event. The contents of the in_out_event should be preserved between calls to this method.
    static const size_t decode_event(bool is_file, stream* in, midi_stream_event* in_out_event);
};
}  // namespace sfx
#endif  // HTCW_SFX_MIDI_STREAM_HPP
