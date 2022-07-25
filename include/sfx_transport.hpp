#ifndef HTCW_SFX_TRANSPORT_HPP
#define HTCW_SFX_TRANSPORT_HPP
#include "sfx_core.hpp"
namespace sfx {
    class transport final {
        audio_destination* m_destination;
        audio_source* m_source;
        void* m_block;
        size_t m_block_samples;
        float m_volume;
        void(*m_deallocator)(void*);
        void deallocate();
        void copy_from(const transport& rhs);
        transport(const transport& rhs)=delete;
        transport& operator=(const transport& rhs)=delete;
    public:
        inline transport() : m_block(nullptr),m_volume(1.0) {}
        inline ~transport() { deallocate();}
        transport(transport&& rhs);
        transport& operator=(transport&& rhs);
        inline audio_destination& destination() const { return *m_destination; }
        inline void destination(audio_destination& destination) { *m_destination=destination; }
        inline audio_source& source() const { return *m_source; }
        inline void source(audio_source& source) { *m_source=source; }
        inline float volume() const { return m_volume; }
        inline void volume(float value) { m_volume = value; }
        sfx_result update();
        static sfx_result create(audio_destination& destination,audio_source& source, transport* out_transport, size_t block_samples = 32,void*(allocator)(size_t)=::malloc, void(deallocator)(void*)=::free);
    };

}
#endif
