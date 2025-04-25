#include <ctype.h>
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
    char c;
    read(STDIN_FILENO, &c, 1);
    return (int)c;
}

int input_process_action(int action, Array *buffer)
{
    terminal_cursor_hide(buffer);
    switch (action)
    {
    case 'j':
    {
        unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
        if (record_index < state.record_array.len - 1)
        {
            state.cursor_i++;
        }
        break;
    }
    case 'k':
        if (state.cursor_i > state.ruler_pane_height + 1)
        {
            state.cursor_i--;
        }
        break;
    case 'h':
        if (state.cursor_j > state.header_pane_width + 1)
        {
            state.cursor_j--;
        }
        break;
    case 'l':
    {
        unsigned int record_index = state.cursor_i + state.offset_i - state.ruler_pane_height - 1;
        SeqRecord record = state.record_array.records[record_index];
        unsigned int max_j = record.len - state.offset_j + state.header_pane_width;
        if (state.cursor_j < state.terminal_cols && state.cursor_j < max_j)
        {
            state.cursor_j++;
        }
        break;
    }
    case 'q':
        exit(0);
        break;
    }
    display_cursor(buffer);
    terminal_cursor_show(buffer);
    return 0;
}

void input_buffer_flush(Array *buffer)
{
    write(STDOUT_FILENO, buffer->data, buffer->len);
    buffer->len = 0;
}
