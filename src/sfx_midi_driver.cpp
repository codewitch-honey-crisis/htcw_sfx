#include "sfx_midi_driver.hpp"
namespace sfx {
// implement std::move to limit dependencies on the STL, which may not be there
template< class T > struct midi_driver_remove_reference      { typedef T type; };
template< class T > struct midi_driver_remove_reference<T&>  { typedef T type; };
template< class T > struct midi_driver_remove_reference<T&&> { typedef T type; };
template <typename T>
typename midi_driver_remove_reference<T>::type&& midi_driver_move(T&& arg) {
    return static_cast<typename midi_driver_remove_reference<T>::type&&>(arg);
}
void midi_driver::tick_callback(uint32_t pending, unsigned long long elapsed, void* state) {
    midi_driver& rthis = *(midi_driver*)state;
    // events on the queue?
    while (!rthis.m_queue.empty()) {
        // peek the next one
        const midi_event_ex* event = rthis.m_queue.peek();
        // is it ready to be played
        if (event->absolute <= elapsed) {
            // special handing for midi meta file events
            if (event->message.type() == midi_message_type::meta_event) {
                // if it's a tempo event update the clock tempo
                if(event->message.meta.type == 0x51) {
                    int32_t mt = (event->message.meta.data[0] << 16) | (event->message.meta.data[1] << 8) | event->message.meta.data[2];
                    // update the clock microtempo
                    rthis.m_clock.microtempo(mt/rthis.m_tempo_scale);
                }
            } else {
                midi_message msg = event->message;
                if(rthis.m_velocity_scale!=1.0) {
                    switch(msg.type()) {
                        case midi_message_type::note_off:
                        case midi_message_type::note_on:
                            int i = msg.msb()*rthis.m_velocity_scale;
                            if(i>127) i = 127;
                            msg.msb(uint8_t(i));
                            break;
                    }
                }
                rthis.m_output.send(msg);
            }
            // ensure the message gets destroyed
            // (necessary? i don't think so but i'd rather not leak)
            event->message.~midi_message();
            // remove the message
            rthis.m_queue.get(nullptr);
        } else {
            break;
        }
    }
}
midi_driver::midi_driver(midi_output& output) : m_output(output),m_tempo_scale(1.0),m_velocity_scale(1.0) {
    m_clock.tick_callback(tick_callback,this);
}
int16_t midi_driver::timebase() const {
    return m_clock.timebase();
}
void midi_driver::timebase(int16_t value) {
    m_clock.timebase(value);
}
int32_t midi_driver::microtempo() const {
    return m_clock.microtempo()*m_tempo_scale;
}
void midi_driver::microtempo(int32_t value) {
    m_clock.microtempo(value*m_tempo_scale);
}
unsigned long long midi_driver::elapsed() const {
    return m_clock.elapsed_ticks();
}
bool midi_driver::full() const {
    return m_queue.full();
}
size_t midi_driver::queued() const {
    return m_queue.size();
}
sfx_result midi_driver::send(const midi_event& event) {
    if(m_queue.full()) {
        return sfx_result::out_of_memory;
    }
    midi_event_ex mse;
    mse.absolute = event.delta + m_clock.elapsed_ticks();
    mse.delta = event.delta;
    mse.message = event.message;
    m_queue.put(mse);
    return sfx_result::success;
}
sfx_result midi_driver::send(const midi_event_ex& event) {
    if(m_queue.full()) {
        return sfx_result::out_of_memory;
    }
    m_queue.put(event);
    return sfx_result::success;
}

sfx_result midi_driver::start() {
    m_clock.start();
    return sfx_result::success;
}
sfx_result midi_driver::stop() {
    m_clock.stop();
    m_queue.clear();
    return sfx_result::success;
}
sfx_result midi_driver::update() {
    m_clock.update();
    return sfx_result::success;
}
}