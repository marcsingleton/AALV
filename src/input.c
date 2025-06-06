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
    case 'u':
        input_move_page_up(HALF);
        break;
    case 'd':
        input_move_page_down(HALF);
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

void input_move_up(unsigned int x)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    if (state.cursor_record_i > record_panes_height - 1)
        state.cursor_record_i = record_panes_height - 1;
    if (state.cursor_sequence_j > sequence_pane_width)
        state.cursor_sequence_j = sequence_pane_width - 1;

    unsigned int record_index = state.cursor_record_i + state.offset_record;
    if (x > record_index)
        x = record_index;
    if (x > state.cursor_record_i)
    {
        state.cursor_record_i = 0;
        state_set_offset_record(&state, record_index - x);
    }
    else
        state.cursor_record_i -= x;
}

void input_move_down(unsigned int x)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    if (state.cursor_record_i > record_panes_height - 1)
        state.cursor_record_i = record_panes_height - 1;
    if (state.cursor_sequence_j > sequence_pane_width)
        state.cursor_sequence_j = sequence_pane_width - 1;

    unsigned int record_index = state.cursor_record_i + state.offset_record;
    if (x + record_index >= state.record_array.len - 1)
        x = state.record_array.len - 1 - record_index;
    if (x + state.cursor_record_i > record_panes_height - 1)
    {
        x += state.cursor_record_i - record_panes_height + 1;
        state.cursor_record_i = record_panes_height - 1;
        state_set_offset_record(&state, state.offset_record + x);
    }
    else
        state.cursor_record_i += x;
}

void input_move_right(unsigned int x)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    if (state.cursor_record_i > record_panes_height - 1)
        state.cursor_record_i = record_panes_height - 1;
    if (state.cursor_sequence_j > sequence_pane_width)
        state.cursor_sequence_j = sequence_pane_width - 1;

    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_index = state.cursor_sequence_j + state.offset_sequence;

    if (sequence_index > record.len - 1)
    {
        sequence_index = record.len - 1;
        if (sequence_index < state.offset_sequence)
        {
            state.cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            state.cursor_sequence_j = sequence_index - state.offset_sequence;
    }

    if (x + sequence_index >= record.len - 1)
        x = record.len - 1 - sequence_index;
    if (x + state.cursor_sequence_j > sequence_pane_width - 1)
    {
        x += state.cursor_sequence_j - sequence_pane_width + 1;
        state.cursor_sequence_j = sequence_pane_width - 1;
        state_set_offset_sequence(&state, state.offset_sequence + x);
    }
    else
        state.cursor_sequence_j += x;
}

void input_move_left(unsigned int x)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    if (state.cursor_record_i > record_panes_height - 1)
        state.cursor_record_i = record_panes_height - 1;
    if (state.cursor_sequence_j > sequence_pane_width)
        state.cursor_sequence_j = sequence_pane_width - 1;

    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_index = state.cursor_sequence_j + state.offset_sequence;

    if (sequence_index > record.len - 1)
    {
        sequence_index = record.len - 1;
        if (sequence_index < state.offset_sequence)
        {
            state.cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            state.cursor_sequence_j = sequence_index - state.offset_sequence;
    }

    if (x > sequence_index)
        x = sequence_index;
    if (x > state.cursor_sequence_j)
    {
        state.cursor_sequence_j = 0;
        state_set_offset_sequence(&state, sequence_index - x);
    }
    else
        state.cursor_sequence_j -= x;
}

void input_move_page_up(PageSize page_size)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    if (state.cursor_record_i > record_panes_height - 1)
        state.cursor_record_i = record_panes_height - 1;
    if (state.cursor_sequence_j > sequence_pane_width)
        state.cursor_sequence_j = sequence_pane_width - 1;

    unsigned int x = record_panes_height;
    if (page_size == HALF)
        x /= 2;
    if (x > state.offset_record)
        state_set_offset_record(&state, 0);
    else
        state_set_offset_record(&state, state.offset_record - x);
}

void input_move_page_down(PageSize page_size)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    if (state.cursor_record_i > record_panes_height - 1)
        state.cursor_record_i = record_panes_height - 1;
    if (state.cursor_sequence_j > sequence_pane_width)
        state.cursor_sequence_j = sequence_pane_width - 1;

    unsigned int x = record_panes_height;
    if (page_size == HALF)
        x /= 2;
    if (state.offset_record + x > state.record_array.len - 1)
        state_set_offset_record(&state, state.record_array.len - 1);
    else
        state_set_offset_record(&state, state.offset_record + x);
    if (state.offset_record + state.cursor_record_i > state.record_array.len - 1)
        state.cursor_record_i = state.record_array.len - 1 - state.offset_record;
}

void input_move_page_right(void)
{
}

void input_move_page_left(void)
{
}

void input_move_line_start(void)
{
    state_set_offset_sequence(&state, 0);
    state.cursor_sequence_j = 0;
}

void input_move_line_end(void)
{

    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_index = state.cursor_sequence_j + state.offset_sequence;
    input_move_right(record.len - 1 - sequence_index);
}

void input_move_first_record(void)
{
    state.cursor_record_i = 0;
    state_set_offset_record(&state, 0);
}

void input_move_last_record(void)
{
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    unsigned int x = state.record_array.len - 1 - record_index;
    input_move_down(x);
}

void input_move_bottom_edge(void)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (state.offset_record + record_panes_height > state.record_array.len)
        state.cursor_record_i = state.record_array.len - state.offset_record - 1;
    else
        state.cursor_record_i = record_panes_height - 1;
}

void input_move_top_edge(void)
{
    state.cursor_record_i = 0;
}

void input_move_left_edge(void)
{
    state.cursor_sequence_j = 0;
}

void input_move_right_edge(void)
{
    state.cursor_sequence_j = state_get_sequence_pane_width(&state) - 1;
}

void input_move_vertical_middle(void)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (state.offset_record + record_panes_height > state.record_array.len)
        state.cursor_record_i = (state.record_array.len - state.offset_record - 1) / 2;
    else
        state.cursor_record_i = (record_panes_height - 1) / 2;
}

void input_move_horizontal_middle(void)
{
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    state.cursor_sequence_j = sequence_pane_width / 2;
}

void input_increase_header_pane_width(void)
{
    state_set_header_pane_width(&state, state.header_pane_width + 1);
}

void input_decrease_header_pane_width(void)
{
    state_set_header_pane_width(&state, state.header_pane_width - 1);
}

void input_increase_ruler_pane_height(void)
{
    state_set_ruler_pane_height(&state, state.ruler_pane_height + 1);
}

void input_decrease_ruler_pane_height(void)
{
    state_set_ruler_pane_height(&state, state.ruler_pane_height - 1);
}

void input_increase_tick_spacing(void)
{
    state_set_tick_spacing(&state, state.tick_spacing + 1);
}

void input_decrease_tick_spacing(void)
{
    state_set_tick_spacing(&state, state.tick_spacing - 1);
}
