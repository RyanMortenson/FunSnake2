#ifndef GREENBODY_H
#define GREENBODY_H
#include <stdint.h>

#define GREENBODY_BITS_PER_PIXEL 16
#define GREENBODY_LENGTH 256
#define GREENBODY_W 16
#define GREENBODY_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t greenbody[256];

#endif // GREENBODY_H
