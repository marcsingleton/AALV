#ifndef SCHEMES_H
#define SCHEMES_H

/*
 * Color schemes
 */

#include <stdbool.h>

#include "color.h"
#include "sequences.h"
#include "terminal.h"

typedef struct
{
    char sym;
    Color4Bit fg_color;
    Color4Bit bg_color;
    bool fg_mask;
    bool bg_mask;
} ColorMapRecord4Bit;

typedef struct
{
    char sym;
    Color8Bit fg_color;
    Color8Bit bg_color;
    bool fg_mask;
    bool bg_mask;
} ColorMapRecord8Bit;

typedef struct
{
    ColorScheme *scheme;
    const char *name;
    const ColorMapRecord4Bit *map;
    Alphabet *alphabet;
} ColorSchemeRecord4Bit;

typedef struct
{
    ColorScheme *scheme;
    const char *name;
    const ColorMapRecord8Bit *map;
    Alphabet *alphabet;
} ColorSchemeRecord8Bit;

// 4 bit
static const ColorMapRecord4Bit schemes_default_nucleic_map_records_4_bit[] = {
    {.sym = 'A', .fg_color = FG_GREEN, .fg_mask = true},
    {.sym = 'C', .fg_color = FG_BLUE, .fg_mask = true},
    {.sym = 'G', .fg_color = FG_YELLOW, .fg_mask = true},
    {.sym = 'T', .fg_color = FG_RED, .fg_mask = true},
    {.sym = 'N', .fg_color = FG_WHITE, .fg_mask = true},
    {.sym = 0},
};

extern ColorScheme schemes_default_nucleic_4_bit;

static const ColorMapRecord4Bit schemes_default_protein_map_records_4_bit[] = {
    {.sym = 'A', .fg_color = FG_BRIGHT_GREEN, .fg_mask = true},
    {.sym = 'C', .fg_color = FG_YELLOW, .fg_mask = true},
    {.sym = 'D', .fg_color = FG_RED, .fg_mask = true},
    {.sym = 'E', .fg_color = FG_RED, .fg_mask = true},
    {.sym = 'F', .fg_color = FG_MAGENTA, .fg_mask = true},
    {.sym = 'G', .fg_color = FG_WHITE, .fg_mask = true},
    {.sym = 'H', .fg_color = FG_BRIGHT_BLUE, .fg_mask = true},
    {.sym = 'I', .fg_color = FG_GREEN, .fg_mask = true},
    {.sym = 'K', .fg_color = FG_BLUE, .fg_mask = true},
    {.sym = 'L', .fg_color = FG_GREEN, .fg_mask = true},
    {.sym = 'M', .fg_color = FG_GREEN, .fg_mask = true},
    {.sym = 'N', .fg_color = FG_BRIGHT_CYAN, .fg_mask = true},
    {.sym = 'P', .fg_color = FG_BRIGHT_MAGENTA, .fg_mask = true},
    {.sym = 'Q', .fg_color = FG_BRIGHT_CYAN, .fg_mask = true},
    {.sym = 'R', .fg_color = FG_BLUE, .fg_mask = true},
    {.sym = 'S', .fg_color = FG_BRIGHT_RED, .fg_mask = true},
    {.sym = 'T', .fg_color = FG_BRIGHT_RED, .fg_mask = true},
    {.sym = 'V', .fg_color = FG_GREEN, .fg_mask = true},
    {.sym = 'W', .fg_color = FG_MAGENTA, .fg_mask = true},
    {.sym = 'Y', .fg_color = FG_MAGENTA, .fg_mask = true},
    {.sym = 'X', .fg_color = FG_BRIGHT_BLACK, .fg_mask = true},
    {.sym = 0},
};

extern ColorScheme schemes_default_protein_4_bit;

static const ColorSchemeRecord4Bit schemes_base_records_4_bit[] = {
    {
        .scheme = &schemes_default_nucleic_4_bit,
        .name = "default_nucleic_4_bit",
        .map = schemes_default_nucleic_map_records_4_bit,
        .alphabet = &NUCLEIC_ALPHABET,
    },
    {
        .scheme = &schemes_default_protein_4_bit,
        .name = "default_protein_4_bit",
        .map = schemes_default_protein_map_records_4_bit,
        .alphabet = &PROTEIN_ALPHABET,
    },
};

#define SCHEMES_N_BASE_4_BIT sizeof(schemes_base_records_4_bit) / sizeof(ColorSchemeRecord4Bit)

// 8 bit
static const ColorMapRecord8Bit schemes_default_nucleic_map_records_8_bit[] = {
    {.sym = 'A', .fg_color = 2, .fg_mask = true},
    {.sym = 'C', .fg_color = 4, .fg_mask = true},
    {.sym = 'G', .fg_color = 3, .fg_mask = true},
    {.sym = 'T', .fg_color = 1, .fg_mask = true},
    {.sym = 'N', .fg_color = 7, .fg_mask = true},
    {.sym = 0},
};

extern ColorScheme schemes_default_nucleic_8_bit;

static const ColorMapRecord8Bit schemes_default_protein_map_records_8_bit[] = {
    {.sym = 'A', .fg_color = 114, .fg_mask = true},
    {.sym = 'C', .fg_color = 226, .fg_mask = true},
    {.sym = 'D', .fg_color = 196, .fg_mask = true},
    {.sym = 'E', .fg_color = 196, .fg_mask = true},
    {.sym = 'F', .fg_color = 141, .fg_mask = true},
    {.sym = 'G', .fg_color = 252, .fg_mask = true},
    {.sym = 'H', .fg_color = 39, .fg_mask = true},
    {.sym = 'I', .fg_color = 41, .fg_mask = true},
    {.sym = 'K', .fg_color = 33, .fg_mask = true},
    {.sym = 'L', .fg_color = 41, .fg_mask = true},
    {.sym = 'M', .fg_color = 41, .fg_mask = true},
    {.sym = 'N', .fg_color = 51, .fg_mask = true},
    {.sym = 'P', .fg_color = 213, .fg_mask = true},
    {.sym = 'Q', .fg_color = 51, .fg_mask = true},
    {.sym = 'R', .fg_color = 33, .fg_mask = true},
    {.sym = 'S', .fg_color = 214, .fg_mask = true},
    {.sym = 'T', .fg_color = 214, .fg_mask = true},
    {.sym = 'V', .fg_color = 41, .fg_mask = true},
    {.sym = 'W', .fg_color = 99, .fg_mask = true},
    {.sym = 'Y', .fg_color = 141, .fg_mask = true},
    {.sym = 'X', .fg_color = 244, .fg_mask = true},
    {.sym = 0},
};

extern ColorScheme schemes_default_protein_8_bit;

static const ColorSchemeRecord8Bit schemes_base_records_8_bit[] = {
    {
        .scheme = &schemes_default_nucleic_8_bit,
        .name = "default_nucleic_8_bit",
        .map = schemes_default_nucleic_map_records_8_bit,
        .alphabet = &NUCLEIC_ALPHABET,
    },
    {
        .scheme = &schemes_default_protein_8_bit,
        .name = "default_protein_8_bit",
        .map = schemes_default_protein_map_records_8_bit,
        .alphabet = &PROTEIN_ALPHABET,
    },
};

#define SCHEMES_N_BASE_8_BIT sizeof(schemes_base_records_8_bit) / sizeof(ColorSchemeRecord8Bit)

#define SCHEMES_N_BASE (SCHEMES_N_BASE_4_BIT + SCHEMES_N_BASE_8_BIT)

extern ColorScheme schemes_base[];

int schemes_init_base(void);

#endif // SCHEMES_H
