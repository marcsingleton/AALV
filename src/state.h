#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <termios.h>

#include "sequences.h"

typedef struct
{
    struct termios old_termios;
    struct termios raw_termios;
    SeqRecordArray record_array;
    unsigned int terminal_rows;
    unsigned int terminal_cols;
    unsigned int cursor_i;
    unsigned int cursor_j;
    unsigned int header_pane_width;
    unsigned int ruler_pane_height;
    size_t record_offset;
} State;

#endif // STATE_H
