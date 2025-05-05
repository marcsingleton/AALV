#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <stdbool.h>
#include <termios.h>

#include "sequences.h"

typedef struct
{
    struct termios old_termios;
    struct termios raw_termios;
    SeqRecordArray record_array;
    unsigned int terminal_rows;
    unsigned int terminal_cols;
    unsigned int header_pane_width;
    unsigned int ruler_pane_height;
    unsigned int offset_record;
    unsigned int offset_header;
    unsigned int offset_sequence;
    unsigned int cursor_record_i;   // Row index in header/sequence panes
    unsigned int cursor_header_j;   // Column index in header pane
    unsigned int cursor_sequence_j; // Column index in sequence pane
    bool refresh_ruler_pane;
    bool refresh_header_pane;
    bool refresh_sequence_pane;
    bool refresh_window;
} State;

void state_set_header_pane_width(State *state, unsigned int header_pane_width);
void state_set_offset_record(State *state, unsigned int offset_record);
void state_set_offset_header(State *state, unsigned int offset_header);
void state_set_offset_sequence(State *state, unsigned int offset_sequence);

#endif // STATE_H
