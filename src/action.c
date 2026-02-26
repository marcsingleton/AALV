#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "action.h"
#include "pane.h"
#include "scroller.h"
#include "state.h"

extern State state;

void action_move_up(size_t x)
{
    FileState *active_file = state.active_file;
    scroller_move_up(&active_file->layout.scroller, active_file->record_array.len, x);
}

void action_move_down(size_t x)
{
    FileState *active_file = state.active_file;
    scroller_move_down(&active_file->layout.scroller, active_file->record_array.len, x);
}

void action_move_right(size_t x)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t line_len;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        line_len = strlen(record->header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        line_len = record->len;
        break;
    }
    scroller_move_right(&active_file->layout.scroller, active_file->record_array.len, line_len, x);
}

void action_move_left(size_t x)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t line_len;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        line_len = strlen(record->header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        line_len = record->len;
        break;
    }
    scroller_move_left(&active_file->layout.scroller, active_file->record_array.len, line_len, x);
}

void action_move_page_up(PageSize page_size)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_up(scroller, active_file->record_array.len, page_size);
}

void action_move_page_down(PageSize page_size)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_down(scroller, active_file->record_array.len, page_size);
}

void action_move_page_right(PageSize page_size)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_right(scroller, active_file->record_array.len, active_file->records_max_len, page_size);
}

void action_move_page_left(PageSize page_size)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_page_left(scroller, active_file->record_array.len, page_size);
}

void action_move_line_start(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_line_start(scroller, active_file->record_array.len);
}

void action_move_line_end(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t line_len;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        line_len = strlen(record->header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        line_len = record->len;
        break;
    }
    scroller_move_line_end(scroller, active_file->record_array.len, line_len);
}

void action_move_first_non_gap_or_non_whitespace(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t line_len;
    size_t new_index;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
    {
        line_len = strlen(record->header);
        for (new_index = 0; new_index < line_len; new_index++)
        {
            if (!isspace(record->header[new_index]))
                break;
        }
        break;
    }
    case SCROLLER_SEQUENCE_PANE:
        line_len = record->len;
        Alphabet *alphabet = sequences_seq_type_to_alphabet(record->type);
        if (alphabet)
        {
            for (new_index = 0; new_index < line_len; new_index++)
                if (!sequences_sym_is_gap(alphabet, record->seq[new_index]))
                    break;
        }
        else
            new_index = 0;
    }
    unsigned int active_index = scroller->active_pane_index;
    size_t old_index = scroller->offsets_j[active_index] + scroller->cursors_j[active_index];
    if (new_index > old_index)
        scroller_move_right(scroller, active_file->record_array.len, line_len, new_index - old_index);
    else if (old_index > new_index)
        scroller_move_left(scroller, active_file->record_array.len, line_len, old_index - new_index);
}

void action_move_last_non_gap_or_non_whitespace(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    size_t line_len;
    size_t new_index;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
    {
        line_len = strlen(record->header);
        new_index = (line_len > 0) ? line_len - 1 : 0;
        for (; 0 < new_index; new_index--)
        {
            if (!isspace(record->header[new_index]))
                break;
        }
        break;
    }
    case SCROLLER_SEQUENCE_PANE:
        line_len = record->len;
        Alphabet *alphabet = sequences_seq_type_to_alphabet(record->type);
        if (alphabet)
        {
            new_index = (line_len > 0) ? line_len - 1 : 0;
            for (; 0 < new_index; new_index--)
                if (!sequences_sym_is_gap(alphabet, record->seq[new_index]))
                    break;
        }
        else
            new_index = 0;
    }
    unsigned int active_index = scroller->active_pane_index;
    size_t old_index = scroller->offsets_j[active_index] + scroller->cursors_j[active_index];
    if (new_index > old_index)
        scroller_move_right(scroller, active_file->record_array.len, line_len, new_index - old_index);
    else if (old_index > new_index)
        scroller_move_left(scroller, active_file->record_array.len, line_len, old_index - new_index);
}

void action_move_first_record(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_first_line(scroller, active_file->record_array.len);
}

void action_move_last_record(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_last_line(scroller, active_file->record_array.len);
}

void action_move_to_record(size_t x)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_to_line(scroller, active_file->record_array.len, x);
}

void action_move_bottom_edge(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_bottom_edge(scroller, active_file->record_array.len);
}

void action_move_top_edge(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_top_edge(scroller, active_file->record_array.len);
}

void action_move_left_edge(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_left_edge(scroller, active_file->record_array.len);
}

void action_move_right_edge(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_right_edge(scroller, active_file->record_array.len);
}

void action_move_vertical_middle(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_vertical_middle(scroller, active_file->record_array.len);
}

void action_move_horizontal_middle(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_move_horizontal_middle(scroller, active_file->record_array.len);
}

void action_increase_header_sequence_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_header_sequence_divider(&state, active_file->layout.header_sequence_divider + 1);
}

void action_decrease_header_sequence_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_header_sequence_divider(&state, active_file->layout.header_sequence_divider - 1);
}

void action_increase_ruler_records_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_records_divider(&state, active_file->layout.ruler_records_divider + 1);
}

void action_decrease_ruler_records_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_records_divider(&state, active_file->layout.ruler_records_divider - 1);
}

void action_increase_tick_spacing(void)
{
    FileState *active_file = state.active_file;
    state_set_tick_spacing(&state, active_file->tick_spacing + 1);
}

void action_decrease_tick_spacing(void)
{
    FileState *active_file = state.active_file;
    state_set_tick_spacing(&state, active_file->tick_spacing - 1);
}

void action_enter_command_mode(void)
{
    state.mode = COMMAND;
    state.refresh_command_pane = true;
}

void action_set_header_pane_active(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_set_active_pane(scroller, SCROLLER_HEADER_PANE);
}

void action_set_sequence_pane_active(void)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    scroller_set_active_pane(scroller, SCROLLER_SEQUENCE_PANE);
}
