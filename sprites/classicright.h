#ifndef CLASSICRIGHT_H
#define CLASSICRIGHT_H
#include <stdint.h>

#define CLASSICRIGHT_BITS_PER_PIXEL 16
#define CLASSICRIGHT_LENGTH 256
#define CLASSICRIGHT_W 16
#define CLASSICRIGHT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t classicright[256];

#endif // CLASSICRIGHT_H
