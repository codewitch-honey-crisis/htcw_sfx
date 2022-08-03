#ifndef HTCW_SFX_MIDI_STREAM_HPP
#define HTCW_SFX_MIDI_STREAM_HPP
#include "sfx_core.hpp"
#include "sfx_midi_core.hpp"
namespace sfx {

// represents a class for fetching MIDI messages out of a stream
class midi_stream final {
public:
    static const size_t decode_message(bool is_file,stream& in, midi_message* in_out_message);
    // decode the next event. The contents of the 
    // in_out_event should be preserved between calls to this method.
    static const size_t decode_event(bool is_file, 
                                    stream& in, 
                                    midi_event_ex* in_out_event);
};
}  // namespace sfx
#endif  // HTCW_SFX_MIDI_STREAM_HPP
