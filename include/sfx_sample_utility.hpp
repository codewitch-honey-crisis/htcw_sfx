#ifndef HTCW_SFX_SAMPLE_UTILITY_HPP
#define HTCW_SFX_SAMPLE_UTILITY_HPP
#include "sfx_core.hpp"
namespace sfx {
    struct sample_utility final {
        typedef void(*convert)(void**in,void**out,size_t in_sample_count);
        static void mono8_to_stereo16(void** in,void** out, size_t in_sample_count);
        static void mono16_to_stereo16(void** in,void** out, size_t in_sample_count);
        static void stereo8_to_stereo16(void** in,void** out, size_t in_sample_count);
        static void stereo16_to_stereo16(void** in,void** out, size_t in_sample_count);
        static void stereo16_to_mono8(void** in,void** out, size_t in_sample_count);
        static void stereo16_to_mono16(void** in,void** out, size_t in_sample_count);
        static void stereo16_to_stereo8(void** in,void** out, size_t in_sample_count);
    };
}
#endif