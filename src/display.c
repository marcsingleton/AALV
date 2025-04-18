#include <string.h>

#include "array.h"
#include "state.h"
#include "terminal.h"

extern State state;

void display_header_pane(Array *buffer)
{
    terminal_cursor_ij(buffer, state.ruler_pane_height + 1, 1);
    for (int i = 0; i < state.record_array.len; i++)
    {
        SeqRecord record = state.record_array.records[i];
        unsigned int len = strnlen(record.header, state.header_pane_width - 1);
        if (len < state.header_pane_width - 1)
        {
            array_extend(buffer, record.header, len);
            for (unsigned int i = len; i < state.header_pane_width - 1; i++)
                array_append(buffer, " ");
        }
        else
        {
            array_extend(buffer, record.header, state.header_pane_width - 4);
            array_extend(buffer, "...", 3);
        }
        char s[] = "┃\r\n";
        array_extend(buffer, s, sizeof(s) - 1);
    }
}

void display_ruler_pane(Array *buffer)
{
    terminal_cursor_ij(buffer, 1, state.header_pane_width);
    for (unsigned int i = 0; i < state.ruler_pane_height - 1; i++)
    {
        char s[] = "│\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }
    terminal_cursor_ij(buffer, state.ruler_pane_height, 1);
    for (unsigned int j = 0; j < state.header_pane_width - 1; j++)
        array_extend(buffer, "─", sizeof("─") - 1);
    array_extend(buffer, "╆", sizeof("╆") - 1);
    for (unsigned int j = state.header_pane_width + 1; j < state.terminal_cols; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
}

void display_sequence_pane(Array *buffer)
{
    for (int i = 0; i < state.record_array.len; i++)
    {
        SeqRecord record = state.record_array.records[i];
        terminal_cursor_ij(buffer, state.ruler_pane_height + 1 + i, state.header_pane_width + 1);
        array_extend(buffer, record.seq, record.len);
    }
}
