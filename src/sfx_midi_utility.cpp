#include <sfx_midi_utility.hpp>
namespace sfx {
const uint8_t* midi_utility::decode_varlen(const uint8_t* in, int32_t* out_value) {
    uint8_t c;
    uint32_t value;
    if ((value = *(in++)) & 0x80) {
        value &= 0x7f;
        do {
            value = (value << 7) + ((c = *(in++)) & 0x7f);
        } while (c & 0x80);
    }
    *out_value = value;
    return in;
}
size_t midi_utility::decode_varlen(stream& in, int32_t* out_value) {
    uint8_t c;
    uint32_t value;
    size_t result = 1;
    if ((value = (uint8_t)in.getch()) & 0x80) {
        value &= 0x7f;
        do {
            value = (value << 7) + ((c = (uint8_t)in.getch()) & 0x7f);
            ++result;
        } while (c & 0x80 && result < 4);
    }
    *out_value = value;
    
    return result > 3 ? 0 : result;
}
uint8_t* midi_utility::encode_varlen(int32_t value, uint8_t* out) {
    uint32_t buffer;
    buffer = value & 0x7f;
    while ((value >>= 7) > 0) {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7f);
    }

    while (true) {
        *(out++) = buffer;
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
    return out;
}
size_t midi_utility::encode_varlen(int32_t value, stream* out) {
    uint32_t buffer;
    buffer = value & 0x7f;
    size_t result = 0;
    while ((value >>= 7) > 0) {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7f);
    }

    while (true) {
        out->putch(buffer);
        ++result;
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
    return result;
}
}  // namespace sfx