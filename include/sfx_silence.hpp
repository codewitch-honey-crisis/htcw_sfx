#ifndef HTCW_SFX_SILENCE_HPP
#define HTCW_SFX_SILENCE_HPP
#include "sfx_core.hpp"
#include <math.h>
namespace sfx {
    template<size_t SampleRate=44100,size_t Channels=2, size_t BitDepth = 16>
    class silence_source final : public audio_source {
        static_assert(SampleRate>0,"SampleRate must be greater than zero");
        static_assert(Channels==1||Channels==2,"Channels must be 1 or 2");
        static_assert(BitDepth==8||BitDepth==16,"BitDepth must be 8 or 16");
        constexpr static const size_t sample_rate_ = SampleRate;
        constexpr static const size_t channels_ = Channels;
        constexpr static const size_t bit_depth_ = BitDepth;
    public:
        
    public:
        using type = silence_source;
        silence_source() {

        }
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
            switch(bit_depth_) {
                case 8:
                    memset(samples,-128,sample_count);
                    break;
                case 16: {
                    int16_t* p = (int16_t*)samples;
                    for(int i = 0;i<sample_count;++i) {
                        *(p++)=-32768;
                    }
                    break;
                }
            }
            return sample_count;
        }
    };
}
#endif