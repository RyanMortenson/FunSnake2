#ifndef BLUEUP_H
#define BLUEUP_H
#include <stdint.h>

#define BLUEUP_BITS_PER_PIXEL 16
#define BLUEUP_LENGTH 256
#define BLUEUP_W 16
#define BLUEUP_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t blueup[256];

#endif // BLUEUP_H
