#include <sfx_transport.hpp>
namespace sfx {
    sfx_result transport::create(audio_destination& destination,audio_source& source, transport* out_transport, size_t block_samples,void*(allocator)(size_t), void(deallocator)(void*)) {
        if(out_transport==nullptr || 
            0>=block_samples || 
            allocator==nullptr||
            deallocator==nullptr) {
            return sfx_result::invalid_argument;
        }
        if(source.sample_rate()!=destination.sample_rate() ||
            source.channels()!=destination.channels() ||
            source.bit_depth()!=destination.bit_depth() ||
            source.format()!=destination.format()) {
            return sfx_result::invalid_argument;
        }
        
        size_t block_bytes = (block_samples*source.bit_depth()+7)/8;
        void* p = allocator(block_bytes);
        if(p==nullptr) {
            return sfx_result::out_of_memory;
        }
        out_transport->m_destination = &destination;
        out_transport->m_source = &source;
        out_transport->m_block = p;
        out_transport->m_block_samples = block_samples;
        out_transport->m_deallocator = deallocator;
        out_transport->m_volume = 1.0f;
        return sfx_result::success;
    }
    sfx_result transport::update() {

        if(m_block==nullptr) {
            return sfx_result::invalid_state;
        }
        size_t samples_read = m_source->read(m_block,m_block_samples);
        if(samples_read>0) {
            if(m_volume!=1.0) {
                switch(m_destination->bit_depth()) {
                    case 8: {
                        int8_t* p = (int8_t*)m_block;
                        for(size_t i = 0;i<samples_read;++i) {
                            *p=int8_t(*p*m_volume);
                            ++p;
                        }
                    }
                    break;
                    case 16: {
                        int16_t* p = (int16_t*)m_block;
                        for(size_t i = 0;i<samples_read;++i) {
                            *p=int16_t(*p*m_volume);
                            ++p;
  
                        }
                    }
                    break;
                }
            }

            if(samples_read!=m_destination->write(m_block,samples_read)) {
                return sfx_result::device_error;
            }
        } else {
            return sfx_result::device_error;
        }
        return sfx_result::success;
    }
    void transport::deallocate() {
        if(m_deallocator) {
            if(m_block) {
                m_deallocator(m_block);
                m_block = nullptr;
            }
        }
    }
    void transport::copy_from(const transport& rhs) {
        m_destination = rhs.m_destination;
        m_source = rhs.m_source;
        m_block = rhs.m_block;
        m_block_samples = rhs.m_block_samples;
        m_deallocator = rhs.m_deallocator;
    }
    transport::transport(transport&& rhs) {
        copy_from(rhs);
        rhs.m_block = nullptr;
    }
    transport& transport::operator=(transport&& rhs) {
        deallocate();
        copy_from(rhs);
        rhs.m_block = nullptr;
        return *this;
    }

}