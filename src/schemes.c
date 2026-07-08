#include "schemes.h"

typedef struct
{
    char sym;
    Color fg_color;
    Color bg_color;
    bool fg_mask;
    bool bg_mask;
} ColorMapRecord;

typedef struct
{
    SeqColorScheme *scheme;
    const char *name;
    const ColorMapRecord *map;
} ColorSchemeRecord;

// 4 bit
static const ColorMapRecord schemes_default_nucleic_4_bit_map_records[] = {
    {.sym = 'A', .fg_color.u4 = FG_GREEN, .fg_mask = true},
    {.sym = 'C', .fg_color.u4 = FG_BLUE, .fg_mask = true},
    {.sym = 'G', .fg_color.u4 = FG_YELLOW, .fg_mask = true},
    {.sym = 'T', .fg_color.u4 = FG_RED, .fg_mask = true},
    {.sym = 'U', .fg_color.u4 = FG_BRIGHT_RED, .fg_mask = true},
    {.sym = 'N', .fg_color.u4 = FG_WHITE, .fg_mask = true},
    {.sym = 0},
};

static const ColorMapRecord schemes_default_protein_4_bit_map_records[] = {
    {.sym = 'A', .fg_color.u4 = FG_BRIGHT_GREEN, .fg_mask = true},
    {.sym = 'C', .fg_color.u4 = FG_YELLOW, .fg_mask = true},
    {.sym = 'D', .fg_color.u4 = FG_RED, .fg_mask = true},
    {.sym = 'E', .fg_color.u4 = FG_RED, .fg_mask = true},
    {.sym = 'F', .fg_color.u4 = FG_MAGENTA, .fg_mask = true},
    {.sym = 'G', .fg_color.u4 = FG_WHITE, .fg_mask = true},
    {.sym = 'H', .fg_color.u4 = FG_BRIGHT_BLUE, .fg_mask = true},
    {.sym = 'I', .fg_color.u4 = FG_GREEN, .fg_mask = true},
    {.sym = 'K', .fg_color.u4 = FG_BLUE, .fg_mask = true},
    {.sym = 'L', .fg_color.u4 = FG_GREEN, .fg_mask = true},
    {.sym = 'M', .fg_color.u4 = FG_GREEN, .fg_mask = true},
    {.sym = 'N', .fg_color.u4 = FG_BRIGHT_CYAN, .fg_mask = true},
    {.sym = 'P', .fg_color.u4 = FG_BRIGHT_MAGENTA, .fg_mask = true},
    {.sym = 'Q', .fg_color.u4 = FG_BRIGHT_CYAN, .fg_mask = true},
    {.sym = 'R', .fg_color.u4 = FG_BLUE, .fg_mask = true},
    {.sym = 'S', .fg_color.u4 = FG_BRIGHT_RED, .fg_mask = true},
    {.sym = 'T', .fg_color.u4 = FG_BRIGHT_RED, .fg_mask = true},
    {.sym = 'V', .fg_color.u4 = FG_GREEN, .fg_mask = true},
    {.sym = 'W', .fg_color.u4 = FG_MAGENTA, .fg_mask = true},
    {.sym = 'Y', .fg_color.u4 = FG_MAGENTA, .fg_mask = true},
    {.sym = 'X', .fg_color.u4 = FG_BRIGHT_BLACK, .fg_mask = true},
    {.sym = 0},
};

static const ColorSchemeRecord schemes_base_4_bit_records[] = {
    {
        .scheme = &schemes_default_nucleic_4_bit,
        .name = "default_nucleic_4_bit",
        .map = schemes_default_nucleic_4_bit_map_records,
    },
    {
        .scheme = &schemes_default_protein_4_bit,
        .name = "default_protein_4_bit",
        .map = schemes_default_protein_4_bit_map_records,
    },
};
#define N_BASE_SCHEMES_4_BIT sizeof(schemes_base_4_bit_records) / sizeof(ColorSchemeRecord)

// 8 bit
static const ColorMapRecord schemes_default_nucleic_8_bit_map_records[] = {
    {.sym = 'A', .fg_color.u8 = 2, .fg_mask = true},
    {.sym = 'C', .fg_color.u8 = 4, .fg_mask = true},
    {.sym = 'G', .fg_color.u8 = 3, .fg_mask = true},
    {.sym = 'T', .fg_color.u8 = 1, .fg_mask = true},
    {.sym = 'U', .fg_color.u8 = 9, .fg_mask = true},
    {.sym = 'N', .fg_color.u8 = 7, .fg_mask = true},
    {.sym = 0},
};

static const ColorMapRecord schemes_default_protein_8_bit_map_records[] = {
    {.sym = 'A', .fg_color.u8 = 114, .fg_mask = true},
    {.sym = 'C', .fg_color.u8 = 226, .fg_mask = true},
    {.sym = 'D', .fg_color.u8 = 196, .fg_mask = true},
    {.sym = 'E', .fg_color.u8 = 196, .fg_mask = true},
    {.sym = 'F', .fg_color.u8 = 141, .fg_mask = true},
    {.sym = 'G', .fg_color.u8 = 252, .fg_mask = true},
    {.sym = 'H', .fg_color.u8 = 39, .fg_mask = true},
    {.sym = 'I', .fg_color.u8 = 41, .fg_mask = true},
    {.sym = 'K', .fg_color.u8 = 33, .fg_mask = true},
    {.sym = 'L', .fg_color.u8 = 41, .fg_mask = true},
    {.sym = 'M', .fg_color.u8 = 41, .fg_mask = true},
    {.sym = 'N', .fg_color.u8 = 51, .fg_mask = true},
    {.sym = 'P', .fg_color.u8 = 213, .fg_mask = true},
    {.sym = 'Q', .fg_color.u8 = 51, .fg_mask = true},
    {.sym = 'R', .fg_color.u8 = 33, .fg_mask = true},
    {.sym = 'S', .fg_color.u8 = 214, .fg_mask = true},
    {.sym = 'T', .fg_color.u8 = 214, .fg_mask = true},
    {.sym = 'V', .fg_color.u8 = 41, .fg_mask = true},
    {.sym = 'W', .fg_color.u8 = 99, .fg_mask = true},
    {.sym = 'Y', .fg_color.u8 = 141, .fg_mask = true},
    {.sym = 'X', .fg_color.u8 = 244, .fg_mask = true},
    {.sym = 0},
};

static const ColorSchemeRecord schemes_base_8_bit_records[] = {
    {
        .scheme = &schemes_default_nucleic_8_bit,
        .name = "default_nucleic_8_bit",
        .map = schemes_default_nucleic_8_bit_map_records,
    },
    {
        .scheme = &schemes_default_protein_8_bit,
        .name = "default_protein_8_bit",
        .map = schemes_default_protein_8_bit_map_records,
    },

};
#define N_BASE_SCHEMES_8_BIT sizeof(schemes_base_8_bit_records) / sizeof(ColorSchemeRecord)

#define N_BASE_SCHEMES (N_BASE_SCHEMES_4_BIT + N_BASE_SCHEMES_8_BIT)

// Public
SeqColorScheme schemes_default_nucleic_4_bit = {.type = SEQ_TYPE_NUCLEIC};
SeqColorScheme schemes_default_protein_4_bit = {.type = SEQ_TYPE_PROTEIN};
SeqColorScheme schemes_default_nucleic_8_bit = {.type = SEQ_TYPE_NUCLEIC};
SeqColorScheme schemes_default_protein_8_bit = {.type = SEQ_TYPE_PROTEIN};

SeqColorScheme schemes_base[N_BASE_SCHEMES];
unsigned int n_base_schemes = N_BASE_SCHEMES;

int schemes_init_base(void)
{
    for (unsigned int i = 0; i < N_BASE_SCHEMES; i++)
    {
        ColorType type;
        const ColorSchemeRecord *scheme_record;
        if (i < N_BASE_SCHEMES_4_BIT)
        {
            type = COLOR_4_BIT;
            scheme_record = schemes_base_4_bit_records + i;
        }
        else
        {
            type = COLOR_8_BIT;
            scheme_record = schemes_base_8_bit_records + i - N_BASE_SCHEMES_4_BIT;
        }

        SeqColorScheme *scheme = scheme_record->scheme;
        Alphabet *alphabet = sequences_seq_type_to_alphabet(scheme->type);
        int retcode = color_init_color_scheme(&scheme->scheme, type, scheme_record->name, alphabet->len);
        if (retcode != 0)
            return retcode;
        for (const ColorMapRecord *map_record = scheme_record->map; map_record->sym != 0; map_record++)
        {
            int index = alphabet->index_map[(unsigned int)map_record->sym];
            if (index == -1)
                return -1;
            if (map_record->fg_mask)
                color_scheme_map_fg_color(&scheme->scheme, map_record->fg_color, index);
            if (map_record->bg_mask)
                color_scheme_map_bg_color(&scheme->scheme, map_record->bg_color, index);
        }
        schemes_base[i] = *scheme;
    }

    return 0;
}

void schemes_deinit_base(void)
{
    for (unsigned int i = 0; i < N_BASE_SCHEMES; i++)
        color_deinit_color_scheme(&schemes_base[i].scheme);
}
