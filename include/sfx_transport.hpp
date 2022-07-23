#ifndef HTCW_SFX_TRANSPORT_HPP
#define HTCW_SFX_TRANSPORT_HPP
#include <sfx_core.hpp>
namespace sfx {
    class transport final {
        audio_destination* m_destination;
        audio_source* m_source;
        void* m_block;
        size_t m_block_samples;
        void(*m_deallocator)(void*);
        void deallocate();
        void copy_from(const transport& rhs);
        transport(const transport& rhs)=delete;
        transport& operator=(const transport& rhs)=delete;
    public:
        inline transport() : m_block(nullptr) {}
        inline ~transport() { deallocate();}
        transport(transport&& rhs);
        transport& operator=(transport&& rhs);
        sfx_result update();
        static sfx_result create(audio_destination* destination,audio_source* source, transport* out_transport, size_t block_samples = 32,void*(allocator)(size_t)=::malloc, void(deallocator)(void*)=::free);
    };

}
#endif
