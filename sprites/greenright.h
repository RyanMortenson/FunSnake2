#ifndef GREENRIGHT_H
#define GREENRIGHT_H
#include <stdint.h>

#define GREENRIGHT_BITS_PER_PIXEL 16
#define GREENRIGHT_LENGTH 256
#define GREENRIGHT_W 16
#define GREENRIGHT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t greenright[256];

#endif // GREENRIGHT_H
