#ifndef CLASSICUP_H
#define CLASSICUP_H
#include <stdint.h>

#define CLASSICUP_BITS_PER_PIXEL 16
#define CLASSICUP_LENGTH 256
#define CLASSICUP_W 16
#define CLASSICUP_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t classicup[256];

#endif // CLASSICUP_H
