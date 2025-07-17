#include <stdlib.h>

#include "color.h"

#include <stdio.h>

int color_init_color_scheme(ColorScheme *color_scheme, ColorType type, const char *name, unsigned int len)
{
    if (color_scheme == NULL || name == NULL || len == 0)
        return 1;

    // Get allocation sizes
    size_t fg_size, bg_size;
    switch (type)
    {
    case COLOR_4_BIT:
    {
        fg_size = sizeof(Color4Bit);
        bg_size = sizeof(Color4Bit);
        break;
    }
    case COLOR_8_BIT:
    {
        fg_size = sizeof(Color8Bit);
        bg_size = sizeof(Color8Bit);
        break;
    }
    default:
        return 1;
    }

    // Allocate
    void *fg_map = malloc(len * fg_size);
    void *bg_map = malloc(len * bg_size);
    bool *fg_mask = malloc(len * sizeof(bool));
    bool *bg_mask = malloc(len * sizeof(bool));
    if (!fg_map || !bg_map || !fg_mask || !bg_mask)
    {
        free(fg_map);
        free(bg_map);
        free(fg_mask);
        free(bg_mask);
        return 1;
    }

    // Common members
    color_scheme->name = name;
    color_scheme->type = type;
    color_scheme->len = len;
    for (unsigned int i = 0; i < len; i++)
    {
        fg_mask[i] = false;
        bg_mask[i] = false;
    }
    color_scheme->mask.fg = fg_mask;
    color_scheme->mask.bg = bg_mask;

    // Union members
    switch (type)
    {
    case COLOR_4_BIT:
    {
        Color4Bit *fg_map_b4 = fg_map;
        Color4Bit *bg_map_b4 = bg_map;
        for (unsigned int i = 0; i < len; i++)
        {
            fg_map_b4[i] = 0;
            bg_map_b4[i] = 0;
        }
        color_scheme->map.b4.fg = fg_map;
        color_scheme->map.b4.bg = bg_map;
        break;
    }
    case COLOR_8_BIT:
    {
        uint8_t *fg_map_b8 = fg_map;
        uint8_t *bg_map_b8 = bg_map;
        for (unsigned int i = 0; i < len; i++)
        {
            fg_map_b8[i] = 0;
            bg_map_b8[i] = 0;
        }
        color_scheme->map.b8.fg = fg_map;
        color_scheme->map.b8.bg = bg_map;
        break;
    }
    }

    return 0;
}

void color_free_color_scheme(ColorScheme *color_scheme)
{
    if (color_scheme == NULL)
        return;

    free(color_scheme->mask.fg);
    free(color_scheme->mask.bg);
    if (color_scheme->type == COLOR_4_BIT)
    {
        free(color_scheme->map.b4.fg);
        free(color_scheme->map.b4.bg);
    }
    else if (color_scheme->type == COLOR_8_BIT)
    {
        free(color_scheme->map.b8.fg);
        free(color_scheme->map.b8.bg);
    }
}
