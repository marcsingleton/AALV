#include "display.h"
#include "state.h"

// FileState setters
void state_set_ruler_records_divider_i(State *state, unsigned int i)
{
    Layout *layout = &state->active_file->layout;
    if (i < layout->min_ruler_records_divider_i)
        i = layout->min_ruler_records_divider_i;
    if (i > layout->records_command_divider_i)
        i = layout->records_command_divider_i;

    layout->ruler_records_divider_i = i;

    layout->ruler_pane.h = layout->ruler_records_divider_i + 1;

    unsigned int h, d;
    d = layout->records_command_divider_i - layout->ruler_records_divider_i;
    if (d > 0)
        h = layout->records_command_divider_i - layout->ruler_records_divider_i - 1;
    else
        h = 0;

    layout->header_pane.i = layout->ruler_records_divider_i + 1;
    layout->header_pane.h = h;

    layout->sequence_pane.i = layout->ruler_records_divider_i + 1;
    layout->sequence_pane.h = h;

    layout->scroller.h = h;

    state->refresh_ruler_pane = true;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_header_sequence_divider_j(State *state, unsigned int j)
{
    Layout *layout = &state->active_file->layout;
    if (j < layout->min_header_sequence_divider_j)
        j = layout->min_header_sequence_divider_j;
    if (j > layout->max_header_sequence_divider_j)
        j = layout->max_header_sequence_divider_j;

    layout->header_sequence_divider_j = j;

    layout->header_pane.w = layout->header_sequence_divider_j + 1;
    layout->scroller.ws[SCROLLER_HEADER_PANE] = layout->header_sequence_divider_j;

    layout->sequence_pane.j = layout->header_sequence_divider_j + 1;
    layout->sequence_pane.w = state->terminal_cols - layout->header_sequence_divider_j - 1;
    layout->scroller.ws[SCROLLER_SEQUENCE_PANE] = state->terminal_cols - layout->header_sequence_divider_j - 1;

    state->refresh_ruler_pane = true;
    state->refresh_header_pane = true;
    state->refresh_sequence_pane = true;
}

void state_set_records_command_divider_i(State *state, unsigned int i)
{
    Layout *layout = &state->active_file->layout;
    if (i < layout->ruler_records_divider_i)
        i = layout->ruler_records_divider_i;
    if (i > layout->max_records_command_divider_i)
        i = layout->max_records_command_divider_i;

    layout->records_command_divider_i = i;

    unsigned int h, d;
    d = layout->records_command_divider_i - layout->ruler_records_divider_i;
    if (d > 0)
        h = layout->records_command_divider_i - layout->ruler_records_divider_i - 1;
    else
        h = 0;

    layout->header_pane.h = h;
    layout->sequence_pane.h = h;
    layout->scroller.h = h;

    layout->command_pane.i = layout->records_command_divider_i;
    layout->command_pane.h = state->terminal_rows - layout->records_command_divider_i;

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

    layout->min_ruler_records_divider_i = RULER_PANE_MIN_HEIGHT - 1;
    layout->min_header_sequence_divider_j = HEADER_PANE_MIN_WIDTH;
    layout->max_header_sequence_divider_j = state->terminal_cols - SEQUENCE_PANE_MIN_WIDTH - 1;
    layout->max_records_command_divider_i = state->terminal_rows - COMMAND_PANE_MIN_HEIGHT;
    state->visible_window = true;
}

void state_set_layout(State *state, unsigned int ruler_records_divider_i, unsigned int header_sequence_divider_j)
{
    state_set_divider_limits(state);
    state_set_records_command_divider_i(state, state->terminal_rows - 2); // Needs to be first
    state_set_ruler_records_divider_i(state, ruler_records_divider_i);
    state_set_header_sequence_divider_j(state, header_sequence_divider_j);
    state->active_file->layout.command_pane.w = state->terminal_cols;
    state->active_file->layout.ruler_pane.w = state->terminal_cols;
}

void state_set_tick_offset(State *state, unsigned int tick_offset)
{
    FileState *active_file = state->active_file;
    if (active_file->records_max_len > SIZE_MAX - tick_offset)
        return;
    if (tick_offset != active_file->tick_offset)
    {
        active_file->tick_offset = tick_offset;
        state->refresh_ruler_pane = true;
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

void state_set_seq_type_color_scheme(State *state, unsigned int seq_type_index, ColorScheme *color_scheme)
{
    if (!color_scheme)
        return;
    if (color_scheme->type == COLOR_4_BIT && state->ncolors < 16)
        return;
    if (color_scheme->type == COLOR_8_BIT && state->ncolors < 256)
        return;
    if (seq_type_index > state->n_seq_types)
        return;
    SeqTypeState *seq_type = state->seq_types + seq_type_index;
    if (seq_type->alphabet->len != color_scheme->len)
        return;
    seq_type->color_scheme = color_scheme;
}

void state_new_color_scheme(State *state, char *name, ColorType type, unsigned int len)
{
    if (!name)
        return;

    ColorScheme new_color_scheme;
    if (color_init_color_scheme(&new_color_scheme, type, name, len) != 0)
        return;

    ColorScheme *new_color_schemes = realloc(state->color_schemes,
                                             (state->n_color_schemes + 1) * sizeof(ColorScheme));
    if (!new_color_schemes)
    {
        color_deinit_color_scheme(&new_color_scheme);
        return;
    }

    new_color_schemes[state->n_color_schemes] = new_color_scheme;
    state->color_schemes = new_color_schemes;
    state->n_color_schemes++;
}
