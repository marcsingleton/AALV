#include "schemes.h"

ColorScheme schemes_default_nucleic_4_bit;
ColorScheme schemes_default_protein_4_bit;
ColorScheme schemes_default_nucleic_8_bit;
ColorScheme schemes_default_protein_8_bit;
ColorScheme schemes_base[N_BASE_SCHEMES];

int schemes_init_base(void)
{
    unsigned int scheme_index = 0;

    for (unsigned int i = 0; i < N_BASE_ALPHABETS; i++)
    {
        const ColorSchemeRecord *scheme_record = schemes_base_records_4_bit + i;
        ColorScheme *scheme = scheme_record->scheme;
        Alphabet *alphabet = scheme_record->alphabet;
        int retcode = color_init_color_scheme(scheme, COLOR_4_BIT, scheme_record->name, alphabet->len);
        if (retcode != 0)
            return retcode;
        for (const ColorMapRecord *map_record = scheme_record->map; map_record->sym != 0; map_record++)
        {
            int index = alphabet->index_map[(unsigned int)map_record->sym];
            if (index == -1)
                return -1;
            if (map_record->fg_mask)
            {
                scheme->map.fg[index].b4 = map_record->fg_color.b4;
                scheme->mask.fg[index] = true;
            }
            if (map_record->bg_mask)
            {
                scheme->map.bg[index].b4 = map_record->bg_color.b4;
                scheme->mask.bg[index] = true;
            }
        }
        schemes_base[scheme_index] = *scheme;
        scheme_index++;
    }

    for (unsigned int i = 0; i < N_BASE_SCHEMES_8_BIT; i++)
    {
        const ColorSchemeRecord *scheme_record = schemes_base_records_8_bit + i;
        ColorScheme *scheme = scheme_record->scheme;
        Alphabet *alphabet = scheme_record->alphabet;
        int retcode = color_init_color_scheme(scheme, COLOR_8_BIT, scheme_record->name, alphabet->len);
        if (retcode != 0)
            return retcode;
        for (const ColorMapRecord *map_record = scheme_record->map; map_record->sym != 0; map_record++)
        {
            int index = alphabet->index_map[(unsigned int)map_record->sym];
            if (index == -1)
                return -1;
            if (map_record->fg_mask)
            {
                scheme->map.fg[index].b8 = map_record->fg_color.b8;
                scheme->mask.fg[index] = true;
            }
            if (map_record->bg_mask)
            {
                scheme->map.bg[index].b8 = map_record->bg_color.b8;
                scheme->mask.bg[index] = true;
            }
        }
        schemes_base[scheme_index] = *scheme;
        scheme_index++;
    }

    return 0;
}

void schemes_deinit_base(void)
{
    for (unsigned int i = 0; i < N_BASE_SCHEMES; i++)
        color_deinit_color_scheme(schemes_base + i);
}
