#include <stdbool.h>
#include <wchar.h>

#include "display.h"
#include "state.h"

// FileState setters
void state_set_header_pane_width(State *state, unsigned int header_pane_width)
{
    FileState *active_file = state->active_file;
    unsigned int min_width = wcswidth(DISPLAY_HEADER_PANE_ELLIPSES, sizeof(DISPLAY_HEADER_PANE_ELLIPSES)) + 1;
    if (header_pane_width > state->terminal_cols - 2)
        header_pane_width = state->terminal_cols - 2;
    if (header_pane_width < min_width)
        header_pane_width = min_width;
    if (header_pane_width != active_file->header_pane_width)
    {
        active_file->header_pane_width = header_pane_width;
        state->refresh_ruler_pane = true;
        state->refresh_header_pane = true;
        state->refresh_sequence_pane = true;
        state->refresh_command_pane = true;
    }
}

void state_set_ruler_pane_height(State *state, unsigned int ruler_pane_height)
{
    FileState *active_file = state->active_file;
    unsigned int min_height = wcswidth(DISPLAY_RULER_PANE_ELLIPSES, sizeof(DISPLAY_RULER_PANE_ELLIPSES)) + 1;
    if (ruler_pane_height > state->terminal_rows - 2)
        ruler_pane_height = state->terminal_rows - 2;
    if (ruler_pane_height < min_height)
        ruler_pane_height = min_height;
    if (ruler_pane_height != active_file->ruler_pane_height)
    {
        active_file->ruler_pane_height = ruler_pane_height;
        state->refresh_ruler_pane = true;
        state->refresh_header_pane = true;
        state->refresh_sequence_pane = true;
    }
}

void state_set_tick_spacing(State *state, unsigned int tick_spacing)
{
    FileState *active_file = state->active_file;
    if (tick_spacing < 1)
        tick_spacing = 1;
    if (tick_spacing != active_file->tick_spacing)
    {
        active_file->tick_spacing = tick_spacing;
        state->refresh_ruler_pane = true;
    }
}

void state_set_offset_record(State *state, unsigned int offset_record)
{
    FileState *active_file = state->active_file;
    active_file->offset_record = offset_record;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_offset_sequence(State *state, unsigned int offset_sequence)
{
    FileState *active_file = state->active_file;
    active_file->offset_sequence = offset_sequence;
    state->refresh_ruler_pane = true;
    state->refresh_sequence_pane = true;
}

inline void state_set_cursor_record_i(State *state, unsigned int cursor_record_i)
{
    FileState *active_file = state->active_file;
    active_file->cursor_record_i = cursor_record_i;
}

inline void state_set_cursor_header_j(State *state, unsigned int cursor_header_j)
{
    FileState *active_file = state->active_file;
    active_file->cursor_header_j = cursor_header_j;
}

inline void state_set_cursor_sequence_j(State *state, unsigned int cursor_sequence_j)
{
    FileState *active_file = state->active_file;
    active_file->cursor_sequence_j = cursor_sequence_j;
}

// FileState getters
unsigned int state_get_record_panes_height(State *state)
{
    FileState *active_file = state->active_file;
    if (state->terminal_rows <= active_file->ruler_pane_height + 2)
        return 0;
    else
        return state->terminal_rows - active_file->ruler_pane_height - 2;
}

unsigned int state_get_sequence_pane_width(State *state)
{
    FileState *active_file = state->active_file;
    if (state->terminal_cols <= active_file->header_pane_width)
        return 0;
    else
        return state->terminal_cols - active_file->header_pane_width;
}

// State setters
void state_set_active_file_index(State *state, unsigned int file_index)
{
    if (file_index > state->nfiles - 1)
        file_index = state->nfiles - 1;
    state->active_file_index = file_index;
    state->active_file = state->files + file_index;
}

void state_set_type_color_scheme(State *state, unsigned int type_index, ColorScheme *color_scheme)
{
    if (color_scheme == NULL)
        return;
    if (color_scheme->type == COLOR_4_BIT && state->ncolors < 16)
        return;
    if (color_scheme->type == COLOR_8_BIT && state->ncolors < 256)
        return;
    if (type_index > state->ntypes)
        return;
    SeqTypeState *type = state->types + type_index;
    if (type->alphabet->len != color_scheme->len)
        return;
    type->color_scheme = color_scheme;
}
