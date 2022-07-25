#ifndef HTCW_SFX_WAV_FILE
#define HTCW_SFX_WAV_FILE
#include "sfx_core.hpp"
namespace sfx {
    typedef void(*wav_file_end_callback)(void* state);
    struct wav_file final {
        uint16_t channels;
        uint32_t sample_rate;
        uint16_t bit_depth;
        uint16_t format;
        uint32_t data_offset; // always 44 for wav files
        uint32_t data_size;
        static sfx_result read(stream& in, wav_file* out_file);
    };
    class wav_file_source final : public audio_source {
        wav_file m_file;
        stream* m_stream;
        uint32_t m_position;
        bool m_loop;
        bool m_ended;
        wav_file_end_callback m_ended_callback;
        void* m_ended_callback_state;
    public:
        inline const wav_file& file() const { return m_file; }
        bool can_loop() const;
        inline bool loop() const { return m_loop; }
        sfx_result loop(bool value);
        inline bool ended() const { return m_ended; }
        inline void ended_callback(wav_file_end_callback callback, void* state = nullptr) {
            m_ended_callback = callback;
            m_ended_callback_state = state;
        }
        inline unsigned long long position() const { return (unsigned long long)m_position; }
        static sfx_result open(stream& in, wav_file_source* out_source);
        virtual size_t bit_depth() const;
        virtual size_t sample_rate() const;
        virtual size_t channels() const;
        virtual audio_format format() const;
        virtual size_t read(void* samples, size_t sample_count);
    };
}
#endif