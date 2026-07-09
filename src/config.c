#include "config.h"

void config_init(Config *config)
{
    config_set_ruler_records_divider(config, 3);
    config_set_header_sequence_divider(config, 30);
    config_set_tick_offset(config, 1);
    config_set_tick_spacing(config, 10);
    config_set_nucleic_tiebreak_len(config, 10);
}

void config_set_ruler_records_divider(Config *config, unsigned int j)
{
    config->ruler_records_divider = j;
}

void config_set_header_sequence_divider(Config *config, unsigned int i)
{
    config->header_sequence_divider = i;
}

void config_set_tick_offset(Config *config, int tick_offset)
{
    config->tick_offset = tick_offset;
}

void config_set_tick_spacing(Config *config, int tick_spacing)
{
    if (tick_spacing < 1)
        tick_spacing = 1;
    config->tick_spacing = tick_spacing;
}

void config_set_nucleic_tiebreak_len(Config *config, unsigned int nucleic_tiebreak_len)
{
    config->nucleic_tiebreak_len = nucleic_tiebreak_len;
}
