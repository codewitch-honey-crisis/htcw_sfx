#ifndef HTCW_SFX_MIDI_BUFFER_HPP
#define HTCW_SFX_MIDI_BUFFER_HPP
#include "sfx_midi_core.hpp"
#include "sfx_midi_stream.hpp"
namespace sfx {
    template <size_t Capacity>
    class midi_buffer {
        midi_event_ex m_data[Capacity];
        size_t m_head;
        size_t m_tail;
        bool m_full;
        void advance() {
            if (m_full) {
                if (++(m_tail) == capacity) {
                    m_tail = 0;
                }
            }

            if (++(m_head) == capacity) {
                m_head = 0;
            }
            m_full = (m_head == m_tail);
        }
        void retreat() {
            m_full = false;
            if (++(m_tail) == capacity) { 
                m_tail = 0;
            }
        }
    public:
        using type = midi_buffer;
        using element_type = midi_event_ex;
        constexpr static const size_t capacity = Capacity;

        inline midi_buffer() {
            clear();
        }
        inline bool empty() const {
            return (!m_full && (m_head == m_tail));
        }
        inline bool full() const {
            return m_full;
        }
        size_t size() const {
            size_t result = capacity;
            if(!m_full) {
                if(m_head >= m_tail) {
                    result = (m_head - m_tail);
                } else {
                    result = (capacity + m_head - m_tail);
                }
            }
            return result;
        }
        inline void clear() {
            m_head = 0;
            m_tail = 0;
            m_full = false;
        }
        void put(const element_type& value) {
            m_data[m_head] = value;
            advance();
        }
        const element_type* peek() const {
            if(!empty()) {
                return m_data+m_tail;
            }
            return nullptr;
        }
        bool get(element_type* out_value) {
            if(!empty()) {
                if(out_value!=nullptr) {
                    *out_value = m_data[m_tail];
                }
                retreat();
                return true;
            }
            return false;
        }
        
    };
    using midi_buffer16 = midi_buffer<16>;
    using midi_buffer32 = midi_buffer<32>;
}  // namespace sfx
#endif