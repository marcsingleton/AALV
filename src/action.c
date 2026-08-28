#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "action.h"
#include "scroller.h"

// Private
static size_t get_page_ncols(FileState *active_file)
{
    if (!active_file->record_array.data)
        return 0;

    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t max_index;
    if (scroller->h > active_file->record_array.len - scroller->offset_i) // Invariant: offset < len
        max_index = active_file->record_array.len;
    else
        max_index = scroller->offset_i + scroller->h;
    size_t ncols = 0;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        for (size_t record_index = scroller->offset_i; record_index < max_index; record_index++)
        {
            SeqRecord *record = active_file->record_array.data + record_index;
            size_t row_ncols = strlen(record->header);
            if (row_ncols > ncols)
                ncols = row_ncols;
        }
        break;
    case SCROLLER_SEQUENCE_PANE:
        for (size_t record_index = scroller->offset_i; record_index < max_index; record_index++)
        {
            SeqRecord *record = active_file->record_array.data + record_index;
            size_t row_ncols = record->len;
            if (row_ncols > ncols)
                ncols = row_ncols;
        }
    }

    return ncols;
}

static size_t get_row_ncols(FileState *active_file)
{
    if (!active_file->record_array.data)
        return 0;

    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t ncols;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        ncols = strlen(record->header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        ncols = record->len;
        break;
    }
    return ncols;
}

// Public
void action_move_up(State *state, size_t x)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_up(scroller, active_file->record_array.len, x);
}

void action_move_down(State *state, size_t x)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_down(scroller, active_file->record_array.len, x);
}

void action_move_right(State *state, size_t x)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);
    scroller_move_right(scroller, active_file->record_array.len, ncols, x);
}

void action_move_left(State *state, size_t x)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);
    scroller_move_left(scroller, active_file->record_array.len, ncols, x);
}

void action_move_page_up(State *state, PageSize page_size)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_up(scroller, active_file->record_array.len, page_size);
}

void action_move_page_down(State *state, PageSize page_size)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_down(scroller, active_file->record_array.len, page_size);
}

void action_move_page_right(State *state, PageSize page_size)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_page_ncols(active_file);
    scroller_move_page_right(scroller, active_file->record_array.len, ncols, page_size);
}

void action_move_page_left(State *state, PageSize page_size)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_left(scroller, active_file->record_array.len, page_size);
}

void action_move_row_start(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_row_start(scroller, active_file->record_array.len);
}

void action_move_row_middle(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);
    scroller_move_to_column(scroller, active_file->record_array.len, ncols, ncols / 2);
}

void action_move_row_end(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);
    scroller_move_row_end(scroller, active_file->record_array.len, ncols);
}

void action_move_to_column(State *state, size_t x)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);
    scroller_move_to_column(scroller, active_file->record_array.len, ncols, x);
}

void action_move_first_non_gap_or_non_whitespace(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);

    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t x;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
    {
        for (x = 0; x < ncols; x++)
        {
            if (!isspace(record->header[x]))
                break;
        }
        break;
    }
    case SCROLLER_SEQUENCE_PANE:
    {
        Alphabet *alphabet = sequences_seq_type_to_alphabet(record->type);
        if (alphabet)
        {
            for (x = 0; x < ncols; x++)
                if (!sequences_sym_is_gap(alphabet, record->seq[x]))
                    break;
        }
        else
            x = 0;
        break;
    }
    }

    scroller_move_to_column(scroller, active_file->record_array.len, ncols, x);
}

void action_move_last_non_gap_or_non_whitespace(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t ncols = get_row_ncols(active_file);

    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t x;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
    {
        x = (ncols > 0) ? ncols - 1 : 0;
        for (; 0 < x; x--)
        {
            if (!isspace(record->header[x]))
                break;
        }
        break;
    }
    case SCROLLER_SEQUENCE_PANE:
    {
        Alphabet *alphabet = sequences_seq_type_to_alphabet(record->type);
        if (alphabet)
        {
            x = (ncols > 0) ? ncols - 1 : 0;
            for (; 0 < x; x--)
                if (!sequences_sym_is_gap(alphabet, record->seq[x]))
                    break;
        }
        else
            x = 0;
        break;
    }
    }

    scroller_move_to_column(scroller, active_file->record_array.len, ncols, x);
}

void action_move_first_row(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_first_row(scroller, active_file->record_array.len);
}

void action_move_last_row(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_last_row(scroller, active_file->record_array.len);
}

void action_move_to_row(State *state, size_t x)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_to_row(scroller, active_file->record_array.len, x);
}

void action_move_top_edge(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_top_edge(scroller, active_file->record_array.len);
}

void action_move_bottom_edge(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_bottom_edge(scroller, active_file->record_array.len);
}

void action_move_right_edge(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_right_edge(scroller, active_file->record_array.len);
}

void action_move_left_edge(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_left_edge(scroller, active_file->record_array.len);
}

void action_move_vertical_middle(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_vertical_middle(scroller, active_file->record_array.len);
}

void action_move_horizontal_middle(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_horizontal_middle(scroller, active_file->record_array.len);
}

void action_move_row_to_center(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_row_to_center(scroller, active_file->record_array.len);
}

void action_increase_header_sequence_divider(State *state)
{
    FileState *active_file = state->active_file;
    state_set_header_sequence_divider(state, active_file->layout.header_sequence_divider + 1);
}

void action_decrease_header_sequence_divider(State *state)
{
    FileState *active_file = state->active_file;
    state_set_header_sequence_divider(state, active_file->layout.header_sequence_divider - 1);
}

void action_increase_ruler_records_divider(State *state)
{
    FileState *active_file = state->active_file;
    state_set_ruler_records_divider(state, active_file->layout.ruler_records_divider + 1);
}

void action_decrease_ruler_records_divider(State *state)
{
    FileState *active_file = state->active_file;
    state_set_ruler_records_divider(state, active_file->layout.ruler_records_divider - 1);
}

void action_increase_tick_spacing(State *state)
{
    FileState *active_file = state->active_file;
    state_set_tick_spacing(state, active_file->layout.tick_spacing + 1);
}

void action_decrease_tick_spacing(State *state)
{
    FileState *active_file = state->active_file;
    state_set_tick_spacing(state, active_file->layout.tick_spacing - 1);
}

void action_enter_command_mode(State *state)
{
    state->mode = COMMAND;
    state->refresh_command_pane = true;
}

void action_set_header_pane_active(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_set_active_pane(scroller, SCROLLER_HEADER_PANE);
}

void action_set_sequence_pane_active(State *state)
{
    FileState *active_file = state->active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_set_active_pane(scroller, SCROLLER_SEQUENCE_PANE);
}
