#ifndef HTCW_SFX_MIDI_CLOCK_HPP
#define HTCW_SFX_MIDI_CLOCK_HPP
#include <chrono>
#include "sfx_midi_utility.hpp"
#include "sfx_midi_message.hpp"
namespace sfx {

// provides a clock suitable for timing MIDI playback
class midi_clock final {
    using clock_t = std::chrono::steady_clock;
    using tp_t = clock_t::time_point;
    using dur_t = clock_t::duration;
    constexpr static const double clock_period = ((double)clock_t::period::num/(double)clock_t::period::den);
    constexpr static const double clock_hz = ((double)clock_t::period::den/(double)clock_t::period::num);
    double m_tempo;
    int32_t m_microtempo;
    int16_t m_timebase;
#ifdef ARDUINO
    int8_t m_source_pin;
#endif
    void(*m_tick_callback)(uint32_t, unsigned long long, void*);
    void* m_tick_callback_state;
    tp_t m_last;
    unsigned long long m_elapsed;
    bool m_started;
    
    midi_clock(const midi_clock& rhs)=delete;
    midi_clock& operator=(const midi_clock& rhs)=delete;
public:
    // constructs an instance
    midi_clock();
    // copy constructor
    midi_clock(midi_clock&& rhs);
    // assignment
    midi_clock& operator=(midi_clock&& rhs);
    // destructor
    ~midi_clock();
    // gets the tempo
    inline double tempo() const {
        return m_tempo;
    }
    // sets the tempo
    inline void tempo(double value) {
        m_tempo = value;
        m_microtempo = midi_utility::tempo_to_microtempo(value);
    }
    // gets the MIDI microtempo
    inline int32_t microtempo() const {
        return m_microtempo;
    }
    // sets the MIDI microtempo
    inline void microtempo(int32_t value) {
        m_microtempo = value;
        m_tempo = midi_utility::microtempo_to_tempo(value);
    }
    // gets the timebase in pulses per quarter note (PPQN)
    inline int16_t timebase() const {
        return m_timebase;
    }
    // sets the timebase in pulses per quarter note (PPQN)
    inline void timebase(int16_t value) {
        m_timebase = value;
    }
    // gets the elapsed ticks since start()
    inline unsigned long long elapsed() const {
        return m_elapsed*m_started;
    }
    inline void elapsed(unsigned long long value) {
        m_elapsed=value;
    }
    inline bool started() const {
        return m_started;
    }
    // gets the waiting ticks that weren't processed since the last time the clock was updated
    uint32_t pending();
    // update the clock. call this in a loop unless source_pin() is set
    void update();
    // the optional callback function with which to receive tick notifications
    void tick_callback(void(callback)(uint32_t pending_ticks,unsigned long long elapsed_ticks,void* state),void* state=nullptr);

    // starts or resets the clock
    void start();
    // stops the clock
    void stop();
};
} // namespace sfx
#endif // HTCW_SFX_MIDI_CLOCK_HPP