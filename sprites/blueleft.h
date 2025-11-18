#ifndef BLUELEFT_H
#define BLUELEFT_H
#include <stdint.h>

#define BLUELEFT_BITS_PER_PIXEL 16
#define BLUELEFT_LENGTH 256
#define BLUELEFT_W 16
#define BLUELEFT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t blueleft[256];

#endif // BLUELEFT_H
