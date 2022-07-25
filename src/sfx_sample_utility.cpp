#include <sfx_sample_utility.hpp>
namespace sfx {
    void sample_utility::mono8_to_stereo16(void** in,void** out, size_t in_sample_count) {
        int8_t* pin = *((int8_t**)in);
        int16_t* pout = *((int16_t**)out);
        for(size_t i = 0;i<in_sample_count;++i) {
            int16_t smp = int16_t(*(pin++)<<8);
            *(pout++)= smp;
            *(pout++)=smp;
        }
    }
    void sample_utility::mono16_to_stereo16(void** in,void** out, size_t in_sample_count) {
        int16_t* pin = *((int16_t**)in);
        int16_t* pout = *((int16_t**)out);
        for(size_t i = 0;i<in_sample_count;++i) {
            int16_t smp = *(pin++);
            *(pout++)= smp;
            *(pout++)= smp;
        }
    }
    void sample_utility::stereo8_to_stereo16(void** in,void** out, size_t in_sample_count) {
        int8_t* pin = *((int8_t**)in);
        int16_t* pout = *((int16_t**)out);
        for(size_t i = 0;i<in_sample_count;++i) {
            *(pout++) = int16_t(*(pin++)<<8);
        }
    }
    void sample_utility::stereo16_to_stereo16(void** in,void** out, size_t in_sample_count) {
        int16_t* pin = *((int16_t**)in);
        int16_t* pout = *((int16_t**)out);
        if(pin==pout) {return;}
        for(size_t i = 0;i<in_sample_count;++i) {
            *(pout++)=*(pin++);
        }
    }
    void sample_utility::stereo16_to_mono8(void** in,void** out, size_t in_sample_count) {
        int16_t* pin = *((int16_t**)in);
        int8_t* pout = *((int8_t**)out);
        for(size_t i = 0;i<in_sample_count;++i) {
            int8_t l = int8_t((*pin)/256);
            ++pin;
            int8_t r = int8_t((*pin)/256);
            ++pin;
            *(pout++)=int8_t((l+r)/2);
        }
    }
    void sample_utility::stereo16_to_mono16(void** in,void** out, size_t in_sample_count) {
        int16_t* pin = *((int16_t**)in);
        int16_t* pout = *((int16_t**)out);
        for(size_t i = 0;i<in_sample_count;++i) {
            int16_t l = *pin;
            ++pin;
            int16_t r = *pin;
            ++pin;
            *(pout++)=int16_t((l+r)/2);
        }
    }
    void sample_utility::stereo16_to_stereo8(void** in,void** out, size_t in_sample_count) {
        int16_t* pin = *((int16_t**)in);
        int8_t* pout = *((int8_t**)out);
        for(size_t i = 0;i<in_sample_count;++i) {
            *(pout++) = int8_t((*pin)/256);
        }
    }
}