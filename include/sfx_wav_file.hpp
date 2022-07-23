#ifndef HTCW_SFX_WAV_FILE
#define HTCW_SFX_WAV_FILE
#include "sfx_core.hpp"
namespace sfx {
    struct wav_file final {
        uint16_t channels;
        uint32_t sample_rate;
        uint16_t bit_depth;
        uint16_t format;
        uint32_t data_offset; // always 44 for wav files
        uint32_t data_size;
        static sfx_result read(stream* in, wav_file* out_file);
    };
    class wav_file_source final : public audio_source {
        wav_file m_file;
        stream* m_stream;
        uint32_t m_position;
        bool m_loop;
    public:
        bool can_loop() const;
        inline bool loop() const { return m_loop; }
        sfx_result loop(bool value);
        inline unsigned long long position() const { return (unsigned long long)m_position; }
        static sfx_result open(stream* in, wav_file_source* out_source);
        virtual size_t bit_depth() const;
        virtual size_t sample_rate() const;
        virtual size_t channels() const;
        virtual audio_format format() const;
        virtual size_t read(void* samples, size_t sample_count);
    };
}
#endif