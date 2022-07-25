#ifndef HTCW_SFX_MIDI_FILE_HPP
#define HTCW_SFX_MIDI_FILE_HPP
#include "sfx_core.hpp"
#include "sfx_midi_core.hpp"
#include "sfx_midi_stream.hpp"
namespace sfx {
    // represents a MIDI track entry in a MIDI file
    struct midi_track final {
        // the size of the track in bytes
        size_t size;
        // the offset where the track begins
        size_t offset;
    };
    // represents the data in a MIDI file
    class midi_file final {
        void copy(const midi_file& rhs);
    public:
        // The type of MIDI file
        int16_t type;
        // The timebase
        int16_t timebase;
        // The number of tracks
        size_t tracks_size;
        // the track entries
        midi_track* tracks;
        // constructs a new instance
        midi_file();
        // steals an instance
        midi_file(midi_file&& rhs);
        // steals an instance 
        midi_file& operator=(midi_file&& rhs);
        // copies an instance
        midi_file(const midi_file& rhs);
        // copies an instance
        midi_file& operator=(const midi_file& rhs);
        // destroys an instance
        ~midi_file();
        // reads a file from a stream
        static sfx_result read(stream& in, midi_file* out_file);
    };
    // represents a MIDI source from a MIDI file
    class midi_file_source final : public midi_source {
        
        struct source_context {
            midi_event_ex event;
            unsigned long long input_position;
            bool eos;
        };

        midi_file m_file;
        stream* m_stream;
        const uint8_t* m_begin;
        unsigned long long m_elapsed;
        source_context* m_contexts;
        size_t m_next_context;
        void deallocate();
        sfx_result read_next_event();
        midi_file_source(const midi_file_source& rhs)=delete;
        midi_file_source& operator=(const midi_file_source& rhs)=delete;
        midi_file_source(const midi_file& file, stream& input); 
        midi_file_source(const midi_file& file, uint8_t& input);;
    public:
        // constructs a new instance
        midi_file_source();
        // destroys an instance
        ~midi_file_source();
        // steals an instance
        midi_file_source(midi_file_source&& rhs);
        // steals an instance
        midi_file_source& operator=(midi_file_source&& rhs);
        // opens a stream for reading. the stream must be readable and seekable
        static sfx_result open(stream& input,midi_file_source* out_source);
        // gets the MIDI file data associated with this instance
        inline const midi_file& file() const { return m_file; }
        // gets the number of ticks that have elapsed since the last open or reset
        virtual unsigned long long elapsed() const;
        // receives the next event, advancing the input cursor
        virtual sfx_result receive(midi_event* out_event);
        // resets the source to the beginning
        virtual sfx_result reset();
    };
}
#endif // HTCW_SFX_MIDI_FILE_HPP

