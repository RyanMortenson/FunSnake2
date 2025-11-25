#ifndef GRAYLEFT_H
#define GRAYLEFT_H
#include <stdint.h>

#define GRAYLEFT_BITS_PER_PIXEL 16
#define GRAYLEFT_LENGTH 256
#define GRAYLEFT_W 16
#define GRAYLEFT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t grayleft[256];

#endif // GRAYLEFT_H
