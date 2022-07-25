#include <sfx_wav_file.hpp>
namespace sfx {
    void sfx_wav_file_zero_samples(size_t bit_depth,void* samples,size_t sample_count) {
        switch(bit_depth) {
            case 8: {
                int8_t* p = (int8_t*)samples;
                for(size_t i = 0;i<sample_count;++i) {
                    *(p++)=-128;
                }
                break;
            }
            case 16: {
                int16_t* p = (int16_t*)samples;
                for(size_t i = 0;i<sample_count;++i) {
                    *(p++)=-32768;
                }
                break;
            }
        }
    }
    sfx_result sfx_wav_file_read_riff_chunk(stream& in,uint8_t* buffer,uint32_t* out_length) {
        if(4!=in.read(buffer,4)) {
            return sfx_result::end_of_stream;
        }
        if(4!=in.read((uint8_t*)out_length,4)) {
            return sfx_result::end_of_stream;
        }
        if(bits::endianness()==bits::endian_mode::big_endian) {
            *out_length=bits::swap(*out_length);
        }
        return sfx_result::success;
    }
    sfx_result sfx_wav_file_skip_riff_chunk(stream& in,uint32_t length) {
        if(in.caps().seek) {
            in.seek((long long)length,seek_origin::current);
            return sfx_result::success;
        }
        while(length--) {
            in.getch();
        }
        return sfx_result::success;
    }
    sfx_result wav_file::read(stream& in, wav_file* out_file) {
        if(out_file==nullptr) {
            return sfx_result::invalid_argument;
        }
        if(!in.caps().read) {
            return sfx_result::io_error;
        }
        uint8_t buffer[4];
        uint32_t tmp32;
        uint16_t tmp16;
        uint32_t len;
        sfx_result r;
        if(4!=in.read(buffer,4)) {
            return sfx_result::invalid_format;
        }
        // fourcc
        if(buffer[0]!='R'||buffer[1]!='I'||buffer[2]!='F'||buffer[3]!='F') {
            return sfx_result::invalid_format;
        }
        // skip file length
        if(4!=in.read((uint8_t*)&tmp32,4)) {
            return sfx_result::end_of_stream;
        }
        if(4!=in.read(buffer,4)) {
            return sfx_result::end_of_stream;
        }
        if(buffer[0]!='W'||buffer[1]!='A'||buffer[2]!='V'||buffer[3]!='E') {
            return sfx_result::invalid_format;
        }
        
        while(true) {
            r=sfx_wav_file_read_riff_chunk(in,buffer,&len);
            if(r!=sfx_result::success) {
                return r;
            }
            if(buffer[0]=='f'&&buffer[1]=='m'&&buffer[2]=='t'&&buffer[3]==' ') {
                if(len!=16) {
                    return sfx_result::invalid_format;
                }
                if(2!=in.read((uint8_t*)&tmp16,2)) {
                    return sfx_result::end_of_stream;
                }
                if(bits::endianness()==bits::endian_mode::big_endian) {
                    tmp16=bits::swap(tmp16);
                }
                out_file->format = tmp16;
                if(2!=in.read((uint8_t*)&tmp16,2)) {
                    return sfx_result::end_of_stream;
                }
                if(bits::endianness()==bits::endian_mode::big_endian) {
                    tmp16=bits::swap(tmp16);
                }
                out_file->channels = tmp16;
                if(4!=in.read((uint8_t*)&tmp32,4)) {
                    return sfx_result::end_of_stream;
                }
                if(bits::endianness()==bits::endian_mode::big_endian) {
                    tmp32=bits::swap(tmp32);
                }
                out_file->sample_rate = tmp32;
                // skip (we don't need it)
                if(4!=in.read((uint8_t*)&tmp32,4)) {
                    return sfx_result::end_of_stream;
                }
                // skip
                if(2!=in.read((uint8_t*)&tmp16,2)) {
                    return sfx_result::end_of_stream;
                }
                if(2!=in.read((uint8_t*)&tmp16,2)) {
                    return sfx_result::end_of_stream;
                }
                if(bits::endianness()==bits::endian_mode::big_endian) {
                    tmp16=bits::swap(tmp16);
                }
                out_file->bit_depth = tmp16;
            } else if(buffer[0]=='d'&&buffer[1]=='a'&&buffer[2]=='t'&&buffer[3]=='a') {
                out_file->data_offset = 44;
                out_file->data_size = len;
                return sfx_result::success;
            } else {
                r=sfx_wav_file_skip_riff_chunk(in,len);
                if(r!=sfx_result::success) {
                    return r;
                }
            }
        }
        if(4!=in.read((uint8_t*)&tmp32,4)) {
            return sfx_result::end_of_stream;
        }
        if(bits::endianness()==bits::endian_mode::big_endian) {
            tmp32=bits::swap(tmp32);
        }
        out_file->data_offset = 44;
        out_file->data_size = tmp32;
        return sfx_result::success;
    }
    sfx_result wav_file_source::open(stream& in, wav_file_source* out_source) {
        sfx_result r = wav_file::read(in,&out_source->m_file);
        if(r!=sfx_result::success) {
            return r;
        }
        out_source->m_ended=false;
        out_source->m_ended_callback = nullptr;
        out_source->m_ended_callback_state = nullptr;
        out_source->m_loop = false;
        out_source->m_stream = &in;
        out_source->m_position = 0;
        return sfx_result::success;
    }
    size_t wav_file_source::bit_depth() const {
        return (size_t)m_file.bit_depth;
    }
    size_t wav_file_source::sample_rate() const {
        return (size_t)m_file.sample_rate;
    }
    size_t wav_file_source::channels() const {
        return (size_t)m_file.channels;
    }
    audio_format wav_file_source::format() const {
        return audio_format::pcm;
    }
    bool wav_file_source::can_loop() const {
        if(m_stream==nullptr) {
            return false;
        }
        return m_stream->caps().seek;
    }
    sfx_result wav_file_source::loop(bool value) {
        if(m_stream==nullptr) {
            return sfx_result::invalid_state;
        }
        if(value) {
            if(!m_stream->caps().seek) {
                return sfx_result::not_supported;
            }
        }
        m_loop = value;
        return sfx_result::success;
    }
    size_t wav_file_source::read(void* samples, size_t sample_count) {
        size_t bytes_to_read = ((m_file.bit_depth*sample_count)+7)/8;
        if(m_position>=m_file.data_size) {
            sfx_wav_file_zero_samples(bit_depth(),samples,sample_count);
        }
        if(bytes_to_read+m_position>m_file.data_size) {
            bytes_to_read = m_file.data_size - m_position;
        }
        size_t bytes_read = m_stream->read((uint8_t*)samples,bytes_to_read);
        
        m_position += bytes_read;
        size_t samples_read = (bytes_read*8)/m_file.bit_depth;
        if(samples_read<sample_count) {
            if(!m_ended) {
                m_ended = true;
                if(m_ended_callback!=nullptr) {
                    m_ended_callback(m_ended_callback_state);
                }
            }
            if(m_loop) {
                m_ended = false;
                m_position = (unsigned long long)m_stream->seek(m_file.data_offset)-m_file.data_offset;
                return read(((uint8_t*)samples)+bytes_read,sample_count-samples_read)+samples_read;
            } else {
                sfx_wav_file_zero_samples(bit_depth(),((uint8_t*)samples)+bytes_read,bytes_to_read-bytes_read);
            }
        }
        return samples_read;
    }
}