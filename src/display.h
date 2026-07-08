#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * Display functions
 *
 * Functions in this module may access the global State variable, so they operate within the context of a program in
 * execution. These functions may not modify the program State, however.
 */

#include "array.h"
#include "sequences.h"
#include "state.h"

typedef void (*DisplayFunction)(State *state, Array *buffer, SeqRecord *record, size_t start, unsigned int display_len);

void display_refresh(State *state, Array *buffer);
void display_all_panes(State *state, Array *buffer);
void display_header_pane(State *state, Array *buffer);
void display_ruler_pane(State *state, Array *buffer);
void display_ruler_pane_ticks(State *state, Array *buffer);
void display_sequence_pane(State *state, Array *buffer);
void display_command_pane(State *state, Array *buffer);
void display_cursor(State *state, Array *buffer);
void display_splash_screen(State *state, Array *buffer);
void display_header(State *state, Array *buffer, SeqRecord *record, size_t offset, unsigned int display_len);
void display_sequence(State *state, Array *buffer, SeqRecord *record, size_t offset, unsigned int display_len);
void display_continued_line(State *state, Array *buffer,
                            SeqRecord *record, size_t offset, size_t len,
                            DisplayFunction display_fn, unsigned int display_width);

#endif // DISPLAY_H
