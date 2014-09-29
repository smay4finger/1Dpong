#include <color.h>

color_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    color_t color;

    color.c.r = r;
    color.c.g = g;
    color.c.b = b;

    return color;
}

color_t brightness(color_t color, uint8_t brightness)
{
    color.c.r = (color.c.r * brightness) >> 8;
    color.c.g = (color.c.g * brightness) >> 8;
    color.c.b = (color.c.b * brightness) >> 8;

    return color;
}
