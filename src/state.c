#include <stdbool.h>

#include "state.h"

void state_set_header_pane_width(State *state, unsigned int header_pane_width)
{
    state->header_pane_width = header_pane_width;
    state->refresh_ruler_pane = true;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
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
