#ifndef CLASSICLEFT_H
#define CLASSICLEFT_H
#include <stdint.h>

#define CLASSICLEFT_BITS_PER_PIXEL 16
#define CLASSICLEFT_LENGTH 256
#define CLASSICLEFT_W 16
#define CLASSICLEFT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t classicleft[256];

#endif // CLASSICLEFT_H
