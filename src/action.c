#include <stdbool.h>
#include <string.h>

#include "action.h"
#include "pane.h"
#include "scroller.h"
#include "state.h"

extern State state;

void action_quit(void)
{
    exit(0);
}

void action_next_file(void)
{
    if (state.active_file_index + 1 >= state.nfiles)
        return;
    state_set_active_file_index(&state, state.active_file_index + 1);
    state.refresh_window = true;
}

void action_previous_file(void)
{
    if (state.active_file_index == 0)
        return;
    state_set_active_file_index(&state, state.active_file_index - 1);
    state.refresh_window = true;
}

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
    SeqRecord record = active_file->record_array.data[record_index];
    size_t line_len;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        line_len = strlen(record.header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        line_len = record.len;
        break;
    }
    scroller_move_right(&active_file->layout.scroller, active_file->record_array.len, line_len, x);
}

void action_move_left(size_t x)
{
    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord record = active_file->record_array.data[record_index];
    size_t line_len;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        line_len = strlen(record.header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        line_len = record.len;
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
    scroller_move_page_right(scroller, active_file->record_array.len, active_file->records_maxlen, page_size);
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
    SeqRecord record = active_file->record_array.data[record_index];
    size_t line_len;
    switch (scroller->active_pane_index)
    {
    case SCROLLER_HEADER_PANE:
        line_len = strlen(record.header);
        break;
    case SCROLLER_SEQUENCE_PANE:
        line_len = record.len;
        break;
    }
    scroller_move_line_end(scroller, active_file->record_array.len, line_len);
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
    state_set_header_sequence_divider_j(&state, active_file->layout.header_sequence_divider_j + 1);
}

void action_decrease_header_sequence_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_header_sequence_divider_j(&state, active_file->layout.header_sequence_divider_j - 1);
}

void action_increase_ruler_records_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_records_divider_i(&state, active_file->layout.ruler_records_divider_i + 1);
}

void action_decrease_ruler_records_divider(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_records_divider_i(&state, active_file->layout.ruler_records_divider_i - 1);
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
    state.refresh_window = true;
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
