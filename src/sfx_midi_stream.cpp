#include <sfx_midi_stream.hpp>
namespace sfx {
const size_t midi_stream::decode_event(bool is_file, stream* in, midi_event_ex* in_out_event) {
    if (in == nullptr || in_out_event == nullptr) {
        return 0;
    }
    int32_t delta;
    size_t result = midi_utility::decode_varlen(in,&delta);
    
    in_out_event->absolute+=delta;
    in_out_event->delta=delta;
    int i = in->getch();
        if(i==-1) {
        return 0;
    }
    ++result;
    uint8_t b = (uint8_t)i;
    if(in_out_event->message.status==0xFF && in_out_event->message.meta.data!=nullptr) {
        free(in_out_event->message.meta.data);
        in_out_event->message.meta.data=nullptr;
    }
    if(in_out_event->message.status==0xF7 && in_out_event->message.sysex.data!=nullptr) {
        free(in_out_event->message.sysex.data);
        in_out_event->message.sysex.data=nullptr;
    }
    bool has_status = b&0x80;
    // expecting a status byte
    if(!has_status) {
        if(!(in_out_event->message.status&0x80)) {
            // no status byte in message
            return 0;
        }
    } else {
        in_out_event->message.status = b;
    }
    switch(in_out_event->message.type()) {
    case midi_message_type::note_off:
    case midi_message_type::note_on:
    case midi_message_type::polyphonic_pressure:
    case midi_message_type::control_change:
    case midi_message_type::pitch_wheel_change:
    case midi_message_type::song_position:
        if(has_status) {
            if(2!=in->read((uint8_t*)&in_out_event->message.value16,2)) {
                return 0;
            }
            result+=2;
            return result;
        }
        i=in->getch();
        if(i==-1) {
            return 0;
        }
        ++result;
        in_out_event->message.lsb(b);
        in_out_event->message.msb((uint8_t)i);    
        return result;
    case midi_message_type::program_change:
    case midi_message_type::channel_pressure:
    case midi_message_type::song_select:
        if(has_status) {
            i=in->getch();
            if(i==-1) {
                return 0;
            }
            ++result;
            in_out_event->message.value8 = (uint8_t)i;
            return result;
        } 
        in_out_event->message.value8 = b;
        
        return  result;
    case midi_message_type::system_exclusive:
        {
            uint8_t* psx = nullptr;
            size_t sxsz = 0;
            uint8_t buf[512];
            uint8_t b = 0;
            int i = 0;
            while(b!=0xF7) {
                if(0==in->read(&b,1)) {
                    if(nullptr!=psx) {
                        free(psx);
                    }
                    return 0;
                }
                ++result;
                buf[i++]=b;
                if(i==512) {
                    sxsz+=512;
                    if(psx==nullptr) {
                        psx=(uint8_t*)malloc(sxsz);
                        if(nullptr==psx) {
                            return 0;
                        }
                    } else {
                        psx=(uint8_t*)realloc(psx,sxsz);
                        if(nullptr==psx) {
                            return 0;
                        }
                    }
                    memcpy(psx+sxsz-512,buf,512);
                    i=0;
                }
            }
            if(i>0) {
                sxsz+=i;
                if(psx==nullptr) {
                    psx=(uint8_t*)malloc(sxsz);
                    if(nullptr==psx) {
                        return 0;
                    }
                } else {
                    psx=(uint8_t*)realloc(psx,sxsz);
                    if(nullptr==psx) {
                        return 0;
                    }
                }
                memcpy(psx+sxsz-i,buf,i);
            }
            in_out_event->message.sysex.data = psx;
            in_out_event->message.sysex.size = sxsz;
            return result;
        }
    case midi_message_type::reset:
        if(!is_file) {
            return result;
        }
        // this is a meta event
            i=in->getch();
            if(i==-1) {
                return 0;
            }
            ++result;
            in_out_event->message.meta.type = (uint8_t)i;
        {
            int32_t vl;
            size_t sz=midi_utility::decode_varlen(in,&vl);
            // re-encode it to fill our midi message
            midi_utility::encode_varlen(vl,in_out_event->message.meta.encoded_length);
            result+=sz;
            if(vl>0) {
                uint8_t* p = (uint8_t*)malloc(vl);
                if(nullptr==p) {
                    return 0;
                }
                if(vl!=in->read(p,vl)) {
                    free(p);
                    return 0;
                }
                result+=vl;
                in_out_event->message.meta.data=p;
                return result;
            }
            in_out_event->message.meta.data = nullptr;
            return result;
        }    
    case midi_message_type::end_system_exclusive:
    case midi_message_type::active_sensing:
    case midi_message_type::start_playback:
    case midi_message_type::stop_playback:
    case midi_message_type::tune_request:
    case midi_message_type::timing_clock:
        return result;
    default:
        return result;
    }
}
}