#ifndef GRAYRIGHT_H
#define GRAYRIGHT_H
#include <stdint.h>

#define GRAYRIGHT_BITS_PER_PIXEL 16
#define GRAYRIGHT_LENGTH 256
#define GRAYRIGHT_W 16
#define GRAYRIGHT_H 16

// Stored as RGB565 (uint16_t). Cast to (const color_t*) if your color_t matches.
extern const uint16_t grayright[256];

#endif // GRAYRIGHT_H
