#ifndef GREENAPPLE_H
#define GREENAPPLE_H
#include <stdint.h>

#define GREENAPPLE_BITS_PER_PIXEL 16
#define GREENAPPLE_LENGTH 256
#define GREENAPPLE_W 16
#define GREENAPPLE_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t greenapple[256];

#endif // GREENAPPLE_H
