#include <stdbool.h>
#include <wchar.h>

#include "display.h"
#include "state.h"

void state_set_header_pane_width(State *state, unsigned int header_pane_width)
{
    unsigned int min_width = wcswidth(HEADER_PANE_ELLIPSES, sizeof(HEADER_PANE_ELLIPSES)) + 1;
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
        state->refresh_command_pane = true;
    }
}

void state_set_ruler_pane_height(State *state, unsigned int ruler_pane_height)
{

    unsigned int min_height = wcswidth(RULER_PANE_ELLIPSES, sizeof(RULER_PANE_ELLIPSES)) + 1;
    if (ruler_pane_height > state->terminal_rows - 2)
        ruler_pane_height = state->terminal_rows - 2;
    if (ruler_pane_height < min_height)
        ruler_pane_height = min_height;
    if (ruler_pane_height != state->ruler_pane_height)
    {
        state->ruler_pane_height = ruler_pane_height;
        state->refresh_ruler_pane = true;
        state->refresh_header_pane = true;
        state->refresh_sequence_pane = true;
    }
}

void state_set_tick_spacing(State *state, unsigned int tick_spacing)
{
    if (tick_spacing < 1)
        tick_spacing = 1;
    if (tick_spacing != state->tick_spacing)
    {
        state->tick_spacing = tick_spacing;
        state->refresh_ruler_pane = true;
    }
}

void state_set_offset_record(State *state, unsigned int offset_record)
{
    state->offset_record = offset_record;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_offset_sequence(State *state, unsigned int offset_sequence)
{
    state->offset_sequence = offset_sequence;
    state->refresh_ruler_pane = true;
    state->refresh_sequence_pane = true;
}

unsigned int state_get_record_panes_height(State *state)
{
    if (state->terminal_rows <= state->ruler_pane_height + 2)
        return 0;
    else
        return state->terminal_rows - state->ruler_pane_height - 2;
}

unsigned int state_get_sequence_pane_width(State *state)
{
    if (state->terminal_cols <= state->header_pane_width)
        return 0;
    else
        return state->terminal_cols - state->header_pane_width;
}
