#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "array.h"
#include "display.h"
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
    unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
    if (record_index > 0)
    {
        if (state.cursor_i > state.ruler_pane_height + 1)
            state.cursor_i--;
        else
            state_set_offset_y(&state, state.offset_y - 1);
    }
}

void input_move_down(void)
{
    unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
    if (record_index < state.record_array.len - 1)
    {
        if (state.cursor_i < state.terminal_rows - 2)
            state.cursor_i++;
        else
            state_set_offset_y(&state, state.offset_y + 1);
    }
}

void input_move_right(void)
{
    unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int cursor_x = state.cursor_j + state.offset_x - state.header_pane_width;
    if (state.cursor_j < state.terminal_cols && cursor_x < record.len)
        state.cursor_j++;
    else if (state.cursor_j == state.terminal_cols && cursor_x < record.len)
        state_set_offset_x(&state, state.offset_x + 1);
}

void input_move_left(void)
{
    unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int cursor_x = state.cursor_j + state.offset_x - state.header_pane_width;
    if (cursor_x > record.len)
    {
        if (record.len < state.offset_x + 1)
        {
            state_set_offset_x(&state, record.len - 1);
            state.cursor_j = state.header_pane_width + 1;
        }
        else if (record.len == state.offset_x + 1)
        {
            state_set_offset_x(&state, state.offset_x - 1);
            state.cursor_j = state.header_pane_width + 1;
        }
        else
            state.cursor_j = record.len - state.offset_x + state.header_pane_width - 1;
    }
    else if (state.cursor_j > state.header_pane_width + 1)
        state.cursor_j--;
    else if (state.cursor_j == state.header_pane_width + 1 && state.offset_x > 0)
        state_set_offset_x(&state, state.offset_x - 1);
}

void input_move_line_start(void)
{
    state_set_offset_x(&state, 0);
    state.cursor_j = state.header_pane_width + 1;
}

void input_move_line_end(void)
{
    unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_pane_width = state.terminal_cols - state.header_pane_width;
    if (record.len < sequence_pane_width)
    {
        state_set_offset_x(&state, 0);
        state.cursor_j = record.len + state.header_pane_width;
    }
    else
    {
        state_set_offset_x(&state, record.len - sequence_pane_width);
        state.cursor_j = state.terminal_cols;
    }
}

void input_increase_header_pane_width(void)
{
    if (state.header_pane_width < state.terminal_cols - 1)
        state_set_header_pane_width(&state, state.header_pane_width + 1);
}

void input_decrease_header_pane_width(void)
{
    unsigned int min_width = sizeof(HEADER_PANE_ELLIPSES) - 1 + 1; // Remove null, add separator
    if (state.header_pane_width > min_width)
        state_set_header_pane_width(&state, state.header_pane_width - 1);
}
