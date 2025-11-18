#ifndef CHERRY_H
#define CHERRY_H
#include <stdint.h>

#define CHERRY_BITS_PER_PIXEL 16
#define CHERRY_LENGTH 256
#define CHERRY_W 16
#define CHERRY_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t cherry[256];

#endif // CHERRY_H
