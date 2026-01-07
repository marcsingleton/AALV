#include <stdbool.h>

#include "action.h"
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

void action_cursor_clamp(void)
{
    FileState *active_file = state.active_file;
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    if (active_file->cursor_record_i + 1 > record_panes_height)
        active_file->cursor_record_i = record_panes_height - 1;
    if (active_file->cursor_sequence_j > sequence_pane_width)
        active_file->cursor_sequence_j = sequence_pane_width - 1;
}

void action_move_up(size_t x)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x > record_index)
        x = record_index;
    if (x > active_file->cursor_record_i)
    {
        active_file->cursor_record_i = 0;
        state_set_offset_record(&state, record_index - x);
    }
    else
        active_file->cursor_record_i -= x;
}

void action_move_down(size_t x)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x + record_index + 1 >= active_file->record_array.len)
        x = active_file->record_array.len - record_index - 1;
    if (x + active_file->cursor_record_i + 1 > record_panes_height)
    {
        x += active_file->cursor_record_i - record_panes_height + 1;
        active_file->cursor_record_i = record_panes_height - 1;
        state_set_offset_record(&state, active_file->offset_record + x);
    }
    else
        active_file->cursor_record_i += x;
}

void action_move_right(size_t x)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.data[record_index];
    size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

    // Snap to end
    if (sequence_index + 1 > record.len)
    {
        sequence_index = (record.len > 0) ? record.len - 1 : 0;
        if (sequence_index < active_file->offset_sequence)
        {
            active_file->cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            active_file->cursor_sequence_j = sequence_index - active_file->offset_sequence;
    }

    // Move
    if (record.len == 0)
        return;
    if (x + sequence_index + 1 >= record.len)
        x = record.len - sequence_index - 1;
    if (x + active_file->cursor_sequence_j + 1 > sequence_pane_width)
    {
        x += active_file->cursor_sequence_j - sequence_pane_width + 1;
        active_file->cursor_sequence_j = sequence_pane_width - 1;
        state_set_offset_sequence(&state, active_file->offset_sequence + x);
    }
    else
        active_file->cursor_sequence_j += x;
}

void action_move_left(size_t x)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.data[record_index];
    size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

    // Snap to end
    if (sequence_index + 1 > record.len)
    {
        sequence_index = (record.len > 0) ? record.len - 1 : 0;
        if (sequence_index < active_file->offset_sequence)
        {
            active_file->cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            active_file->cursor_sequence_j = sequence_index - active_file->offset_sequence;
    }

    // Move
    if (x > sequence_index)
        x = sequence_index;
    if (x > active_file->cursor_sequence_j)
    {
        active_file->cursor_sequence_j = 0;
        state_set_offset_sequence(&state, sequence_index - x);
    }
    else
        active_file->cursor_sequence_j -= x;
}

void action_move_page_up(PageSize page_size)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    size_t x = record_panes_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_record == 0)
        return;
    else if (x > active_file->offset_record)
        state_set_offset_record(&state, 0);
    else
        state_set_offset_record(&state, active_file->offset_record - x);
}

void action_move_page_down(PageSize page_size)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int x = record_panes_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_record + 1 == active_file->record_array.len)
        return;
    else if (active_file->offset_record + x + 1 > active_file->record_array.len)
        state_set_offset_record(&state, active_file->record_array.len - 1);
    else
        state_set_offset_record(&state, active_file->offset_record + x);
    if (active_file->offset_record + active_file->cursor_record_i + 1 > active_file->record_array.len)
        active_file->cursor_record_i = active_file->record_array.len - 1 - active_file->offset_record;
}

void action_move_page_right(PageSize page_size)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int x = state_get_sequence_pane_width(&state);
    size_t maxlen = active_file->records_maxlen;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_sequence + x + 2 > maxlen) // Accounts for continuation symbol
        state_set_offset_sequence(&state, maxlen - 2);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence + x);
    if (active_file->cursor_sequence_j + active_file->offset_sequence + 1 > maxlen) // Check for cursor exceeding sequence end
    {
        size_t sequence_index = (maxlen > 0) ? maxlen - active_file->offset_sequence - 1 : 0;
        active_file->cursor_sequence_j = sequence_index;
    }
}

void action_move_page_left(PageSize page_size)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int x = state_get_sequence_pane_width(&state);
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (x > active_file->offset_sequence)
        state_set_offset_sequence(&state, 0);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence - x);
}

void action_move_line_start(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    state_set_offset_sequence(&state, 0);
    active_file->cursor_sequence_j = 0;
}

void action_move_line_end(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.data[record_index];
    size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
    size_t x = (record.len > 0) ? record.len - 1 - sequence_index : 0;
    action_move_right(x);
}

void action_move_first_record(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_record_i = 0;
    state_set_offset_record(&state, 0);
}

void action_move_last_record(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    size_t x = active_file->record_array.len - 1 - record_index;
    action_move_down(x);
}

void action_move_to_record(size_t x)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x > record_index)
        action_move_down(x - record_index);
    else if (x < record_index)
        action_move_up(record_index - x);
}

void action_move_bottom_edge(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->record_array.len)
        active_file->cursor_record_i = active_file->record_array.len - active_file->offset_record - 1;
    else
        active_file->cursor_record_i = record_panes_height - 1;
}

void action_move_top_edge(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_record_i = 0;
}

void action_move_left_edge(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_sequence_j = 0;
}

void action_move_right_edge(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_sequence_j = state_get_sequence_pane_width(&state) - 1;
}

void action_move_vertical_middle(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->record_array.len)
        active_file->cursor_record_i = (active_file->record_array.len - active_file->offset_record - 1) / 2;
    else
        active_file->cursor_record_i = (record_panes_height - 1) / 2;
}

void action_move_horizontal_middle(void)
{
    action_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    active_file->cursor_sequence_j = sequence_pane_width / 2;
}

void action_increase_header_pane_width(void)
{
    FileState *active_file = state.active_file;
    state_set_header_pane_width(&state, active_file->header_pane_width + 1);
}

void action_decrease_header_pane_width(void)
{
    FileState *active_file = state.active_file;
    state_set_header_pane_width(&state, active_file->header_pane_width - 1);
}

void action_increase_ruler_pane_height(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_pane_height(&state, active_file->ruler_pane_height + 1);
}

void action_decrease_ruler_pane_height(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_pane_height(&state, active_file->ruler_pane_height - 1);
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
