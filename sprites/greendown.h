#ifndef GREENDOWN_H
#define GREENDOWN_H
#include <stdint.h>

#define GREENDOWN_BITS_PER_PIXEL 16
#define GREENDOWN_LENGTH 256
#define GREENDOWN_W 16
#define GREENDOWN_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t greendown[256];

#endif // GREENDOWN_H
