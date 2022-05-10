#ifndef HTCW_SFX_MIDI_DRIVER_HPP
#define HTCW_SFX_MIDI_DRIVER_HPP
#include "sfx_midi_core.hpp"
#include "sfx_midi_stream.hpp"
#include "sfx_midi_buffer.hpp"
#include "sfx_midi_clock.hpp"
namespace sfx {
    class midi_driver : public midi_destination {
        midi_output& m_output;
        midi_clock m_clock;
        midi_buffer16 m_queue;
        float m_tempo_scale;
        float m_velocity_scale;
        static void tick_callback(uint32_t pending, unsigned long long elapsed, void* state);
    public:
        midi_driver(midi_output& output);
        virtual int16_t timebase() const;
        virtual void timebase(int16_t value);
        virtual int32_t microtempo() const;
        virtual void microtempo(int32_t value);
        virtual unsigned long long elapsed() const;
        virtual bool full() const;
        virtual size_t queued() const;
        virtual sfx_result send(const midi_event& event);
        virtual sfx_result send(const midi_event_ex& event);
        virtual sfx_result start();
        virtual sfx_result stop();
        virtual sfx_result update();
        inline float tempo_scale() const { return m_tempo_scale; }
        inline void tempo_scale(float value) { if(value==value && value>0.0) { m_tempo_scale = value; m_clock.microtempo(m_clock.microtempo()/m_tempo_scale);} }
        inline float velocity_scale() const { return m_velocity_scale;}
        inline void velocity_scale(float value) { if(value==value && value>0.0) { m_velocity_scale = value; } }
    };
}
#endif // HTCW_SFX_MIDI_DRIVER_HPP