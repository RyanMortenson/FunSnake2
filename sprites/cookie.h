#ifndef COOKIE_H
#define COOKIE_H
#include <stdint.h>

#define COOKIE_BITS_PER_PIXEL 16
#define COOKIE_LENGTH 256
#define COOKIE_W 16
#define COOKIE_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t cookie[256];

#endif // COOKIE_H
