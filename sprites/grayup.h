#ifndef GRAYUP_H
#define GRAYUP_H
#include <stdint.h>

#define GRAYUP_BITS_PER_PIXEL 16
#define GRAYUP_LENGTH 256
#define GRAYUP_W 16
#define GRAYUP_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t grayup[256];

#endif // GRAYUP_H
