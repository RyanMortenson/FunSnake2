#ifndef BLUEBODY_H
#define BLUEBODY_H
#include <stdint.h>

#define BLUEBODY_BITS_PER_PIXEL 16
#define BLUEBODY_LENGTH 256
#define BLUEBODY_W 16
#define BLUEBODY_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t bluebody[256];

#endif // BLUEBODY_H
