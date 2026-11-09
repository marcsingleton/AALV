#include "config.h"

int config_init(Config *config)
{
    if (config_set_ruler_records_divider(config, 3) != 0 ||
        config_set_header_sequence_divider(config, 30) != 0 ||
        config_set_tick_offset(config, 1) != 0 ||
        config_set_tick_spacing(config, 10) != 0 ||
        config_set_nucleic_tiebreak_len(config, 10) != 0)
        return -1;

    return 0;
}

int config_set_ruler_records_divider(Config *config, unsigned int j)
{
    config->ruler_records_divider = j;

    return 0;
}

int config_set_header_sequence_divider(Config *config, unsigned int i)
{
    config->header_sequence_divider = i;

    return 0;
}

int config_set_tick_offset(Config *config, int tick_offset)
{
    config->tick_offset = tick_offset;

    return 0;
}

int config_set_tick_spacing(Config *config, int tick_spacing)
{
    if (tick_spacing < 1)
        tick_spacing = 1;
    config->tick_spacing = tick_spacing;

    return 0;
}

int config_set_nucleic_tiebreak_len(Config *config, unsigned int nucleic_tiebreak_len)
{
    config->nucleic_tiebreak_len = nucleic_tiebreak_len;

    return 0;
}
