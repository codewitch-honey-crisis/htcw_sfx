#ifndef HTCW_SFX_MIXER_HPP
#define HTCW_SFX_MIXER_HPP
#include "sfx_core.hpp"
#include "sfx_sample_utility.hpp"
#include <string.h>
namespace sfx {
template<size_t Tracks,size_t SampleRate=44100,size_t Channels=2, size_t BitDepth = 16, size_t InitialBlockSamples=32>
class mixer_source final : public audio_source {
    static_assert(Tracks>0,"Tracks must be greater than zero");
    static_assert(SampleRate>0,"SampleRate must be greater than zero");
    static_assert(Channels==1||Channels==2,"Channels must be 1 or 2");
    static_assert(BitDepth==8||BitDepth==16,"BitDepth must be 8 or 16");
    constexpr static const size_t sample_rate_ = SampleRate;
    constexpr static const size_t channels_ = Channels;
    constexpr static const size_t bit_depth_ = BitDepth;
    constexpr static const size_t initial_block_samples = InitialBlockSamples;
    constexpr static const size_t initial_block_size = ((initial_block_samples*bit_depth_)+7)/8;
public:
    using type = mixer_source;
    constexpr static const size_t tracks = Tracks;
private:
    audio_source* m_tracks[tracks];
    float m_track_levels[tracks];
    void* m_block;
    size_t m_block_size;
    void*(*m_reallocator)(void*,size_t);
    void(*m_deallocator)(void*);
    void deallocate() {
        if(m_deallocator!=nullptr) {
            if(m_block!=nullptr) {
                m_deallocator(m_block);
                m_block = nullptr;
            }
        }
    }
    mixer_source(const mixer_source& rhs)=delete;
    mixer_source& operator=(const mixer_source& rhs)=delete;
public:
    inline mixer_source() : m_block(nullptr) {}
    mixer_source(mixer_source&& rhs) {
        for(int i = 0;i<tracks;++i) {
            m_track_levels[i]=rhs.m_track_levels[i];
            m_tracks[i]=rhs.m_tracks[i];
            rhs.m_tracks[i]=nullptr;
        }
        m_block = rhs.m_block;
        rhs.m_block = nullptr;
        m_block_size = rhs.m_block_size;
        rhs.m_block_size = 0;
        m_deallocator = rhs.m_deallocator;
        m_reallocator = rhs.m_reallocator;
    }
    mixer_source& operator=(mixer_source&& rhs) {
        deallocate();
        for(int i = 0;i<tracks;++i) {
            m_track_levels[i]=rhs.m_track_levels[i];
            m_tracks[i]=rhs.m_tracks[i];
            rhs.m_tracks[i]=nullptr;
        }
        m_block = rhs.m_block;
        rhs.m_block = nullptr;
        m_block_size = rhs.m_block_size;
        rhs.m_block_size = 0;
        m_deallocator = rhs.m_deallocator;
        m_reallocator = rhs.m_reallocator;
        return *this;
    }
    inline ~mixer_source() {
        deallocate();
    }
    inline bool initialized() const { return m_block!=nullptr; }
    audio_source* track(size_t voice_index) const {
        if(voice_index<0 || voice_index>=tracks) {
            return nullptr;
        }
        return m_tracks[voice_index];
    }
    void track(size_t track_index,audio_source* value) {
        if(track_index<0 || track_index>=tracks) {
            return;
        }
        m_tracks[track_index]=value;
    }
    float level(size_t voice_index) const {
        if(voice_index<0 || voice_index>=tracks) {
            return NAN;
        }
        return m_track_levels[voice_index];
    }
    void level(size_t voice_index,float value) {
        if(voice_index<0 || voice_index>=tracks) {
            return;
        }
        m_track_levels[voice_index]=value;
    }
    virtual size_t bit_depth() const {return bit_depth_;}
    virtual size_t sample_rate() const { return sample_rate_; }
    virtual size_t channels() const { return channels_; }
    virtual audio_format format() const { return audio_format::pcm; }
    virtual size_t read(void* samples, size_t sample_count) {
        size_t result=0;
        if(samples!=nullptr && m_block!=nullptr && sample_count!=0) {
            const size_t bytes_requested = (sample_count*bit_depth_+7)/8;
            if(bytes_requested>m_block_size) {
                m_block = m_reallocator(m_block,bytes_requested);
                if(nullptr==m_block) {
                    // out of memory!
                    return 0;
                }
                m_block_size = bytes_requested;
            }
            
            if(m_tracks[0]==nullptr||m_track_levels[0]==0.0) {
                size_t samples_read = sample_count;
                if(m_tracks[0]!=nullptr) {
                    // advance it
                    samples_read=m_tracks[0]->read((uint8_t*)samples,sample_count);
                }
                int16_t* p = (int16_t*)samples;
                for(size_t i = 0;i<sample_count;++i) {
                    p[i]=-32768*tracks==1;
                }
                result = samples_read;
            } else {
                const size_t samples_to_read = sample_count;
                const size_t samples_read= m_tracks[0]->read((uint8_t*)samples,samples_to_read);
                int16_t* p;
                if(m_track_levels[0]!=1.0) {
                    p = (int16_t*)samples;
                    for(int i = 0;i<samples_read;++i) {
                        int32_t smp = *p*m_track_levels[0];
                        if(smp>32767) {
                            smp=32767;
                        } else if(smp<-32768) {
                            smp=-32768;
                        }
                        *(p++) = int16_t(smp);
                    }
                }
                p = (int16_t*)samples;
                for(int i = samples_read;i<samples_to_read;++i) {
                    p[i]=-32768;
                }
                result = samples_to_read;
            }
            for(size_t j = 1;j<tracks;++j) {
                audio_source* track = m_tracks[j];
                if(track==nullptr) {
                    continue;
                }
                int16_t* ps = (int16_t*)samples;
                int16_t* pb = (int16_t*)m_block;
                const size_t samples_to_read = sample_count;
                const size_t samples_read = track->read(m_block,samples_to_read);
                int32_t smp;
                if(m_track_levels[j]!=0.0) {
                    for(size_t i = 0;i<samples_read;++i) {
                        smp = *ps;
                        smp+=(*(pb++)*m_track_levels[j]);
                        if(smp>32767) {
                            smp=32767;
                        } else if(smp<-32768) {
                            smp=-32768;
                        }
                        *(ps++)=(int16_t)smp;
                    }
                }
                if(samples_read>result) {
                    result = samples_read;
                }
            }
        }
        return result;
    }
    static sfx_result create(mixer_source* out_mixer, void*(allocator)(size_t)=::malloc,void*(reallocator)(void*,size_t)=::realloc,void(deallocator)(void*)=::free) {
        if(out_mixer==nullptr||allocator==nullptr||reallocator==nullptr||deallocator==nullptr) {
            return sfx_result::invalid_argument;
        }
        out_mixer->m_block = nullptr;
        for(int i = 0; i <tracks;++i) {
            out_mixer->m_track_levels[i]=1.0;
            out_mixer->m_tracks[i]=nullptr;
        }
        out_mixer->m_block = allocator(initial_block_size);
        if(out_mixer->m_block == nullptr) {
            return sfx_result::out_of_memory;
        }
        out_mixer->m_block_size = initial_block_size;
        
        out_mixer->m_reallocator = reallocator;
        out_mixer->m_deallocator = deallocator;
        return sfx_result::success;
    }
};
}
#endif