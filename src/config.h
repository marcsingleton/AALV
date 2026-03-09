#ifndef CONFIG_H
#define CONFIG_H

/*
 * User configuration
 */

extern unsigned int config_header_sequence_divider;
extern unsigned int config_ruler_records_divider;
extern int config_tick_offset;
extern int config_tick_spacing;
extern unsigned int config_nucleic_tiebreak_len; // Threshold for when indeterminate sequences are called nucleic

void config_init(void);

void config_set_ruler_records_divider(unsigned int j);
void config_set_header_sequence_divider(unsigned int i);
void config_set_tick_offset(int tick_offset);
void config_set_tick_spacing(int tick_spacing);
void config_set_nucleic_tiebreak_len(unsigned int nucleic_tiebreak_len);

#endif // CONFIG_H
