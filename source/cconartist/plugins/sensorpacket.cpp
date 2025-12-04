#include "decoder_interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void decode(const unsigned char* data, unsigned int size, decoder_alloc_fn_t alloc_func, decoder_property_t** out_properties, short* out_property_count)
    {
        *out_property_count = 0;

        decoder_property_t* prop = (decoder_property_t*)alloc_func(sizeof(decoder_property_t), alignof(decoder_property_t));
        prop->m_name             = "Temperature";
        prop->m_unit             = UTemperature;
        prop->m_type             = PTDouble;
        prop->m_value.m_double   = 42.0;
        *out_property_count += 1;

        out_properties = (decoder_property_t**)alloc_func(sizeof(decoder_property_t*) * (*out_property_count), alignof(decoder_property_t*));
    }

#ifdef __cplusplus
}
#endif
