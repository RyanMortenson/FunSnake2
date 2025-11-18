#ifndef GRAYDOWN_H
#define GRAYDOWN_H
#include <stdint.h>

#define GRAYDOWN_BITS_PER_PIXEL 16
#define GRAYDOWN_LENGTH 256
#define GRAYDOWN_W 16
#define GRAYDOWN_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t graydown[256];

#endif // GRAYDOWN_H
