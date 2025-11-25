#ifndef CLASSICDOWN_H
#define CLASSICDOWN_H
#include <stdint.h>

#define CLASSICDOWN_BITS_PER_PIXEL 16
#define CLASSICDOWN_LENGTH 256
#define CLASSICDOWN_W 16
#define CLASSICDOWN_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t classicdown[256];

#endif // CLASSICDOWN_H
