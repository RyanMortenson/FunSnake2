#ifndef GREENLEFT_H
#define GREENLEFT_H
#include <stdint.h>

#define GREENLEFT_BITS_PER_PIXEL 16
#define GREENLEFT_LENGTH 256
#define GREENLEFT_W 16
#define GREENLEFT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t greenleft[256];

#endif // GREENLEFT_H
