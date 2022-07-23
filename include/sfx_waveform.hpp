#ifndef HTCW_SFX_WAVFORM
#define HTCW_SFX_WAVFORM
#include "sfx_core.hpp"
#include <math.h>
namespace sfx {
    enum struct waveform_shape {
        sine = 0,
        square = 1,
        triangle = 2,
        sawtooth = 3
    };
    template<size_t SampleRate=44100,size_t Channels=2, size_t BitDepth = 16>
    class waveform_source final : public audio_source {
        static_assert(SampleRate>0,"SampleRate must be greater than zero");
        static_assert(Channels==1||Channels==2,"Channels must be 1 or 2");
        static_assert(BitDepth==8||BitDepth==16,"BitDepth must be 8 or 16");
        constexpr static const size_t sample_rate_ = SampleRate;
        constexpr static const size_t channels_ = Channels;
        constexpr static const size_t bit_depth_ = BitDepth;
    public:
        
    private:
        float m_frequency;
        float m_amplitude;
        waveform_shape m_shape;
        float m_phase;
        void inc_phase(float delta) {
            m_phase+=delta;
            if(m_phase>TWO_PI) {
                m_phase-=TWO_PI;
            }
        }
    public:
        using type = waveform_source;
        waveform_source() : m_frequency(440.0),m_amplitude(1.0),m_shape(waveform_shape::sine) {

        }
        inline float frequency() const { return m_frequency; }
        void frequency(float value_hz) { m_frequency = value_hz; }
        void amplitude(float amplitude_scale) { m_amplitude = amplitude_scale; }
        virtual size_t bit_depth() const {
            return bit_depth_;
        }
        virtual size_t sample_rate() const {
            return sample_rate_;
        }
        virtual size_t channels() const {
            return channels_;
        }
        virtual audio_format format() const {
            return audio_format::pcm;
        }
        virtual size_t read(void* samples, size_t sample_count) {
            const float delta = TWO_PI*m_frequency/sample_rate_;
            float phase;
            float f;
            switch(bit_depth_) {
                case 8: {
                    int8_t* out = (int8_t*)samples;
                    for(int i = 0;i<sample_count;++i) {
                        switch(m_shape) {
                            case waveform_shape::sine:
                                f=sin(m_phase);
                                break;
                            case waveform_shape::square:
                                f=(m_phase>PI)*2-1.0;;
                                break;
                            case waveform_shape::triangle:
                                f=(m_phase-PI)/PI;
                                break;
                            case waveform_shape::sawtooth:
                                f=(m_phase-PI)/PI;
                                break;
                        }
                        f*=m_amplitude;
                        if(f<-1) f=-1;
                        if(f>1) f=1;
                        int8_t smp = f==1.0?127:f*128;;
                        *(out++) = smp;
                        if(channels_==2) {
                            *(out++) = smp;
                        }
                        inc_phase(delta);
                    }
                    break;
                }
                case 16: {
                    int16_t* out = (int16_t*)samples;
                    for(int i = 0;i<sample_count;++i) {
                        switch(m_shape) {
                            case waveform_shape::sine:
                                f=sin(m_phase);
                                break;
                            case waveform_shape::square:
                                f=(m_phase>PI)*2-1.0;;
                                break;
                            case waveform_shape::triangle:
                                f=(m_phase-PI)/PI;
                                break;
                            case waveform_shape::sawtooth:
                                f=(m_phase-PI)/PI;
                                break;
                        }
                        f*=m_amplitude;
                        if(f<-1) f=-1;
                        if(f>1) f=1;
                        int16_t smp = f==1.0?32767:f*32768;;
                        *(out++) = smp;
                        if(channels_==2) {
                            *(out++) = smp;
                        }
                        inc_phase(delta);
                    }
                    break;
                }
            }
            
            return sample_count;
        }
    };
}
#endif