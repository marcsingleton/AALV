#include <stdbool.h>

#include "display.h"
#include "state.h"

void state_set_header_pane_width(State *state, unsigned int header_pane_width)
{
    unsigned int min_width = sizeof(HEADER_PANE_ELLIPSES) - 1 + 1; // Remove null, add separator
    if (header_pane_width > state->terminal_cols - 2)
        header_pane_width = state->terminal_cols - 2;
    if (header_pane_width < min_width)
        header_pane_width = min_width;
    if (header_pane_width != state->header_pane_width)
    {
        state->header_pane_width = header_pane_width;
        state->refresh_ruler_pane = true;
        state->refresh_header_pane = true;
        state->refresh_sequence_pane = true;
    }
}

void state_set_offset_x(State *state, unsigned int offset_x)
{
    state->offset_x = offset_x;
    state->refresh_ruler_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_offset_y(State *state, unsigned int offset_y)
{
    state->offset_y = offset_y;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}
