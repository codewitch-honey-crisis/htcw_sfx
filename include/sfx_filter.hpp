#ifndef HTCW_SFX_FILTER_HPP
#define HTCW_SFX_FILTER_HPP
#include "sfx_core.hpp"
namespace sfx {

typedef void(*filter_callback)(void* samples,size_t sample_count, void* state);
template<size_t Filters=1,size_t SampleRate=44100,size_t Channels=2, size_t BitDepth = 16>
class filter_source : public audio_source {
    static_assert(SampleRate>0,"SampleRate must be greater than zero");
    static_assert(Channels==1||Channels==2,"Channels must be 1 or 2");
    static_assert(BitDepth==8||BitDepth==16,"BitDepth must be 8 or 16");    
    static_assert(Filters>0,"Filters must be greater than zero");
    constexpr static const size_t sample_rate_ = SampleRate;
    constexpr static const size_t channels_ = Channels;
    constexpr static const size_t bit_depth_ = BitDepth;
public:
    using type = filter_source;
    constexpr static const size_t filters = Filters;
private:
    audio_source* m_source;
    struct filter_cb_info {
        filter_callback callback;
        void* state;
    };
    filter_cb_info m_filters[filters];
public:
    virtual size_t bit_depth() const {return bit_depth_;}
    virtual size_t sample_rate() const { return sample_rate_; }
    virtual size_t channels() const { return channels_; }
    virtual audio_format format() const { return audio_format::pcm; }

    audio_source* source() const {
        return m_source;
    }
    void source(audio_source* value) {
        m_source = value;
    }
    filter_callback filter(size_t index,void** out_state = nullptr) {
        if(0>index||index>=filters) {
            return nullptr;
        }
        if(out_state!=nullptr) {
            *out_state = m_filters[index].state;
        }
        return m_filters[index].callback;
    }
    bool filter(size_t index,filter_callback callback, void* state = nullptr) {
        if(0>index||index>=filters) {
            return false;
        }
        m_filters[index].callback = callback;
        m_filters[index].state = state;
        return true;
    }
    virtual size_t read(void* samples,size_t sample_count) {
        size_t read = m_source->read(samples,sample_count);
        for(int i = 0;i<filters;++i) {
            if(m_filters[i].callback!=nullptr) {
                m_filters[i].callback(samples,read,m_filters[i].state);
            }
        }
        return read;
    }
    static sfx_result create(filter_source* out_filter, audio_source* source = nullptr) {
        if(out_filter==nullptr) {
            return sfx_result::invalid_argument;
        }
        for(int i = 0; i <filters;++i) {
            out_filter->m_filters[i].callback = nullptr;
            out_filter->m_filters[i].state = nullptr;
        }
        out_filter->m_source = source;
        return sfx_result::success;
    }
    static void x_pass(void* samples,size_t sample_count, double alpha1,double alpha2) {
        switch(channels_) {
        case 1:
            switch(bit_depth_) {
                case 8: {
                    int8_t*p=(int8_t*)samples;
                    for(int i = 1; i < sample_count; ++i) {
                        int32_t smp = p[i-1]*alpha1 + (alpha2*(p[i] - p[i-1])); 
                        if(smp<-128) {
                            smp = -128;
                        } else if(smp>127) {
                            smp=127;
                        }
                        p[i] = smp;
                    }
                }
                break;
                case 16: {
                    int16_t*p=(int16_t*)samples;
                    for(int i = 1; i < sample_count; ++i) {
                        int32_t smp = p[i-1]*alpha1 + (alpha2*(p[i] - p[i-1])); 
                        if(smp<-32768) {
                            smp = -32768;
                        } else if(smp>32767) {
                            smp=32767;
                        }
                        p[i] = smp;
                    }
                }
                break;
            }
            break;
        case 2:
            switch(bit_depth_) {
                case 8: {
                    int8_t*p=(int8_t*)samples;
                    for(int i = 3; i < sample_count; i+=2) {
                        // p[i] = p[i-1] + (alpha*(p[i] - p[i-1])); 
                        int8_t* ppl=p-3;
                        int8_t* ppr=p-2;
                        int8_t* pl=p-1;
                        int8_t* pr=p;
                        int32_t smp = ppl[i]*alpha1+(alpha2*(pl[i]-ppl[i]));
                        if(smp<-128) {
                            smp = -128;
                        } else if(smp>127) {
                            smp=127;
                        }
                        pl[i]=(int8_t)smp;
                        smp = ppr[i]*alpha1+(alpha2*(pr[i]-ppr[i]));
                        if(smp<-128) {
                            smp = -128;
                        } else if(smp>127) {
                            smp=127;
                        }
                        pr[i]=(int8_t)smp;
                    }
                }
                break;
                case 16: {
                    int16_t*p=(int16_t*)samples;
                    for(int i = 3; i < sample_count; i+=2) {
                        // p[i] = p[i-1] + (alpha*(p[i] - p[i-1])); 
                        int16_t* ppl=p-3;
                        int16_t* ppr=p-2;
                        int16_t* pl=p-1;
                        int16_t* pr=p;
                        int32_t smp = ppl[i]*alpha1+(alpha2*(pl[i]-ppl[i]));
                        if(smp<-32768) {
                            smp = -32768;
                        } else if(smp>32767) {
                            smp=32767;
                        }
                        pl[i]=(int16_t)smp;
                        smp = ppr[i]*alpha1+(alpha2*(pr[i]-ppr[i]));
                        if(smp<-32768) {
                            smp = -32768;
                        } else if(smp>32767) {
                            smp=32767;
                        }
                        pr[i]=(int16_t)smp;
                    }
                }
                break;
            }
            break;
        }
    }
    static void low_pass(void* samples,size_t sample_count,void* cutoff)
    {
        float co = *(float*)cutoff;
        double RC = 1.0/(co*2*3.14);
        double dt = 1.0/sample_rate_;  
        double alpha = dt/(RC+dt); 
        
        x_pass(samples,sample_count,1.0,alpha);
    }
    static void high_pass(void* samples,size_t sample_count,void* cutoff)
    {
        float co = *(float*)cutoff;
        double RC = 1.0/(co*2*3.14);
        double dt = 1.0/sample_rate_;  
        double alpha = RC/(RC+dt); 
        
        x_pass(samples,sample_count,alpha,alpha);
    }
};
}
#endif