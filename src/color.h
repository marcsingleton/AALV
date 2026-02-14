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

typedef union
{
    Color4Bit b4;
    Color8Bit b8;
} Color;

typedef struct
{
    Color *fg;
    Color *bg;
} ColorMap;

typedef struct
{
    bool *fg;
    bool *bg;
} ColorMask;

typedef struct
{
    char *name;
    ColorType type;
    ColorMap map;
    ColorMask mask;
    unsigned int len;
} ColorScheme;

int color_init_color_scheme(ColorScheme *color_scheme, ColorType type, const char *name, unsigned int len);
void color_deinit_color_scheme(ColorScheme *color_scheme);

#endif // COLOR_H
