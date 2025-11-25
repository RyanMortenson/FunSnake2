#ifndef BLUERIGHT_H
#define BLUERIGHT_H
#include <stdint.h>

#define BLUERIGHT_BITS_PER_PIXEL 16
#define BLUERIGHT_LENGTH 256
#define BLUERIGHT_W 16
#define BLUERIGHT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t blueright[256];

#endif // BLUERIGHT_H
