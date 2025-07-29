#include "schemes.h"

ColorScheme schemes_default_nucleic_4_bit;
ColorScheme schemes_default_protein_4_bit;
ColorScheme schemes_default_nucleic_8_bit;
ColorScheme schemes_default_protein_8_bit;
ColorScheme schemes_base[SCHEMES_N_BASE];

int schemes_init_base(void)
{
    unsigned int index = 0;

    for (unsigned int i = 0; i < SCHEMES_N_BASE_4_BIT; i++)
    {
        const ColorSchemeRecord4Bit *scheme_record = schemes_base_records_4_bit + i;
        ColorScheme *scheme = scheme_record->scheme;
        Alphabet *alphabet = scheme_record->alphabet;
        int code = color_init_color_scheme(scheme, COLOR_4_BIT, scheme_record->name, alphabet->len);
        if (code != 0)
            return code;
        for (const ColorMapRecord4Bit *map_record = scheme_record->map; map_record->sym != 0; map_record++)
        {
            int index = alphabet->index_map[(unsigned int)map_record->sym];
            if (index == -1)
                return 1;
            if (map_record->fg_mask)
            {
                scheme->map.b4.fg[index] = map_record->fg_color;
                scheme->mask.fg[index] = true;
            }
            if (map_record->bg_mask)
            {
                scheme->map.b4.bg[index] = map_record->bg_color;
                scheme->mask.bg[index] = true;
            }
        }
        schemes_base[index] = *scheme;
        index++;
    }

    for (unsigned int i = 0; i < SCHEMES_N_BASE_8_BIT; i++)
    {
        const ColorSchemeRecord8Bit *scheme_record = schemes_base_records_8_bit + i;
        ColorScheme *scheme = scheme_record->scheme;
        Alphabet *alphabet = scheme_record->alphabet;
        int code = color_init_color_scheme(scheme, COLOR_8_BIT, scheme_record->name, alphabet->len);
        if (code != 0)
            return code;
        for (const ColorMapRecord8Bit *map_record = scheme_record->map; map_record->sym != 0; map_record++)
        {
            int index = alphabet->index_map[(unsigned int)map_record->sym];
            if (index == -1)
                return 1;
            if (map_record->fg_mask)
            {
                scheme->map.b8.fg[index] = map_record->fg_color;
                scheme->mask.fg[index] = true;
            }
            if (map_record->bg_mask)
            {
                scheme->map.b8.bg[index] = map_record->bg_color;
                scheme->mask.bg[index] = true;
            }
        }
        schemes_base[index] = *scheme;
        index++;
    }

    return 0;
}
