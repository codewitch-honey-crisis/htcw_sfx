#include <htcw_endian.hpp>
#include <sfx_midi_file.hpp>
namespace sfx {
// implement std::move to limit dependencies on the STL, which may not be there
template< class T > struct midi_file_remove_reference      { typedef T type; };
template< class T > struct midi_file_remove_reference<T&>  { typedef T type; };
template< class T > struct midi_file_remove_reference<T&&> { typedef T type; };
template <typename T>
typename midi_file_remove_reference<T>::type&& midi_file_move(T&& arg) {
    return static_cast<typename midi_file_remove_reference<T>::type&&>(arg);
}
// reads a chunk out of a multipart chunked file (MIDI file, basically)
bool midi_file_read_chunk_part(stream& in,size_t* in_out_offset, size_t* out_size) {
    uint32_t tmp;
    // read the size
    if(4!=in.read((uint8_t*)&tmp,4)) {
        return false;
    }
    *in_out_offset+=4;
    if(bits::endianness()==bits::endian_mode::little_endian) {
        tmp = bits::swap(tmp);
    }
    *out_size=(size_t)tmp;
    return true;
}
// implement copy constructor/assignment operator
void midi_file::copy(const midi_file& rhs) {
    type = rhs.type;
    timebase = rhs.timebase;
    tracks_size = rhs.tracks_size;
    const size_t sz = sizeof(midi_track)*tracks_size;
    tracks = (midi_track*)malloc(sz);
    if(tracks!=nullptr) {
        memcpy(tracks,rhs.tracks,sz);
    }
}
midi_file::midi_file(const midi_file& rhs) {
    copy(rhs);
}
midi_file& midi_file::operator=(const midi_file& rhs) {
    if(tracks!=nullptr) {
        free(tracks);
    }
    copy(rhs);
    return *this;
}
// create an empy instance
midi_file::midi_file() : type(0),timebase(0),tracks_size(0),tracks(nullptr) {
}
// steal
midi_file::midi_file(midi_file&& rhs) {
    type = rhs.type;
    timebase = rhs.timebase;
    tracks_size = rhs.tracks_size;
    tracks = rhs.tracks;
    rhs.tracks = nullptr;
    rhs.tracks_size = 0;
}
// steal
midi_file& midi_file::operator=(midi_file&& rhs) {
    if(tracks!=nullptr) {
        free(tracks);
    }
    type = rhs.type;
    timebase = rhs.timebase;
    tracks_size = rhs.tracks_size;
    tracks = rhs.tracks;
    rhs.tracks = nullptr;
    rhs.tracks_size = 0;
    return *this;
}
// destroy
midi_file::~midi_file() {
    if(tracks!=nullptr) {
        free(tracks);
    }
}
// read file info from a stream
sfx_result midi_file::read(stream& in, midi_file* out_file) {
    // MIDI files are a series of "chunks" that are a 4 byte ASCII string
    // "magic" identifier, and a 4 byte integer size, followed by 
    // that many bytes of data. After that is the next chunk
    // or the end of the file.
    // the two relevant chunks are MThd (always size of 6)
    // that contains the MIDI file global info
    // and then MTrk for each MIDI track in the file
    // chunks with any other magic id are ignored.
    if(out_file==nullptr) {
        return sfx_result::invalid_argument;
    }
    if(!in.caps().read) {
        return sfx_result::io_error;
    }
    int16_t tmp;
    union {
        uint32_t magic_id;
        char magic[5];
    } m;
    m.magic[4]=0;
    size_t pos = 0;
    size_t sz;
    if(4!=in.read((uint8_t*)m.magic,4)) {
        return sfx_result::invalid_format;
    }
    if(0!=strcmp(m.magic,"MThd")) {
        return sfx_result::invalid_format;
    }
    pos+=4;
    if(!midi_file_read_chunk_part(in,&pos,&sz) || 6!=sz) {
        return sfx_result::invalid_format;
    }
    
    if(2!=in.read((uint8_t*)&tmp,2)) {
        return sfx_result::end_of_stream;
    }
    if(bits::endianness()==bits::endian_mode::little_endian) {
        tmp = bits::swap(tmp);
    }
    pos+=2;
    out_file->type = tmp;

    if(2!=in.read((uint8_t*)&tmp,2)) {
        return sfx_result::end_of_stream;
    }
    if(bits::endianness()==bits::endian_mode::little_endian) {
        tmp = bits::swap(tmp);
    }
    pos+=2;
    out_file->tracks_size = tmp;
    if(2!=in.read((uint8_t*)&tmp,2)) {
        return sfx_result::end_of_stream;
    }
    if(bits::endianness()==bits::endian_mode::little_endian) {
        tmp = bits::swap(tmp);
    }
    pos+=2;
    out_file->timebase = tmp;
    out_file->tracks = (midi_track*)malloc(sizeof(midi_track)*out_file->tracks_size);
    if(out_file->tracks==nullptr) {
        return sfx_result::out_of_memory;
    }
    size_t i = 0;
    while(i<out_file->tracks_size) {
        if(4!=in.read((uint8_t*)m.magic,4)) {
            if(out_file->tracks_size==i) {
                return sfx_result::success;
            }
            return sfx_result::invalid_format;
        }
        pos+=4;
        if(!midi_file_read_chunk_part(in,&pos,&sz)) {
            return sfx_result::end_of_stream;
        }
        if(0==strcmp(m.magic,"MTrk")) {
            out_file->tracks[i].offset=pos;
            out_file->tracks[i].size=sz;
            ++i;
        } 
        if(in.caps().seek) {
            in.seek(sz,io::seek_origin::current);
        } else {
            for(int j = 0;j<sz;++j) {
                if(-1==in.getch()) {
                    return sfx_result::end_of_stream;            
                }
            }
        }
        pos+=sz;
    }
    if(i==out_file->tracks_size) {
        return sfx_result::success;
    }
    if(i<out_file->tracks_size) {
        return sfx_result::end_of_stream;
    }
    return sfx_result::invalid_format;   
}


// construct an instance given a file and stream
midi_file_source::midi_file_source(const midi_file& file, stream& input) : m_file(file), m_stream(&input),m_elapsed(0),m_contexts(nullptr),m_next_context(0) {
    
}

void midi_file_source::deallocate() {
    if(m_contexts!=nullptr) {
        free(m_contexts);
        m_contexts = nullptr;
    }
}

midi_file_source::midi_file_source() : m_stream(nullptr), m_elapsed(0), m_contexts(nullptr) {

}
midi_file_source::~midi_file_source() {
    deallocate();
}
midi_file_source::midi_file_source(midi_file_source&& rhs) {
    m_file = rhs.m_file;
    m_stream = rhs.m_stream;
    m_elapsed = rhs.m_elapsed;
    m_contexts = rhs.m_contexts;
    rhs.m_contexts = nullptr;
    m_next_context = rhs.m_next_context;
}
midi_file_source& midi_file_source::operator=(midi_file_source&& rhs) {
    deallocate();
    m_file = rhs.m_file;
    m_stream = rhs.m_stream;
    m_elapsed = rhs.m_elapsed;
    m_contexts = rhs.m_contexts;
    rhs.m_contexts = nullptr;
    m_next_context = rhs.m_next_context;
    return *this;
}
sfx_result midi_file_source::open(stream& input,midi_file_source* out_source) {
    if(out_source==nullptr) {
        return sfx_result::invalid_argument;
    }
    if(!input.caps().read || !input.caps().seek) {
        return sfx_result::io_error;
    }
    sfx_result r= midi_file::read(input,&out_source->m_file);
    if(r!=sfx_result::success) {
        return r;
    }
    out_source->m_stream = &input;
    out_source->m_contexts = (source_context*)calloc(out_source->m_file.tracks_size,sizeof(source_context));
    if(nullptr==out_source->m_contexts) {
        return sfx_result::out_of_memory;
    }
    
    return out_source->reset();
}

unsigned long long midi_file_source::elapsed() const {
    return m_elapsed;
}
sfx_result midi_file_source::read_next_event() {
    size_t tsz = m_file.tracks_size;
    if(m_next_context==tsz) {
        return sfx_result::end_of_stream;
    }
    // find the next context we're 
    // pulling the message from
    source_context* ctx = m_contexts+m_next_context;
    if(ctx->eos) {
        return sfx_result::end_of_stream;
    }
    // seek to the current input position
    if(ctx->input_position!=m_stream->seek(ctx->input_position)) {
        return sfx_result::io_error;
    }
    // set the end of stream flag if we're there
    if(ctx->input_position-m_file.tracks[m_next_context].offset>=m_file.tracks[m_next_context].size) {
        ctx->eos = true;
    } else {
        // decode the next event
        size_t sz = midi_stream::decode_event(true,*m_stream,&ctx->event);
        if(sz==0) {
            return sfx_result::invalid_format;
        }
        // increment the position
        ctx->input_position+=sz;
    }
    // find the context with the nearest absolutely positioned
    // event and store the index of it for later
    bool done = true;
    m_next_context = tsz;
    unsigned long long pos = (unsigned long long)-1;
    for(int i = 0;i<(int)tsz;++i) {   
        ctx = m_contexts+i;
        if(!ctx->eos) {
            if(ctx->event.message.status!=0 && ctx->event.absolute<pos) {
                m_next_context=i;
                pos = ctx->event.absolute;
                done = false;
            }
        }    
    }
    return done?sfx_result::end_of_stream:sfx_result::success;
}
sfx_result midi_file_source::reset() {
    if(m_stream==nullptr) {
        return sfx_result::invalid_state;
    }
    // reset the count of elapsed ticks
    m_elapsed = 0;
    // fill the contexts
    const size_t tsz = m_file.tracks_size;
    for(int i = 0;i<tsz;++i) {
        source_context* ctx = m_contexts+i;
        ctx->input_position = m_file.tracks[i].offset;
        // set the end flag in the case of a zero length track
        ctx->eos = !m_file.tracks[i].size;
        ctx->event.absolute = 0;
        ctx->event.delta = 0;
        ctx->event.message = midi_file_move(midi_message());
        // decode the first event
        if(!ctx->eos && ctx->input_position ==m_stream->seek(ctx->input_position)) {
            if(0!=midi_stream::decode_event(true,*m_stream,&ctx->event)) {
                ctx->input_position = m_stream->seek(0,seek_origin::current);
            }    
        }  
    }
    // now go through the contexts and find the one with
    // the nearest absolute position.
    m_next_context = m_file.tracks_size;
    unsigned long long pos = (unsigned long long)-1;
    for(int i = 0;i<(int)tsz;++i) {   
        source_context* ctx = m_contexts+i;
        if(!ctx->eos) {
            if(ctx->event.absolute<pos) {
                m_next_context=i;
                pos = ctx->event.absolute;
            }
        }    
    }
    return m_next_context==m_file.tracks_size?
            sfx_result::end_of_stream:sfx_result::success;
}
sfx_result midi_file_source::receive(midi_event* out_event) {
    if(m_stream==nullptr) {
        return sfx_result::invalid_state;
    }
    if(m_next_context==m_file.tracks_size) {
        return sfx_result::end_of_stream;
    }
    
    source_context* ctx = m_contexts+m_next_context;
    if(ctx->eos) {
        return sfx_result::end_of_stream;
    }
    out_event->delta = (int32_t)ctx->event.absolute-m_elapsed;
    // the midi_file_move will cause these values
    // to potentially be zeroed so we preserve 
    // them:
    uint8_t status = ctx->event.message.status;
    uint8_t type = ctx->event.message.meta.type;
    out_event->message = midi_file_move(ctx->event.message);
    // set a running status byte
    ctx->event.message.status = status; 
    // set the meta type
    ctx->event.message.meta.type = type; 
    // don't need anything else
    
    // advance the elapsed ticks
    m_elapsed = ctx->event.absolute;
    
    // refill our contexts
    sfx_result r =read_next_event();
    if(r==sfx_result::end_of_stream) {
        return sfx_result::success;
    }
    return r;
}
}