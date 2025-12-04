#ifndef __CCONARTIST_DECODER_PROPERTY_H__
#define __CCONARTIST_DECODER_PROPERTY_H__

enum EPropertyType
{
    PTBool   = 0,
    PTInt64  = 1,
    PTDouble = 2,
    PTString = 3,
};

typedef void* (*decoder_alloc_fn_t)(unsigned int size, unsigned char alignment);

struct decoder_property_t
{
    const char* m_name;  // Null-terminated UTF-8 string
    int         m_unit;  // EPropertyUnit
    int         m_type;  // EPropertyType
    union                // The value
    {
        bool        m_bool;
        long long   m_int64;
        double      m_double;
        const char* m_string;
    } m_value;
};

#endif  // __CCONARTIST_DECODER_PROPERTY_H__
