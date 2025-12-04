#ifndef DECODER_INTERFACE_H
#define DECODER_INTERFACE_H

#include "../include/cconartist/units.h"
#include "../include/cconartist/property.h"

#if defined(_WIN32)
#    define EXPORT __declspec(dllexport)
#else
#    define EXPORT __attribute__((visibility("default")))
#endif

#endif  // DECODER_INTERFACE_H
