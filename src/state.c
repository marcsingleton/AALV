#include <string.h>

#include "config.h"
#include "state.h"

SeqColorScheme *active_color_schemes[SEQ_TYPE_ERROR + 1];
unsigned int n_active_color_schemes = sizeof(active_color_schemes) / sizeof(SeqColorScheme *);

// FileState
int state_init_file_data(FileState *file)
{
    if (!file)
        return -1;

    file->file_path = NULL;
    file->record_array.data = NULL;
    file->record_array.len = 0;
    file->metadata.indices_array.data = NULL;
    file->metadata.indices_array.len = 0;
    file->metadata.max_len = 0;

    return 0;
}

void state_deinit_file_data(FileState *file)
{
    if (!file)
        return;

    free(file->file_path);
    sequences_deinit_seq_record_array(&file->record_array);
    sequences_deinit_unaligned_indices_array(&file->metadata.indices_array);
    file->metadata.max_len = 0;
}

int state_set_file_path(FileState *file, char *file_path)
{
    if (!file || !file_path)
        return -1;

    char *new_file_path = strdup(file_path);
    if (!new_file_path)
        return -1;
    free(file->file_path);
    file->file_path = new_file_path;

    return 0;
}

// FileState and State
int state_set_ruler_records_divider(State *state, unsigned int i)
{
    if (!state || !state->active_file)
        return -1;

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

    return 0;
}

int state_set_header_sequence_divider(State *state, unsigned int j)
{
    if (!state || !state->active_file)
        return -1;

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

    return 0;
}

int state_set_records_command_divider(State *state, unsigned int i)
{
    if (!state || !state->active_file)
        return -1;

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

    return 0;
}

int state_set_divider_limits(State *state)
{
    if (!state || !state->active_file)
        return -1;

    Layout *layout = &state->active_file->layout;
    unsigned int min_height = RULER_PANE_MIN_HEIGHT + RECORDS_PANE_MIN_HEIGHT + COMMAND_PANE_MIN_HEIGHT;
    unsigned int min_width = HEADER_PANE_MIN_WIDTH + SEQUENCE_PANE_MIN_WIDTH;
    if (state->terminal_rows < min_height || state->terminal_cols < min_width)
    {
        state->visible_window = false;
        return -1;
    }

    layout->min_ruler_records_divider = RULER_PANE_MIN_HEIGHT - 1;
    layout->min_header_sequence_divider = HEADER_PANE_MIN_WIDTH;
    layout->max_header_sequence_divider = state->terminal_cols - SEQUENCE_PANE_MIN_WIDTH - 1;
    layout->max_records_command_divider = state->terminal_rows - COMMAND_PANE_MIN_HEIGHT;
    state->visible_window = true;

    return 0;
}

int state_set_layout(State *state, unsigned int ruler_records_divider, unsigned int header_sequence_divider)
{
    if (!state || !state->active_file)
        return -1;

    if (state_set_divider_limits(state) != 0)
        return -1;
    if (state_set_records_command_divider(state, state->terminal_rows - 2) != 0) // Needs to be first
        return -1;
    if (state_set_ruler_records_divider(state, ruler_records_divider) != 0)
        return -1;
    if (state_set_header_sequence_divider(state, header_sequence_divider) != 0)
        return -1;
    state->active_file->layout.command_pane.w = state->terminal_cols;
    state->active_file->layout.ruler_pane.w = state->terminal_cols;

    return 0;
}

int state_set_tick_offset(State *state, int tick_offset)
{
    if (!state || !state->active_file)
        return -1;

    FileState *active_file = state->active_file;
    if (tick_offset > 0 && active_file->metadata.max_len > INT_MAX - (unsigned int)tick_offset)
        return -1;
    if (tick_offset != active_file->layout.tick_offset)
    {
        active_file->layout.tick_offset = tick_offset;
        state->refresh_ruler_pane = true;
    }

    return 0;
}

int state_set_tick_spacing(State *state, int tick_spacing)
{
    if (!state || !state->active_file)
        return -1;

    FileState *active_file = state->active_file;
    if (tick_spacing < 1)
        tick_spacing = 1;
    if (tick_spacing != active_file->layout.tick_spacing)
    {
        active_file->layout.tick_spacing = tick_spacing;
        state->refresh_ruler_pane = true;
    }

    return 0;
}

// State
int state_init(State *state)
{
    if (!state)
        return -1;

    memset(state, 0, sizeof(State));
    state_set_terminal_size(state);
    config_init(&state->config);

    return 0;
}

void state_deinit(State *state)
{
    if (!state)
        return;

    if (state->files)
    {
        for (unsigned int i = 0; i < state->nfiles; i++)
        {
            FileState *file = state->files + i;
            scroller_deinit(&file->layout.scroller);
            state_deinit_file_data(file);
        }
        free(state->files);
    }
    if (state->color_schemes)
    {
        for (unsigned int i = 0; i < state->n_color_schemes; i++)
            color_deinit_color_scheme(&state->color_schemes[i].scheme);
        free(state->color_schemes);
    }
}

int state_set_terminal_size(State *state)
{
    if (!state)
        return -1;

    unsigned int rows, cols;
    terminal_get_window_size(&rows, &cols);
    state->terminal_rows = rows;
    state->terminal_cols = cols;

    return 0;
}

int state_set_active_file_index(State *state, unsigned int file_index)
{
    if (!state)
        return -1;
    if (!state->files || state->nfiles == 0)
    {
        state->active_file = NULL;
        state->active_file_index = 0;
        return 0;
    }

    if (file_index > state->nfiles - 1)
        file_index = state->nfiles - 1;
    state->active_file = state->files + file_index;
    state->active_file_index = file_index;

    return 0;
}

int state_new_file(State *state)
{
    if (!state)
        return -1;

    FileState *files;
    if (state->nfiles == 0)
        files = malloc(sizeof(FileState));
    else
        files = realloc(state->files, (state->nfiles + 1) * sizeof(FileState));
    if (!files)
        return -1;

    FileState *file = files + state->nfiles;
    scroller_init(&file->layout.scroller, SCROLLER_NPANES);
    scroller_set_active_pane(&file->layout.scroller, SCROLLER_SEQUENCE_PANE);
    state_init_file_data(file);

    state->files = files;
    state->nfiles += 1;

    state_set_active_file_index(state, state->nfiles - 1);
    state_set_layout(state, state->config.ruler_records_divider, state->config.header_sequence_divider);
    state_set_tick_offset(state, state->config.tick_offset);
    state_set_tick_spacing(state, state->config.tick_spacing);

    state->refresh_window = true;

    return 0;
}

int state_remove_file(State *state)
{
    if (!state)
        return -1;
    if (!state->active_file || !state->files)
        return -1;

    FileState *file = state->active_file;
    scroller_deinit(&file->layout.scroller);
    state_deinit_file_data(file);

    for (unsigned int i = state->active_file_index + 1; i < state->nfiles; i++)
        state->files[i - 1] = state->files[i];

    if (state->nfiles > 1)
    {
        FileState *files = realloc(state->files, (state->nfiles - 1) * sizeof(FileState));
        if (!files)
            return -1;
        state->files = files;
        state->nfiles -= 1;
        state_set_active_file_index(state, state->nfiles);
    }
    else
    {
        free(state->files);
        state->files = NULL;
        state->nfiles = 0;
        state_set_active_file_index(state, 0);
    }

    state->refresh_window = true;

    return 0;
}

int state_set_active_color_scheme(State *state, SeqColorScheme *color_scheme)
{
    if (!state || !color_scheme)
        return -1;
    if (color_scheme->scheme.type == COLOR_4_BIT && state->ncolors < 16)
        return -1;
    if (color_scheme->scheme.type == COLOR_8_BIT && state->ncolors < 256)
        return -1;
    if (color_scheme->type > state->n_active_color_schemes)
        return -1;

    state->active_color_schemes[color_scheme->type] = color_scheme;
    state->refresh_sequence_pane = true;

    return 0;
}

int state_new_color_scheme(State *state, char *name, SeqType seq_type, ColorType color_type)
{
    if (!state || !name)
        return -1;

    for (unsigned int i = 0; i < state->n_color_schemes; i++)
    {
        SeqColorScheme *color_scheme = state->color_schemes + i;
        if (strcmp(name, color_scheme->scheme.name) == 0)
            return -1;
    }

    SeqColorScheme new_color_scheme;
    Alphabet *alphabet = sequences_seq_type_to_alphabet(seq_type);
    if (!alphabet)
        return -1;
    if (color_init_color_scheme(&new_color_scheme.scheme, color_type, name, alphabet->len) != 0)
        return -1;
    new_color_scheme.type = seq_type;

    SeqColorScheme *new_color_schemes = realloc(state->color_schemes,
                                                (state->n_color_schemes + 1) * sizeof(SeqColorScheme));
    if (!new_color_schemes)
    {
        color_deinit_color_scheme(&new_color_scheme.scheme);
        return -1;
    }

    new_color_schemes[state->n_color_schemes] = new_color_scheme;
    state->color_schemes = new_color_schemes;
    state->n_color_schemes += 1;

    return 0;
}

int state_remove_color_scheme(State *state, char *name)
{
    if (!state || !name)
        return -1;

    for (unsigned int i = 0; i < state->n_active_color_schemes; i++)
    {
        SeqColorScheme *color_scheme = state->active_color_schemes[i];
        if (color_scheme && strcmp(name, color_scheme->scheme.name) == 0)
            state->active_color_schemes[i] = NULL;
    }

    int has_scheme = 0;
    for (unsigned int i = 0; i < state->n_color_schemes; i++)
    {
        SeqColorScheme *color_scheme = state->color_schemes + i;
        if (strcmp(name, color_scheme->scheme.name) == 0)
        {
            has_scheme = 1;
            color_deinit_color_scheme(&color_scheme->scheme);
            for (unsigned int j = i + 1; j < state->n_color_schemes; j++)
                state->color_schemes[j - 1] = state->color_schemes[j];
            break;
        }
    }
    if (has_scheme == 0)
        return 0;

    if (state->n_color_schemes > 1)
    {
        SeqColorScheme *color_schemes = realloc(state->color_schemes,
                                                (state->n_color_schemes - 1) * sizeof(SeqColorScheme));
        if (!color_schemes)
            return -1;
        state->color_schemes = color_schemes;
        state->n_color_schemes -= 1;
    }
    else
    {
        free(state->color_schemes);
        state->color_schemes = NULL;
        state->n_color_schemes = 0;
    }

    state->refresh_sequence_pane = true;

    return 0;
}
