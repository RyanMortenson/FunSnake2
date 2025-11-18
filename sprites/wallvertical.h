#ifndef WALLVERTICAL_H
#define WALLVERTICAL_H
#include <stdint.h>

#define WALLVERTICAL_BITS_PER_PIXEL 16
#define WALLVERTICAL_LENGTH 256
#define WALLVERTICAL_W 16
#define WALLVERTICAL_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t wallvertical[256];

#endif // WALLVERTICAL_H
