#include "config.h"

unsigned int config_ruler_records_divider;
unsigned int config_header_sequence_divider;
int config_tick_offset;
int config_tick_spacing;
unsigned int config_nucleic_tiebreak_len;

void config_init(void)
{
    config_set_ruler_records_divider(3);
    config_set_header_sequence_divider(30);
    config_set_tick_offset(1);
    config_set_tick_spacing(10);
    config_set_nucleic_tiebreak_len(10);
}

void config_set_ruler_records_divider(unsigned int j)
{
    config_ruler_records_divider = j;
}

void config_set_header_sequence_divider(unsigned int i)
{
    config_header_sequence_divider = i;
}

void config_set_tick_offset(int tick_offset)
{
    config_tick_offset = tick_offset;
}

void config_set_tick_spacing(int tick_spacing)
{
    if (tick_spacing < 1)
        tick_spacing = 1;
    config_tick_spacing = tick_spacing;
}

void config_set_nucleic_tiebreak_len(unsigned int nucleic_tiebreak_len)
{
    config_nucleic_tiebreak_len = nucleic_tiebreak_len;
}
