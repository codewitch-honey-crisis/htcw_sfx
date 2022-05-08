#ifndef HTCW_SFX_MIDI_MESSAGE_HPP
#define HTCW_SFX_MIDI_MESSAGE_HPP
#include <htcw_bits.hpp>
#include "sfx_core.hpp"
#include "sfx_midi_utility.hpp"
namespace sfx {
    // represents the type of MIDI message
    enum struct midi_message_type : uint8_t {
        // a note off message
        note_off = 0b10000000,
        // a note on message
        note_on = 0b10010000,
        // polyphonic pressure (aftertouch) message
        polyphonic_pressure = 0b10100000,
        // control change (CC) message
        control_change = 0b10110000,
        // program change/patch select message
        program_change = 0b11000000,
        // channel pressure (aftertouch) message
        channel_pressure = 0b11010000,
        // pitch wheel message
        pitch_wheel_change = 0b11100000,
        // system exclusive (sysex) message
        system_exclusive = 0b11110000,
        // 0b11110001 undefined
        // song position message
        song_position = 0b11110010,
        // song select message
        song_select = 0b11110011,
        // b11110100 undefined
        // b11110101 undefined
        // tune request message
        tune_request = 0b11110110,
        // end of system exclusive message
        end_system_exclusive = 0b11110111,
        // timing clock message
        timing_clock = 0b11111000,
        // 0b11111001 undefined
        // start message
        start_playback = 0b11111010,
        // continue message
        continue_playback = 0b11111011,
        // stop message
        stop_playback = 0b11111100,
        // 0b11111101 undefined
        // active sensing message
        active_sensing = 0b11111110,
        // reset message
        reset = 0b11111111,
        // MIDI file meta event message
        meta_event = 0b11111111
    };
    // represents a MIDI message
    class midi_message final {
        void copy(const midi_message& rhs);
        void deallocate();
    public:
        // the status byte
        uint8_t status;
        union {
            // the 8-bit value holder for a message with a single byte payload
            uint8_t value8;
            // the 16-bit value holder for a message with a two byte payload
            uint16_t value16;
            // systex information (type()==system_exclusive)
            struct {
                // the data
                uint8_t* data;
                // the size of the data
                size_t size;
            } sysex;
            // meta event information (type()==meta_event - MIDI files only) 
            struct {
                // the type of message
                uint8_t type;
                // the length encoded as a varlen 
                uint8_t encoded_length[3];
                // the meta data
                uint8_t* data;
            } meta;
        };
        // constructs a new message
        inline midi_message() : status(0) {
            memset(this,0,sizeof(midi_message));
            meta.data = nullptr;
        }
        // destroys a message
        inline ~midi_message() {
            deallocate();
        }
        // copies a message
        inline midi_message(const midi_message& rhs) {
            copy(rhs);
        }
        // copies a message
        inline midi_message& operator=(const midi_message& rhs) {
            deallocate();
            copy(rhs);
            return *this;
        }
        // steals a message
        inline midi_message(midi_message&& rhs) {
            memcpy(this,&rhs,sizeof(midi_message));
            memset(&rhs,0,sizeof(midi_message));
        }
        // steals a message
        inline midi_message& operator=(midi_message&& rhs) {
            deallocate();
            memcpy(this,&rhs,sizeof(midi_message));
            memset(&rhs,0,sizeof(midi_message));
            return *this;
        }
        // gets the channel (channel messages only)
        inline uint8_t channel() const {
            if(status<0b11110000) {
                return status & 0xF;
            }
            return 0;   
        }
        // sets the channel (channel messages only)
        inline void channel(uint8_t value) {
            if(status<0b11110000) {
                status = (status & 0xF0) | (value & 0x0F);
            }
        }
        // gets the type of message
        inline midi_message_type type() const {
            if(status<0b11110000) {
                return (midi_message_type)(status&0xF0);
            } else {
                return (midi_message_type)(status);
            }
        }
        // sets the type of message
        inline void type(midi_message_type value) {
            if(((int)value)<0xb11110000) {
                status = (status & 0x0F) | (((int)value));
            } else {
                status = (uint8_t)value;
            }
        }
        // get the MSB value for messages with a 2 byte payload
        inline uint8_t msb() const {
            return (value16 >> 8)&0x7f;
        }
        // set the MSB value for messages with a 2 byte payload
        inline void msb(uint8_t value) {
            value16 = (value16 & 0x7f) | uint16_t((value & 0x7f)<<8);
        }
        // get the LSB value for messages with a 2 byte payload
        inline uint8_t lsb() const {
            return value16 & 0x7f;
        }
        // set the LSB value for messages with a 2 byte payload
        inline void lsb(uint8_t value) {
            value16 = (value16 & uint16_t(0x7f<<8)) | (value & 0x7f);
        }
        // indicates the size of the message over the wire
        inline size_t wire_size() const {
            int32_t result;
            
            switch(type()) {
            case midi_message_type::note_off:
            case midi_message_type::note_on:
            case midi_message_type::polyphonic_pressure:
            case midi_message_type::control_change:
            case midi_message_type::pitch_wheel_change:
            case midi_message_type::song_position:
                return 3;
            case midi_message_type::program_change:
            case midi_message_type::channel_pressure:
            case midi_message_type::song_select:
                return  2;
            case midi_message_type::system_exclusive:
                return sysex.size+1;
            case midi_message_type::reset:
                if(meta.type&0x80) {
                    return 1;
                } else {
                    const uint8_t* p=midi_utility::decode_varlen(meta.encoded_length,&result);
                    if(p!=nullptr) {
                        return (size_t)result+(p-meta.encoded_length)+2;
                    }
                }
            
                return 1;
            case midi_message_type::end_system_exclusive:
            case midi_message_type::active_sensing:
            case midi_message_type::start_playback:
            case midi_message_type::stop_playback:
            case midi_message_type::tune_request:
            case midi_message_type::timing_clock:
                return 1;
            default:
                return 1;
            }
        }
    };
    // represents a MIDI event
    struct midi_event final {
        // the offset in MIDI ticks from the previous event
        int32_t delta;
        // the MIDI message
        midi_message message;
    };
    
}
#endif // HTCW_SFX_MIDI_MESSAGE_HPP