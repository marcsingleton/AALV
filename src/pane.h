#ifndef PANE_H
#define PANE_H

/*
 * Panes
 */

#include "array.h"

typedef struct
{
    unsigned int i; // origin row
    unsigned int j; // origin col
    unsigned int w; // width
    unsigned int h; // height
} Pane;

void pane_cursor_ij(Pane *pane, Array *buffer, unsigned int i, unsigned int j);

#endif // PANE_H
