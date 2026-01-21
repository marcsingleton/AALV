#include "panes.h"
#include "terminal.h"

void pane_cursor_ij(Pane *pane, Array *buffer, unsigned int i, unsigned int j)
{
    terminal_cursor_ij(buffer, pane->i + i + 1, pane->j + j + 1);
}
