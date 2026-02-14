#include <stdlib.h>
#include <string.h>

#include "color.h"

int color_init_color_scheme(ColorScheme *color_scheme, ColorType type, const char *name, unsigned int len)
{
    if (!color_scheme || !name || len == 0)
        return -1;

    // Get allocation sizes
    size_t fg_size = sizeof(Color);
    size_t bg_size = sizeof(Color);

    // Allocate
    char *new_name = strdup(name);
    Color *fg_map = malloc(len * fg_size);
    Color *bg_map = malloc(len * bg_size);
    bool *fg_mask = malloc(len * sizeof(bool));
    bool *bg_mask = malloc(len * sizeof(bool));
    if (!fg_map || !bg_map || !fg_mask || !bg_mask)
    {
        free(new_name);
        free(fg_map);
        free(bg_map);
        free(fg_mask);
        free(bg_mask);
        return -1;
    }

    // Common members
    color_scheme->name = new_name;
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
        for (unsigned int i = 0; i < len; i++)
        {
            fg_map[i].b4 = 0;
            bg_map[i].b4 = 0;
        }
        break;
    }
    case COLOR_8_BIT:
    {
        for (unsigned int i = 0; i < len; i++)
        {
            fg_map[i].b8 = 0;
            bg_map[i].b8 = 0;
        }
        break;
    }
    }
    color_scheme->map.fg = fg_map;
    color_scheme->map.bg = bg_map;

    return 0;
}

void color_deinit_color_scheme(ColorScheme *color_scheme)
{
    if (!color_scheme)
        return;

    free(color_scheme->name);
    free(color_scheme->mask.fg);
    free(color_scheme->mask.bg);
    free(color_scheme->map.fg);
    free(color_scheme->map.bg);
}

int color_scheme_map_fg_color(ColorScheme *color_scheme, Color color, unsigned int index)
{
    if (!color_scheme)
        return -1;
    if (index >= color_scheme->len)
        return -1;

    switch (color_scheme->type)
    {
    case COLOR_4_BIT:
        color_scheme->map.fg[index].b4 = color.b4;
        color_scheme->mask.fg[index] = true;
        break;
    case COLOR_8_BIT:
        color_scheme->map.fg[index].b8 = color.b8;
        color_scheme->mask.fg[index] = true;
        break;
    }

    return 0;
}

int color_scheme_map_bg_color(ColorScheme *color_scheme, Color color, unsigned int index)
{
    if (!color_scheme)
        return -1;
    if (index >= color_scheme->len)
        return -1;

    switch (color_scheme->type)
    {
    case COLOR_4_BIT:
        color_scheme->map.bg[index].b4 = color.b4;
        color_scheme->mask.bg[index] = true;
        break;
    case COLOR_8_BIT:
        color_scheme->map.bg[index].b8 = color.b8;
        color_scheme->mask.bg[index] = true;
        break;
    }

    return 0;
}
