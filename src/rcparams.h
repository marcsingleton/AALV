#ifndef RCPARAMS_H
#define RCPARAMS_H

/*
 * User configuration
 */

extern unsigned int rcparams_header_sequence_divider;
extern unsigned int rcparams_ruler_records_divider;
extern unsigned int rcparams_tick_offset;
extern unsigned int rcparams_tick_spacing;
extern unsigned int rcparams_nucleic_tiebreak_len; // Threshold for when indeterminate sequences are called nucleic

void rcparams_init(void);

void rcparams_set_ruler_records_divider(unsigned int j);
void rcparams_set_header_sequence_divider(unsigned int i);
void rcparams_set_tick_offset(unsigned int tick_offset);
void rcparams_set_tick_spacing(unsigned int tick_spacing);
void rcparams_set_nucleic_tiebreak_len(unsigned int nucleic_tiebreak_len);

#endif // RCPARAMS_H
