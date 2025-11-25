#ifndef GREENUP_H
#define GREENUP_H
#include <stdint.h>

#define GREENUP_BITS_PER_PIXEL 16
#define GREENUP_LENGTH 256
#define GREENUP_W 16
#define GREENUP_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t greenup[256];

#endif // GREENUP_H
