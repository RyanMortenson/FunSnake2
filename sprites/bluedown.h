#ifndef BLUEDOWN_H
#define BLUEDOWN_H
#include <stdint.h>

#define BLUEDOWN_BITS_PER_PIXEL 16
#define BLUEDOWN_LENGTH 256
#define BLUEDOWN_W 16
#define BLUEDOWN_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t bluedown[256];

#endif // BLUEDOWN_H
