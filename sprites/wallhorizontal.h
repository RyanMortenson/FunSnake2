#ifndef WALLHORIZONTAL_H
#define WALLHORIZONTAL_H
#include <stdint.h>

#define WALLHORIZONTAL_BITS_PER_PIXEL 16
#define WALLHORIZONTAL_LENGTH 256
#define WALLHORIZONTAL_W 16
#define WALLHORIZONTAL_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t wallhorizontal[256];

#endif // WALLHORIZONTAL_H
