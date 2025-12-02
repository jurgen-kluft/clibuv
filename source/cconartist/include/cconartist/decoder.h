#ifndef __CCONARTIST_DECODER_PLUGINS_H__
#define __CCONARTIST_DECODER_PLUGINS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "clibuv/uv.h"

namespace ncore
{
    namespace nplugins
    {
        typedef uint8_t       u8;
        typedef uint32_t      u32;
        typedef uint64_t      u64;
        typedef unsigned char byte;

        struct property_t
        {
            const char *m_name;
            int         m_unit;
            double      m_value;
        };

        typedef void *(*decoder_alloc_fn_t)(u32 size, u8 alignment);
        typedef bool (*decoder_decode_fn_t)(const byte *data, u32 size, decoder_alloc_fn_t alloc_func, property_t *out_properties, u32 *out_property_count);

        struct plugin_t
        {
            char                m_name[256];
            decoder_decode_fn_t m_decode_fn;
            void               *m_dlhandle;
        };

        struct registry_t;

        registry_t *create_registry(const char *path_to_folder, u32 max_plugins, uv_loop_t *loop);
        void        destroy_registry(registry_t *registry);
        bool        load_plugin(registry_t *registry, const char *filename);
        bool        unload_plugin(registry_t *registry, const char *filename);
        plugin_t   *find_plugin(registry_t *registry, const char *name);
        void        update_plugins(plugin_t *plugins, u32 count);
    }  // namespace nplugins
}  // namespace ncore

#endif  // __CCONARTIST_DECODER_PLUGINS_H__
