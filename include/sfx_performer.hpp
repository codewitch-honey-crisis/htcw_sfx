#ifndef HTCW_SFX_PERFORMER_HPP
#define HTCW_SFX_PERFORMER_HPP
#include <string.h>
#include "sfx_core.hpp"
#include "sfx_waveform.hpp"
#include "sfx_wav_file.hpp"
#include "sfx_silence.hpp"
#include "sfx_mixer.hpp"
#include "sfx_transport.hpp"

namespace sfx {
    template<size_t Voices = 5, size_t SampleRate = 44100, size_t Channels=2, size_t BitDepth = 16, size_t MixerBlockSamples = 256, size_t TransportBlockSamples=32>
    class performer final {
    public:
        using type = performer;
        using mixer_type = mixer_source<Voices,SampleRate,Channels,BitDepth,MixerBlockSamples>;
        using waveform_type = waveform_source<SampleRate,Channels,BitDepth>;
        using wav_type = wav_file_source;
        constexpr static const size_t voices = Voices;
        constexpr static const size_t mixer_block_samples = MixerBlockSamples;
        constexpr static const size_t transport_block_samples = TransportBlockSamples;
    private:
        mixer_type m_mixer;
        transport m_transport;
        audio_destination& m_destination;
        size_t m_next_voice;
        unsigned m_voice_id;
        unsigned m_voice_age[voices];
        wav_type m_wavs[voices];
        waveform_type m_wfrms[voices];
        void deallocate() {
           for(size_t i = 0;i<voices;++i) {
                m_mixer.voice(i,nullptr);
            }
        }
        performer(const performer& rhs)=delete;
        performer& operator=(const performer& rhs)=delete;
        performer(const performer&& rhs)=delete;
        performer& operator=(const performer&& rhs)=delete;
        int next_free_voice() {
            unsigned min_age = 0;
            size_t index;
            for(size_t i = 0;i<voices;++i) {
                if(m_mixer.voice(i)==nullptr) {
                    return i;
                }
                if(m_voice_age[i]>min_age) {
                    min_age = m_voice_age[i];
                    index = i;
                }
            }
            return index;
        }
    public:
        performer(audio_destination& destination, void*(allocator)(size_t)=::malloc,void*(reallocator)(void*,size_t)=::realloc,void(deallocator)(void*)=::free) : m_destination(destination), m_next_voice(0),m_voice_id(0) {
            if(sfx_result::success==mixer_type::create(&m_mixer,allocator,reallocator,deallocator)) {
                transport::create(destination,m_mixer,&m_transport, transport_block_samples,allocator,deallocator);
                memset(m_voice_age,0,sizeof(unsigned)*voices);
            }
        }
        inline ~performer() {
            deallocate();
        }
        inline bool initialized() const {
            return m_mixer.initialized() && m_transport.initialized();
        }
        /*performer(performer&& rhs) {
            m_next_voice = rhs.m_next_voice;
            m_mixer = rhs.m_mixer;
            m_transport = rhs.m_transport;
        }
        performer& operator=(performer&& rhs) {
            deallocate();
            m_next_voice = rhs.m_next_voice;
            m_mixer = rhs.m_mixer;
            m_transport = rhs.m_transport;
            return *this;
        }*/
        int shape(float frequency, float volume = 1.0, waveform_shape shape = waveform_shape::sine) {
            if(!m_mixer.initialized() || !m_transport.initialized()) {
                return -1;
            }
            waveform_type& w = m_wfrms[m_next_voice];
            w.frequency(frequency);
            w.amplitude(1.0);
            w.shape(shape);
            return source(w,volume);
        }
        int wav(stream& in_stream, float volume = 1.0,bool loop = true, wav_file_end_callback ended_callback=nullptr,void* ended_callback_state = nullptr) {
            if(!m_mixer.initialized() || !m_transport.initialized()) {
                return -1;
            }
            if(in_stream.caps().seek) {
                if(0!=in_stream.seek(0)) {
                    return -1;
                }
            }
            wav_type& w = m_wavs[m_next_voice];
            
            if(sfx_result::success!=wav_type::open(in_stream,&w)) {
                // invalid wav
                return -1;
            }
            if(loop) {
                if(!w.can_loop()) {
                    return -1;
                }
                w.loop(true);
            }
            w.ended_callback(ended_callback,ended_callback_state);
            
            return source(w,volume);
        }
        int source(audio_source& source, float volume = 1.0) {
            if(!m_mixer.initialized() || !m_transport.initialized()) {
                return false;
            }
            m_mixer.level(m_next_voice,volume);
            m_mixer.voice(m_next_voice,&source);
            m_voice_age[m_next_voice]=++m_voice_id;
            int result = m_next_voice;
            m_next_voice = next_free_voice();
            return result;
        }
        bool stop(int voice=-1) {
            if(!m_mixer.initialized() || !m_transport.initialized()) {
                return false;
            }
            
            if(voice>=voices) {return false;}
            if(0>voice) {
                for(size_t i = 0;i<voices;++i) {
                    m_mixer.voice(i,nullptr);
                }
                m_next_voice = 0;
                return true;
            } else {
                m_mixer.voice(voice,nullptr);
                m_next_voice=voice;
            }
            return true;
        }
        inline bool update() {
            return m_mixer.initialized() && m_transport.initialized() && m_transport.update()==sfx_result::success;
        }
    };
}
#endif