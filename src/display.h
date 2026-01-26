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

typedef void (*DisplayFunction)(Array *buffer, SeqRecord *record, size_t start, unsigned int display_len);

void display_refresh(Array *buffer);
void display_all_panes(Array *buffer);
void display_header_pane(Array *buffer);
void display_ruler_pane(Array *buffer);
void display_ruler_pane_ticks(Array *buffer);
void display_sequence_pane(Array *buffer);
void display_command_pane(Array *buffer);
void display_cursor(Array *buffer);
void display_header(Array *buffer, SeqRecord *record, size_t offset, unsigned int display_len);
void display_sequence(Array *buffer, SeqRecord *record, size_t offset, unsigned int display_len);
void display_continued_line(Array *buffer,
                            SeqRecord *record, size_t offset, size_t len,
                            DisplayFunction display_fn, unsigned int display_width);

#endif // DISPLAY_H
