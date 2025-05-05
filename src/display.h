#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * Display functions
 *
 * Functions in this module may access the global State variable, so they operate within the context of a program in
 * execution. These functions may not modify the program State, however.
 */

#include "array.h"

#define HEADER_PANE_ELLIPSES "..."

void display_all_panes(Array *buffer);
void display_header_pane(Array *buffer);
void display_ruler_pane(Array *buffer);
void display_ruler_pane_ticks(Array *buffer);
void display_sequence_pane(Array *buffer);
void display_cursor(Array *buffer);

#endif // DISPLAY_H
