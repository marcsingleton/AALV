#include "display.h"
#include "state.h"

// FileState setters
void state_set_ruler_records_divider(State *state, unsigned int i)
{
    Layout *layout = &state->active_file->layout;
    if (i < layout->min_ruler_records_divider)
        i = layout->min_ruler_records_divider;
    if (i > layout->records_command_divider)
        i = layout->records_command_divider;

    layout->ruler_records_divider = i;

    layout->ruler_pane.h = layout->ruler_records_divider + 1;

    unsigned int h, d;
    d = layout->records_command_divider - layout->ruler_records_divider;
    if (d > 0)
        h = layout->records_command_divider - layout->ruler_records_divider - 1;
    else
        h = 0;

    layout->header_pane.i = layout->ruler_records_divider + 1;
    layout->header_pane.h = h;

    layout->sequence_pane.i = layout->ruler_records_divider + 1;
    layout->sequence_pane.h = h;

    layout->scroller.h = h;

    state->refresh_ruler_pane = true;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_header_sequence_divider(State *state, unsigned int j)
{
    Layout *layout = &state->active_file->layout;
    if (j < layout->min_header_sequence_divider)
        j = layout->min_header_sequence_divider;
    if (j > layout->max_header_sequence_divider)
        j = layout->max_header_sequence_divider;

    layout->header_sequence_divider = j;

    layout->header_pane.w = layout->header_sequence_divider + 1;
    layout->scroller.ws[SCROLLER_HEADER_PANE] = layout->header_sequence_divider;

    layout->sequence_pane.j = layout->header_sequence_divider + 1;
    layout->sequence_pane.w = state->terminal_cols - layout->header_sequence_divider - 1;
    layout->scroller.ws[SCROLLER_SEQUENCE_PANE] = state->terminal_cols - layout->header_sequence_divider - 1;

    state->refresh_ruler_pane = true;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_records_command_divider(State *state, unsigned int i)
{
    Layout *layout = &state->active_file->layout;
    if (i < layout->ruler_records_divider)
        i = layout->ruler_records_divider;
    if (i > layout->max_records_command_divider)
        i = layout->max_records_command_divider;

    layout->records_command_divider = i;

    unsigned int h, d;
    d = layout->records_command_divider - layout->ruler_records_divider;
    if (d > 0)
        h = layout->records_command_divider - layout->ruler_records_divider - 1;
    else
        h = 0;

    layout->header_pane.h = h;
    layout->sequence_pane.h = h;
    layout->scroller.h = h;

    layout->command_pane.i = layout->records_command_divider;
    layout->command_pane.h = state->terminal_rows - layout->records_command_divider;

    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
    state->refresh_command_pane = true;
}

void state_set_divider_limits(State *state)
{
    Layout *layout = &state->active_file->layout;
    unsigned int min_height = RULER_PANE_MIN_HEIGHT + RECORDS_PANE_MIN_HEIGHT + COMMAND_PANE_MIN_HEIGHT;
    unsigned int min_width = HEADER_PANE_MIN_WIDTH + SEQUENCE_PANE_MIN_WIDTH;
    if (state->terminal_rows < min_height || state->terminal_cols < min_width)
    {
        state->visible_window = false;
        return;
    }

    layout->min_ruler_records_divider = RULER_PANE_MIN_HEIGHT - 1;
    layout->min_header_sequence_divider = HEADER_PANE_MIN_WIDTH;
    layout->max_header_sequence_divider = state->terminal_cols - SEQUENCE_PANE_MIN_WIDTH - 1;
    layout->max_records_command_divider = state->terminal_rows - COMMAND_PANE_MIN_HEIGHT;
    state->visible_window = true;
}

void state_set_layout(State *state, unsigned int ruler_records_divider, unsigned int header_sequence_divider)
{
    state_set_divider_limits(state);
    state_set_records_command_divider(state, state->terminal_rows - 2); // Needs to be first
    state_set_ruler_records_divider(state, ruler_records_divider);
    state_set_header_sequence_divider(state, header_sequence_divider);
    state->active_file->layout.command_pane.w = state->terminal_cols;
    state->active_file->layout.ruler_pane.w = state->terminal_cols;
}

void state_set_tick_offset(State *state, int tick_offset)
{
    FileState *active_file = state->active_file;
    if (tick_offset > 0 && active_file->records_max_len > INT_MAX - (unsigned int)tick_offset)
        return;
    if (tick_offset != active_file->tick_offset)
    {
        active_file->tick_offset = tick_offset;
        state->refresh_ruler_pane = true;
    }
}

void state_set_tick_spacing(State *state, int tick_spacing)
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

// State setters
void state_set_terminal_size(State *state)
{
    unsigned int rows, cols;
    terminal_get_window_size(&rows, &cols);
    state->terminal_rows = rows;
    state->terminal_cols = cols;
}

void state_set_active_file_index(State *state, unsigned int file_index)
{
    if (file_index > state->nfiles - 1)
        file_index = state->nfiles - 1;
    state->active_file_index = file_index;
    state->active_file = state->files + file_index;
}

void state_set_active_color_scheme(State *state, SeqColorScheme *color_scheme)
{
    if (!color_scheme)
        return;
    if (color_scheme->scheme.type == COLOR_4_BIT && state->ncolors < 16)
        return;
    if (color_scheme->scheme.type == COLOR_8_BIT && state->ncolors < 256)
        return;
    if (color_scheme->type > state->n_active_color_schemes)
        return;
    state->active_color_schemes[color_scheme->type] = color_scheme;
    state->refresh_sequence_pane = true;
}

void state_new_color_scheme(State *state, char *name, SeqType seq_type, ColorType color_type)
{
    if (!name)
        return;

    SeqColorScheme new_color_scheme;
    Alphabet *alphabet = sequences_seq_type_to_alphabet(seq_type);
    if (!alphabet)
        return;
    if (color_init_color_scheme(&new_color_scheme.scheme, color_type, name, alphabet->len) != 0)
        return;
    new_color_scheme.type = seq_type;

    SeqColorScheme *new_color_schemes = realloc(state->color_schemes,
                                                (state->n_color_schemes + 1) * sizeof(SeqColorScheme));
    if (!new_color_schemes)
    {
        color_deinit_color_scheme(&new_color_scheme.scheme);
        return;
    }

    new_color_schemes[state->n_color_schemes] = new_color_scheme;
    state->color_schemes = new_color_schemes;
    state->n_color_schemes++;
}
