#ifndef GRAYBODY_H
#define GRAYBODY_H
#include <stdint.h>

#define GRAYBODY_BITS_PER_PIXEL 16
#define GRAYBODY_LENGTH 256
#define GRAYBODY_W 16
#define GRAYBODY_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t graybody[256];

#endif // GRAYBODY_H
