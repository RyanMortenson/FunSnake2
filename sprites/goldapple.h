#ifndef GOLDAPPLE_H
#define GOLDAPPLE_H
#include <stdint.h>

#define GOLDAPPLE_BITS_PER_PIXEL 16
#define GOLDAPPLE_LENGTH 256
#define GOLDAPPLE_W 16
#define GOLDAPPLE_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t goldapple[256];

#endif // GOLDAPPLE_H
