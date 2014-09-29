#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>

typedef union {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
    } c;
    uint32_t packed;
} color_t;

color_t rgb(uint8_t r, uint8_t g, uint8_t b);
color_t brightness(color_t color, uint8_t n);

#endif
