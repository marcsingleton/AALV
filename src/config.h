#ifndef CONFIG_H
#define CONFIG_H

/*
 * User configuration
 */

typedef struct
{
    unsigned int header_sequence_divider;
    unsigned int ruler_records_divider;
    int tick_offset;
    int tick_spacing;
    unsigned int nucleic_tiebreak_len; // Threshold for when indeterminate sequences are called nucleic
} Config;

void config_init(Config *config);

void config_set_ruler_records_divider(Config *config, unsigned int j);
void config_set_header_sequence_divider(Config *config, unsigned int i);
void config_set_tick_offset(Config *config, int tick_offset);
void config_set_tick_spacing(Config *config, int tick_spacing);
void config_set_nucleic_tiebreak_len(Config *config, unsigned int nucleic_tiebreak_len);

#endif // CONFIG_H
