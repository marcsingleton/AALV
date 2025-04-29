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
    unsigned int cursor_i;
    unsigned int cursor_j;
    unsigned int offset_y;
    unsigned int offset_x;
    bool refresh_ruler_pane;
    bool refresh_header_pane;
    bool refresh_sequence_pane;
} State;

void state_set_header_pane_width(State *state, unsigned int header_pane_width);
void state_set_offset_x(State *state, unsigned int offset_x);
void state_set_offset_y(State *state, unsigned int offset_y);

#endif // STATE_H
