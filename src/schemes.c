#include "schemes.h"

ColorScheme schemes_default_nucleic_4_bit;
ColorScheme schemes_default_protein_4_bit;
ColorScheme schemes_default_nucleic_8_bit;
ColorScheme schemes_default_protein_8_bit;
ColorScheme schemes_base[N_BASE_SCHEMES];

int schemes_init_base(void)
{
    for (unsigned int i = 0; i < N_BASE_ALPHABETS; i++)
    {
        ColorType type;
        const ColorSchemeRecord *scheme_record;
        if (i < N_BASE_SCHEMES_4_BIT)
        {
            type = COLOR_4_BIT;
            scheme_record = schemes_base_records_4_bit + i;
        }
        else
        {
            type = COLOR_8_BIT;
            scheme_record = schemes_base_records_8_bit + i - N_BASE_SCHEMES_4_BIT;
        }

        ColorScheme *scheme = scheme_record->scheme;
        Alphabet *alphabet = scheme_record->alphabet;
        int retcode = color_init_color_scheme(scheme, type, scheme_record->name, alphabet->len);
        if (retcode != 0)
            return retcode;
        for (const ColorMapRecord *map_record = scheme_record->map; map_record->sym != 0; map_record++)
        {
            int index = alphabet->index_map[(unsigned int)map_record->sym];
            if (index == -1)
                return -1;
            if (map_record->fg_mask)
                color_scheme_map_fg_color(scheme, map_record->fg_color, index);
            if (map_record->bg_mask)
                color_scheme_map_bg_color(scheme, map_record->bg_color, index);
        }
        schemes_base[i] = *scheme;
    }

    return 0;
}

void schemes_deinit_base(void)
{
    for (unsigned int i = 0; i < N_BASE_SCHEMES; i++)
        color_deinit_color_scheme(schemes_base + i);
}
