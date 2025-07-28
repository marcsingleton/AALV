#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

#include "array.h"
#include "input.h"
#include "state.h"
#include "terminal.h"

extern State state;

int input_get_action(int fd)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    int nready = select(fd + 1, &readfds, NULL, NULL, &tv);

    char c = '\0';
    if (nready > 0 && FD_ISSET(fd, &readfds))
        read(fd, &c, 1);

    return (int)c;
}

int input_process_action(int action, Array *buffer)
{
    switch (action)
    {
    case '>':
        input_next_file();
        break;
    case '<':
        input_previous_file();
        break;
    case 'j':
        input_move_down(1);
        break;
    case 'J':
        input_move_down(5);
        break;
    case 'k':
        input_move_up(1);
        break;
    case 'K':
        input_move_up(5);
        break;
    case 'l':
        input_move_right(1);
        break;
    case 'L':
        input_move_right(5);
        break;
    case 'h':
        input_move_left(1);
        break;
    case 'H':
        input_move_left(5);
        break;
    case 'U':
        input_move_page_up(PAGE_SIZE_FULL);
        break;
    case 'D':
        input_move_page_down(PAGE_SIZE_FULL);
        break;
    case 'M':
        input_move_page_right(PAGE_SIZE_FULL);
        break;
    case 'N':
        input_move_page_left(PAGE_SIZE_FULL);
        break;
    case 'u':
        input_move_page_up(PAGE_SIZE_HALF);
        break;
    case 'd':
        input_move_page_down(PAGE_SIZE_HALF);
        break;
    case 'm':
        input_move_page_right(PAGE_SIZE_HALF);
        break;
    case 'n':
        input_move_page_left(PAGE_SIZE_HALF);
        break;
    case '$':
        input_move_line_end();
        break;
    case '0':
    case '^':
        input_move_line_start();
        break;
    case 'g':
        input_move_first_record();
        break;
    case 'G':
        input_move_last_record();
        break;
    case 'w':
        input_move_top_edge();
        break;
    case 's':
        input_move_vertical_middle();
        break;
    case 'x':
        input_move_bottom_edge();
        break;
    case 'e':
        input_move_left_edge();
        break;
    case 'r':
        input_move_horizontal_middle();
        break;
    case 't':
        input_move_right_edge();
        break;
    case ']':
        input_increase_header_pane_width();
        break;
    case '[':
        input_decrease_header_pane_width();
        break;
    case '}':
        input_increase_ruler_pane_height();
        break;
    case '{':
        input_decrease_ruler_pane_height();
        break;
    case '+':
        input_increase_tick_spacing();
        break;
    case '-':
        input_decrease_tick_spacing();
        break;
    case 'q':
        exit(0);
        break;
    }

    return 0;
}

void input_buffer_flush(Array *buffer)
{
    write(STDOUT_FILENO, buffer->data, buffer->len);
    buffer->len = 0;
}

void input_next_file(void)
{
    if (state.active_file_index + 1 >= state.nfiles)
        return;
    state_set_active_file_index(&state, state.active_file_index + 1);
    state.refresh_window = true;
}

void input_previous_file(void)
{
    if (state.active_file_index == 0)
        return;
    state_set_active_file_index(&state, state.active_file_index - 1);
    state.refresh_window = true;
}

void input_cursor_clamp(void)
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

void input_move_up(unsigned int x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
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

void input_move_down(unsigned int x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
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

void input_move_right(unsigned int x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

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

void input_move_left(unsigned int x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

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

void input_move_page_up(PageSize page_size)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int x = record_panes_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_record == 0)
        return;
    else if (x > active_file->offset_record)
        state_set_offset_record(&state, 0);
    else
        state_set_offset_record(&state, active_file->offset_record - x);
}

void input_move_page_down(PageSize page_size)
{
    input_cursor_clamp();
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

void input_move_page_right(PageSize page_size)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int x = state_get_sequence_pane_width(&state);
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_sequence + x + 2 > active_file->record_array.maxlen) // Accounts for continuation symbol
        state_set_offset_sequence(&state, active_file->record_array.maxlen - 2);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence + x);
}

void input_move_page_left(PageSize page_size)
{
    input_cursor_clamp();
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

void input_move_line_start(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    state_set_offset_sequence(&state, 0);
    active_file->cursor_sequence_j = 0;
}

void input_move_line_end(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
    unsigned int x = (record.len > 0) ? record.len - 1 - sequence_index : 0;
    input_move_right(x);
}

void input_move_first_record(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_record_i = 0;
    state_set_offset_record(&state, 0);
}

void input_move_last_record(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    unsigned int x = active_file->record_array.len - 1 - record_index;
    input_move_down(x);
}

void input_move_bottom_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->record_array.len)
        active_file->cursor_record_i = active_file->record_array.len - active_file->offset_record - 1;
    else
        active_file->cursor_record_i = record_panes_height - 1;
}

void input_move_top_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_record_i = 0;
}

void input_move_left_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_sequence_j = 0;
}

void input_move_right_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    active_file->cursor_sequence_j = state_get_sequence_pane_width(&state) - 1;
}

void input_move_vertical_middle(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->record_array.len)
        active_file->cursor_record_i = (active_file->record_array.len - active_file->offset_record - 1) / 2;
    else
        active_file->cursor_record_i = (record_panes_height - 1) / 2;
}

void input_move_horizontal_middle(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->record_array.len == 0)
        return;

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    active_file->cursor_sequence_j = sequence_pane_width / 2;
}

void input_increase_header_pane_width(void)
{
    FileState *active_file = state.active_file;
    state_set_header_pane_width(&state, active_file->header_pane_width + 1);
}

void input_decrease_header_pane_width(void)
{
    FileState *active_file = state.active_file;
    state_set_header_pane_width(&state, active_file->header_pane_width - 1);
}

void input_increase_ruler_pane_height(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_pane_height(&state, active_file->ruler_pane_height + 1);
}

void input_decrease_ruler_pane_height(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_pane_height(&state, active_file->ruler_pane_height - 1);
}

void input_increase_tick_spacing(void)
{
    FileState *active_file = state.active_file;
    state_set_tick_spacing(&state, active_file->tick_spacing + 1);
}

void input_decrease_tick_spacing(void)
{
    FileState *active_file = state.active_file;
    state_set_tick_spacing(&state, active_file->tick_spacing - 1);
}
