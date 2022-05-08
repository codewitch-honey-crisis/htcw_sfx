#include <string.h>
#include <sfx_midi_message.hpp>

namespace sfx {
void midi_message::copy(const midi_message& rhs) {
        memcpy(this,&rhs,sizeof(midi_message));
        midi_message_type type = rhs.type();
        
        if(type==midi_message_type::system_exclusive) {
            if(rhs.sysex.data!=nullptr) {
                sysex.data = (uint8_t*)malloc(rhs.sysex.size);
                if(sysex.data==nullptr) {
                    return;
                }
                memcpy(sysex.data,rhs.sysex.data,rhs.sysex.size);
            }    
        } else if(type==midi_message_type::meta_event) {
            if(rhs.meta.data!=nullptr) {
                int32_t l;
                midi_utility::decode_varlen(rhs.meta.encoded_length,&l);
                meta.data = (uint8_t*)malloc((size_t)l);
                if(meta.data==nullptr) {
                    return;
                }
                memcpy(meta.data,rhs.meta.data,(size_t)l);
            }
        }
}
void midi_message::deallocate() {
    switch(status) {
        case 0xFF:
            if(meta.data!=nullptr) {
                free(meta.data);
                meta.data = nullptr;
            }
            break;
        case 0xF7:
            if(sysex.data!=nullptr) {
                free(sysex.data);
                sysex.data=nullptr;
            }
            break;
        default:
            break;
    }
}
}