#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * Display functions
 */

#include "array.h"

void display_header_pane(Array *buffer);
void display_ruler_pane(Array *buffer);
void display_sequence_pane(Array *buffer);

#endif // DISPLAY_H
