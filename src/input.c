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
    tv.tv_usec = 200000;

    int nready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    char c = '\0';
    if (nready > 0 && FD_ISSET(STDIN_FILENO, &readfds))
    {
        read(STDIN_FILENO, &c, 1);
    }

    return (int)c;
}

int input_process_action(int action, Array *buffer)
{
    switch (action)
    {
    case 'j':
        input_move_down();
        break;
    case 'k':
        input_move_up();
        break;
    case 'l':
        input_move_right();
        break;
    case 'h':
        input_move_left();
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

void input_move_up(void)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        return;

    if (state.cursor_record_i >= record_panes_height)
        state.cursor_record_i = record_panes_height - 1;
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    if (record_index == 0)
        return;

    if (state.cursor_record_i > 0)
        state.cursor_record_i--;
    else
        state_set_offset_record(&state, state.offset_record - 1);
}

void input_move_down(void)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        return;

    if (state.cursor_record_i >= record_panes_height)
        state.cursor_record_i = record_panes_height - 1;
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    if (record_index >= state.record_array.len - 1)
        return;

    if (state.cursor_record_i + state.ruler_pane_height < state.terminal_rows - 3)
        state.cursor_record_i++;
    else
        state_set_offset_record(&state, state.offset_record + 1);
}

void input_move_right(void)
{
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_pane_width = state.terminal_cols - state.header_pane_width;
    unsigned int sequence_index = state.cursor_sequence_j + state.offset_sequence;
    if (sequence_index >= record.len - 1)
        return;

    if (state.cursor_sequence_j < sequence_pane_width - 1)
        state.cursor_sequence_j++;
    else if (state.cursor_sequence_j == sequence_pane_width - 1)
        state_set_offset_sequence(&state, state.offset_sequence + 1);
}

void input_move_left(void)
{
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_index = state.cursor_sequence_j + state.offset_sequence;
    if (sequence_index == 0)
        return;

    if (sequence_index > record.len)
    {
        if (record.len < state.offset_sequence + 1)
        {
            unsigned int offset_sequence = (record.len > 1) ? record.len - 2 : 0;
            state_set_offset_sequence(&state, offset_sequence);
            state.cursor_sequence_j = 0;
        }
        else if (record.len == state.offset_sequence + 1)
        {
            unsigned int offset_sequence = (state.offset_sequence > 1) ? state.offset_sequence - 1 : 0;
            state_set_offset_sequence(&state, offset_sequence);
            state.cursor_sequence_j = 0;
        }
        else
            state.cursor_sequence_j = record.len - state.offset_sequence - 1;
    }
    else if (state.cursor_sequence_j > 0)
        state.cursor_sequence_j--;
    else if (state.cursor_sequence_j == 0)
        state_set_offset_sequence(&state, state.offset_sequence - 1);
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
