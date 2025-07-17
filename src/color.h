#ifndef COLOR_H
#define COLOR_H

/*
 * Color representations
 */

#include <stdbool.h>
#include <stdint.h>

#include "array.h"
#include "terminal.h"

typedef enum
{
    COLOR_4_BIT,
    COLOR_8_BIT,
} ColorType;

typedef struct
{
    Color4Bit *fg;
    Color4Bit *bg;
} ColorMap4Bit;

typedef struct
{
    uint8_t *fg;
    uint8_t *bg;
} ColorMap8Bit;

typedef struct
{
    bool *fg;
    bool *bg;
} ColorMask;

typedef union
{
    ColorMap4Bit b4;
    ColorMap8Bit b8;
} ColorMap;

typedef struct
{
    const char *name;
    ColorType type;
    ColorMap map;
    ColorMask mask;
    unsigned int len;
} ColorScheme;

int color_init_color_scheme(ColorScheme *color_scheme, ColorType type, const char *name, unsigned int len);
void color_free_color_scheme(ColorScheme *color_scheme);

#endif // COLOR_H
