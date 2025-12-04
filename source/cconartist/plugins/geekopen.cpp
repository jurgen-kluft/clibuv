#include "decoder_interface.h"

#include <cstdlib>
#include <cstring>

#ifdef __cplusplus
extern "C"
{
#endif

    void decode(const unsigned char* data, uint32_t size, decoder_alloc_fn_t alloc_func, decoder_property_t** out_properties, short* out_property_count)
    {
        // Geekopen packets are JSON, but we don't want to have a full blown JSON parser here, so we do it manually.
        // So in all observerd cases they are flat JSON objects with string keys and either string or numeric values.

        // Examples:
        // {"online":1}
        // {"messageId":"","key":0,"mac":"8CCE4E50AF57","voltage":225.415,"current":0,"power":0.102,"energy":0.028}
        // {"messageId":"","mac":"D48AFC3A53DE","type":"Zero-2","version":"2.1.2","wifiLock":0,"keyLock":0,"ip":"192.168.8.93","ssid":"OBNOSIS8","key1":0,"key3":0}

        *out_property_count = 0;

        const unsigned char* ptr = data;
        const unsigned char* end = data + size;

        const int max_properties = 16;

        decoder_property_t* properties = (decoder_property_t*)alloc_func(sizeof(decoder_property_t) * max_properties, alignof(decoder_property_t));

        while (ptr < end && (*ptr == ' ' || *ptr == '{'))
            ptr++;

        short count = 0;

        while (ptr < end && *ptr != '}' && count < max_properties)
        {
            // Skip whitespace and quotes
            while (ptr < end && (*ptr == ' ' || *ptr == '\"'))
                ptr++;

            // Parse key
            const unsigned char* key_start = ptr;
            while (ptr < end && *ptr != '\"')
                ptr++;
            uint32_t key_len = (uint32_t)(ptr - key_start);

            char* key = (char*)alloc_func(key_len + 1, alignof(char));
            for (uint32_t i = 0; i < key_len; i++)
                key[i] = (char)key_start[i];
            key[key_len] = '\0';

            while (ptr < end && *ptr != ':')
                ptr++;
            ptr++;  // skip ':'

            while (ptr < end && (*ptr == ' '))
                ptr++;

            // Parse value
            EPropertyType type;
            union
            {
                int64_t i64;
                double  d;
                bool    b;
                char*   str;
            } val;

            if (*ptr == '\"')
            {
                // String value
                ptr++;
                const unsigned char* value_start = ptr;
                while (ptr < end && *ptr != '\"')
                    ptr++;
                uint32_t value_len = (uint32_t)(ptr - value_start);

                char* str_val = (char*)alloc_func(value_len + 1, alignof(char));
                for (uint32_t i = 0; i < value_len; i++)
                    str_val[i] = (char)value_start[i];
                str_val[value_len] = '\0';

                type    = PTString;
                val.str = str_val;
                ptr++;  // skip closing quote
            }
            else
            {
                // Numeric or boolean
                const unsigned char* value_start = ptr;
                while (ptr < end && *ptr != ',' && *ptr != '}')
                    ptr++;
                uint32_t value_len = (uint32_t)(ptr - value_start);

                // Copy value into temp buffer
                char         temp[64];
                uint32_t copy_len = (value_len < 63) ? value_len : 63;
                for (uint32_t i = 0; i < copy_len; i++)
                    temp[i] = (char)value_start[i];
                temp[copy_len] = '\0';

                if (temp[0] == 't' || temp[0] == 'f')
                {
                    type  = PTBool;
                    val.b = (temp[0] == 't');
                }
                else if (strchr(temp, '.') != NULL)
                {
                    type  = PTDouble;
                    val.d = atof(temp);
                }
                else
                {
                    type    = PTInt64;
                    val.i64 = atoll(temp);
                }
            }

            // Fill property
            properties[count].m_name  = key;
            properties[count].m_type = type;
            switch (type)
            {
                case PTBool: properties[count].m_value.m_bool = val.b; break;
                case PTInt64: properties[count].m_value.m_int64 = val.i64; break;
                case PTDouble: properties[count].m_value.m_double = val.d; break;
                case PTString: properties[count].m_value.m_string = val.str; break;
            }

            // Can we determine the unit?
            // For the following properties we can set specific units:
            //   "voltage"  -> UVolts
            //   "current"  -> UAmperes
            //   "power"    -> UWatts
            //   "energy"   -> UWh
            //   "wifiLock" -> UOnOff
            //   "keyLock"  -> UOnOff
            //   "key1"     -> UOnOff
            //   "key2"     -> UOnOff
            //   "key3"     -> UOnOff
            //   "online"   -> UTrueFalse
            //   "ip"       -> UString
            //   "ssid"     -> UString
            //   "mac"      -> UString
            //   "type"     -> UString
            //   "version"  -> UString
            //   "messageId"-> UString

            count++;

            // Skip comma
            while (ptr < end && (*ptr == ',' || *ptr == ' '))
                ptr++;
        }

        *out_properties     = properties;
        *out_property_count = count;
    }

#ifdef __cplusplus
}
#endif
