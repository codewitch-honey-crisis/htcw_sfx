#ifndef HTCW_SFX_CORE_HPP
#define HTCW_SFX_CORE_HPP
#include <io_stream.hpp>
#include <stdint.h>
#ifndef ARDUINO
    #define PROGMEM 
    #define pgm_read_byte(x) (*x)
#endif
namespace sfx {
    static_assert(bits::endianness()!=bits::endian_mode::none,"Please define HTCW_LITTLE_ENDIAN or HTCW_BIG_ENDIAN before including SFX to indicate the byte order of the platform.");
    using stream = io::stream;
    using seek_origin = io::seek_origin;
    using stream_caps = io::stream_caps;
#ifdef ARDUINO
    using arduino_stream = io::arduino_stream;
#endif
    using buffer_stream = io::buffer_stream;
    using const_buffer_stream = io::const_buffer_stream;
#ifndef IO_NO_FS
    using file_stream = io::file_stream;
#endif
    enum struct sfx_result {
        success = 0,
        canceled,
        invalid_argument,
        not_supported,
        io_error,
        device_error,
        out_of_memory,
        invalid_format,
        end_of_stream,
        invalid_state,
        unknown_error
    };
    enum struct audio_format {
        pcm = 0
    };
    struct audio_target {
        virtual size_t bit_depth() const=0;
        virtual size_t sample_rate() const=0;
        virtual size_t channels() const=0;
        virtual audio_format format() const=0;
    };
    struct audio_source : public audio_target {
        virtual size_t read(void* samples,size_t sample_count)=0;
    };
    struct audio_destination : public audio_target {
        virtual size_t write(const void* samples, size_t sample_count)=0;
    };
    
    
    
}

#endif
