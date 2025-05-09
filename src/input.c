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
    case '$':
        input_move_line_end();
        break;
    case '0':
    case '^':
        input_move_line_start();
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
        return;

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
        return;

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

void input_move_line_start(void)
{
    state_set_offset_sequence(&state, 0);
    state.cursor_sequence_j = 0;
}

void input_move_line_end(void)
{
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int index_max = state.terminal_cols - state.header_pane_width + state.offset_sequence;
    if (record.len < state.offset_sequence)
    {
        unsigned int offset_sequence = (record.len > 1) ? record.len - 2 : 0;
        state_set_offset_sequence(&state, offset_sequence);
        state.cursor_sequence_j = 0;
    }
    else if (record.len < index_max)
        state.cursor_sequence_j = record.len - state.offset_sequence - 1;
    else
    {
        unsigned int sequence_pane_width = state.terminal_cols - state.header_pane_width;
        state_set_offset_sequence(&state, record.len - sequence_pane_width);
        state.cursor_sequence_j = sequence_pane_width - 1;
    }
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
