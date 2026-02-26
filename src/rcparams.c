#include "rcparams.h"

unsigned int rcparams_ruler_records_divider;
unsigned int rcparams_header_sequence_divider;
unsigned int rcparams_tick_offset;
unsigned int rcparams_tick_spacing;
unsigned int rcparams_nucleic_tiebreak_len;

void rcparams_init(void)
{
    rcparams_set_ruler_records_divider(3);
    rcparams_set_header_sequence_divider(30);
    rcparams_set_tick_offset(1);
    rcparams_set_tick_spacing(10);
    rcparams_set_nucleic_tiebreak_len(10);
}

void rcparams_set_ruler_records_divider(unsigned int j)
{
    rcparams_ruler_records_divider = j;
}

void rcparams_set_header_sequence_divider(unsigned int i)
{
    rcparams_header_sequence_divider = i;
}

void rcparams_set_tick_offset(unsigned int tick_offset)
{
    rcparams_tick_offset = tick_offset;
}

void rcparams_set_tick_spacing(unsigned int tick_spacing)
{
    if (tick_spacing < 1)
        tick_spacing = 1;
    rcparams_tick_spacing = tick_spacing;
}

void rcparams_set_nucleic_tiebreak_len(unsigned int nucleic_tiebreak_len)
{
    rcparams_nucleic_tiebreak_len = nucleic_tiebreak_len;
}
