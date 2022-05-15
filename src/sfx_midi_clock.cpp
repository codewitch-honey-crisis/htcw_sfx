#include <sfx_midi_clock.hpp>
#include <sfx_midi_message.hpp>

namespace sfx {
midi_clock::midi_clock() : m_tempo(120.0),
            m_microtempo(500000),
            m_timebase(24),
            m_tick_callback(nullptr),
            m_tick_callback_state(nullptr),
            m_last(tp_t::min()),
            m_pending_ticks(0.0),
            m_elapsed(0),
            m_started(false) {
    
}
midi_clock::midi_clock(midi_clock&& rhs) : m_started(false) {
    m_tempo=rhs.m_tempo;
    m_microtempo=rhs.m_microtempo;
    m_timebase = rhs.m_timebase;
    m_last = rhs.m_last;
    m_pending_ticks = rhs.m_pending_ticks;
    m_elapsed = rhs.m_elapsed;
    rhs.m_tick_callback_state = rhs.m_tick_callback_state;
    m_tick_callback = rhs.m_tick_callback;
    rhs.m_tick_callback = nullptr;
    m_started = rhs.m_started;
    rhs.m_started = false;
}
midi_clock& midi_clock::operator=(midi_clock&& rhs) {
    m_started = false;
    m_tempo=rhs.m_tempo;
    m_microtempo=rhs.m_microtempo;
    m_timebase = rhs.m_timebase;
    m_last = rhs.m_last;
    m_pending_ticks = rhs.m_pending_ticks;
    m_elapsed = rhs.m_elapsed;
    rhs.m_tick_callback_state = rhs.m_tick_callback_state;
    m_tick_callback = rhs.m_tick_callback;
    rhs.m_tick_callback = nullptr;
    m_started = rhs.m_started;
    rhs.m_started = false;
    return *this;
}
midi_clock::~midi_clock() {
}
void midi_clock::update() {
    if(!m_started) {
        return;
    }
    auto per = clock_t::now()-m_last;
    double elapsed_secs = per.count()/clock_hz;
    if(elapsed_secs!=0.0) {
        double elapsed = elapsed_secs/midi_utility::sec_per_tick(m_microtempo,m_timebase);
        if(elapsed>=1.0) {
            m_elapsed+=elapsed;
            if(m_tick_callback!=nullptr) {
                m_tick_callback(elapsed_secs*elapsed,m_elapsed,m_tick_callback_state);
            }
            m_last = clock_t::now();
        }
    }
    
}
uint32_t midi_clock::pending_ticks() {
    if(!m_started) {
        return 0;
    }
    auto per = clock_t::now()-m_last;
    double elapsed_secs = per.count()*clock_period;
    if(elapsed_secs==0.0) { 
        return 0;
    }
    return elapsed_secs/midi_utility::sec_per_tick(m_microtempo,m_timebase);
}
void midi_clock::tick_callback(void(callback)(uint32_t,unsigned long long,void*),void* state) {
    bool started = m_started;
    m_started = false;
    m_tick_callback_state = state;
    m_tick_callback=callback;
    m_started = started;
}

void midi_clock::start() {
    m_started = false;
    m_elapsed = 0;
    m_pending_ticks = 0;
    m_last = clock_t::now();
    m_started = true;
}
void midi_clock::stop() {
    m_started = false;
}
}