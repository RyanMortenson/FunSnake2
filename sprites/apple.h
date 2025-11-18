#ifndef APPLE_H
#define APPLE_H
#include <stdint.h>

#define APPLE_BITS_PER_PIXEL 16
#define APPLE_LENGTH 256
#define APPLE_W 16
#define APPLE_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t apple[256];

#endif // APPLE_H
