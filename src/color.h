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
    COLOR_4BIT,
    COLOR_8BIT,
} ColorType;

typedef struct
{
    char *name;
    ForegroundColor4Bit *fg_map;
    BackgroundColor4Bit *bg_map;
    bool *fg_mask;
    bool *bg_mask;
    unsigned int len;
} ColorScheme4Bit;

typedef struct
{
    char *name;
    uint8_t *fg_map;
    uint8_t *bg_map;
    bool *fg_mask;
    bool *bg_mask;
    unsigned int len;
} ColorScheme8Bit;

typedef union
{
    ColorType type;
    ColorScheme4Bit b4;
    ColorScheme8Bit b8;
} ColorScheme;

ColorScheme default_nucleic_4bit; // Put somewhere more specific to sequences?
ColorScheme default_protein_4bit;

void color_init_color_scheme(ColorScheme *color_scheme, unsigned int len); // Allocate/deallocate maps and masks
void color_free_color_scheme(ColorScheme *color_scheme);

#endif // COLOR_H
