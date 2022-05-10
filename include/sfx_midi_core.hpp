#ifndef HTCW_SFX_MIDI_HPP
#define HTCW_SFX_MIDI_HPP
#include "sfx_core.hpp"
#include "sfx_midi_message.hpp"
namespace sfx {
    // a class that can output MIDI messages
    struct midi_output {
        virtual sfx_result send(const midi_message& message)=0;
    };
    // a class that can produce MIDI messages
    struct midi_input {
        virtual sfx_result receive(midi_message* out_message)=0;
    };
    // a MIDI source or destination
    struct midi_target {

    };
    // a source for MIDI events
    struct midi_source : public midi_target {
        virtual unsigned long long elapsed() const=0;
        virtual sfx_result receive(midi_event* out_event)=0;
        virtual sfx_result reset()=0;
    };
    // a destination for consuming MIDI events
    struct midi_destination : public midi_target {
        virtual int16_t timebase() const=0;
        virtual void timebase(int16_t value)=0;
        virtual int32_t microtempo() const=0;
        virtual void microtempo(int32_t value)=0;
        virtual unsigned long long elapsed() const=0;
        virtual bool full() const=0;
        virtual size_t queued() const=0;
        virtual sfx_result send(const midi_event& event)=0;
        virtual sfx_result send(const midi_event_ex& event)=0;
        virtual sfx_result start()=0;
        virtual sfx_result stop()=0;
        virtual sfx_result update()=0;
    };
}
#endif // HTCW_SFX_MIDI_HPP