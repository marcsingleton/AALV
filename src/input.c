#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "array.h"
#include "input.h"
#include "state.h"
#include "terminal.h"

extern State state;

int input_get_action(void)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    int nready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    char c = '\0';
    if (nready > 0 && FD_ISSET(STDIN_FILENO, &readfds))
        read(STDIN_FILENO, &c, 1);

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
        input_move_page_up(FULL);
        break;
    case 'D':
        input_move_page_down(FULL);
        break;
    case 'M':
        input_move_page_right(FULL);
        break;
    case 'N':
        input_move_page_left(FULL);
        break;
    case 'u':
        input_move_page_up(HALF);
        break;
    case 'd':
        input_move_page_down(HALF);
        break;
    case 'm':
        input_move_page_right(HALF);
        break;
    case 'n':
        input_move_page_left(HALF);
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
    if (state.active_file_index == state.nfiles - 1)
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

    if (active_file->cursor_record_i > record_panes_height - 1)
        active_file->cursor_record_i = record_panes_height - 1;
    if (active_file->cursor_sequence_j > sequence_pane_width)
        active_file->cursor_sequence_j = sequence_pane_width - 1;
}

void input_move_up(unsigned int x)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

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
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x + record_index >= active_file->record_array.len - 1)
        x = active_file->record_array.len - 1 - record_index;
    if (x + active_file->cursor_record_i > record_panes_height - 1)
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
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

    if (sequence_index > record.len - 1)
    {
        sequence_index = record.len - 1;
        if (sequence_index < active_file->offset_sequence)
        {
            active_file->cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            active_file->cursor_sequence_j = sequence_index - active_file->offset_sequence;
    }

    if (x + sequence_index >= record.len - 1)
        x = record.len - 1 - sequence_index;
    if (x + active_file->cursor_sequence_j > sequence_pane_width - 1)
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
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

    if (sequence_index > record.len - 1)
    {
        sequence_index = record.len - 1;
        if (sequence_index < active_file->offset_sequence)
        {
            active_file->cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            active_file->cursor_sequence_j = sequence_index - active_file->offset_sequence;
    }

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
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int x = record_panes_height;
    if (page_size == HALF)
        x /= 2;
    if (x > active_file->offset_record)
        state_set_offset_record(&state, 0);
    else
        state_set_offset_record(&state, active_file->offset_record - x);
}

void input_move_page_down(PageSize page_size)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int x = record_panes_height;
    if (page_size == HALF)
        x /= 2;
    if (active_file->offset_record + x > active_file->record_array.len - 1)
        state_set_offset_record(&state, active_file->record_array.len - 1);
    else
        state_set_offset_record(&state, active_file->offset_record + x);
    if (active_file->offset_record + active_file->cursor_record_i > active_file->record_array.len - 1)
        active_file->cursor_record_i = active_file->record_array.len - 1 - active_file->offset_record;
}

void input_move_page_right(PageSize page_size)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int max_len = 0;
    for (unsigned int i = 0; i < active_file->record_array.len; i++)
    {
        if (active_file->record_array.records[i].len > max_len)
            max_len = active_file->record_array.records[i].len;
    }

    unsigned int x = state_get_sequence_pane_width(&state);
    if (page_size == HALF)
        x /= 2;
    if (active_file->offset_sequence + x > max_len - 2) // Accounts for continuation symbol
        state_set_offset_sequence(&state, max_len - 2);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence + x);
}

void input_move_page_left(PageSize page_size)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int x = state_get_sequence_pane_width(&state);
    if (page_size == HALF)
        x /= 2;
    if (x > active_file->offset_sequence)
        state_set_offset_sequence(&state, 0);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence - x);
}

void input_move_line_start(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    state_set_offset_sequence(&state, 0);
    active_file->cursor_sequence_j = 0;
}

void input_move_line_end(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
    input_move_right(record.len - 1 - sequence_index);
}

void input_move_first_record(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    active_file->cursor_record_i = 0;
    state_set_offset_record(&state, 0);
}

void input_move_last_record(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_index = active_file->cursor_record_i + active_file->offset_record;
    unsigned int x = active_file->record_array.len - 1 - record_index;
    input_move_down(x);
}

void input_move_bottom_edge(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->record_array.len)
        active_file->cursor_record_i = active_file->record_array.len - active_file->offset_record - 1;
    else
        active_file->cursor_record_i = record_panes_height - 1;
}

void input_move_top_edge(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    active_file->cursor_record_i = 0;
}

void input_move_left_edge(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    active_file->cursor_sequence_j = 0;
}

void input_move_right_edge(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    active_file->cursor_sequence_j = state_get_sequence_pane_width(&state) - 1;
}

void input_move_vertical_middle(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->record_array.len)
        active_file->cursor_record_i = (active_file->record_array.len - active_file->offset_record - 1) / 2;
    else
        active_file->cursor_record_i = (record_panes_height - 1) / 2;
}

void input_move_horizontal_middle(void)
{
    FileState *active_file = state.active_file;
    input_cursor_clamp();

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
