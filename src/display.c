#include <string.h>

#include "array.h"
#include "state.h"
#include "terminal.h"

extern State state;

void display_header_pane(Array *buffer)
{
    for (unsigned int i = state.ruler_pane_height + 1; i <= state.terminal_rows; i++)
    {
        terminal_cursor_ij(buffer, i, 1);
        unsigned int record_index = i - state.ruler_pane_height - 1;
        if (record_index < state.record_array.len)
        {
            SeqRecord record = state.record_array.records[record_index];
            unsigned int len = strnlen(record.header, state.header_pane_width - 1);
            if (len < state.header_pane_width - 1)
            {
                array_extend(buffer, record.header, len);
                for (unsigned int j = len + 1; j <= state.header_pane_width - 1; j++)
                    array_append(buffer, " ");
            }
            else
            {
                array_extend(buffer, record.header, state.header_pane_width - 4);
                array_extend(buffer, "...", 3);
            }
            array_extend(buffer, "┃", sizeof("┃") - 1);
        }
        else
        {
            array_extend(buffer, "~", sizeof("~") - 1);
            for (unsigned int j = 2; j <= state.header_pane_width - 1; j++)
                array_append(buffer, " ");
            array_extend(buffer, "┃", sizeof("┃") - 1);
        }
    }
}

void display_ruler_pane(Array *buffer)
{
    terminal_cursor_ij(buffer, 1, state.header_pane_width);
    for (unsigned int i = 1; i < state.ruler_pane_height; i++)
    {
        char s[] = "│\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }
    terminal_cursor_ij(buffer, state.ruler_pane_height, 1);
    for (unsigned int j = 1; j < state.header_pane_width; j++)
        array_extend(buffer, "─", sizeof("─") - 1);
    array_extend(buffer, "╆", sizeof("╆") - 1);
    for (unsigned int j = state.header_pane_width + 1; j <= state.terminal_cols; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
}

void display_sequence_pane(Array *buffer)
{
    for (unsigned int i = state.ruler_pane_height + 1; i <= state.terminal_rows; i++)
    {
        unsigned int record_index = i - state.ruler_pane_height - 1;
        if (record_index < state.record_array.len)
        {
            terminal_cursor_ij(buffer, i, state.header_pane_width + 1);
            SeqRecord record = state.record_array.records[record_index];
            array_extend(buffer, record.seq, record.len);
        }
        else
            break;
    }
}
