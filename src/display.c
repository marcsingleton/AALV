#include <stdio.h>
#include <string.h>

#include "array.h"
#include "display.h"
#include "state.h"
#include "terminal.h"

extern State state;

void display_all_panes(Array *buffer)
{
    display_ruler_pane(buffer);
    display_ruler_pane_ticks(buffer);
    display_header_pane(buffer);
    display_sequence_pane(buffer);
}

void display_header_pane(Array *buffer)
{
    for (unsigned int i = state.ruler_pane_height + 1; i <= state.terminal_rows - 2; i++)
    {
        unsigned int record_index = i + state.offset_record - state.ruler_pane_height - 1;

        terminal_cursor_ij(buffer, i, 1);
        if (record_index < state.record_array.len)
        {
            SeqRecord record = state.record_array.records[record_index];
            unsigned int len = strnlen(record.header, state.header_pane_width);
            if (len <= state.header_pane_width - 1)
            {
                array_extend(buffer, record.header, len);
                for (unsigned int j = len + 1; j <= state.header_pane_width - 1; j++)
                    array_append(buffer, " ");
            }
            else
            {
                array_extend(buffer, record.header, state.header_pane_width - 4);
                array_extend(buffer, HEADER_PANE_ELLIPSES, sizeof(HEADER_PANE_ELLIPSES) - 1);
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

    terminal_cursor_ij(buffer, state.terminal_rows - 1, 1);
    for (unsigned int j = 1; j <= state.header_pane_width - 1; j++)
    {
        array_extend(buffer, "─", sizeof("─") - 1);
    }
    array_extend(buffer, "┺", sizeof("┺") - 1);
}

void display_ruler_pane(Array *buffer)
{
    terminal_cursor_ij(buffer, 1, state.header_pane_width);
    for (unsigned int i = 1; i <= state.ruler_pane_height - 1; i++)
    {
        terminal_clear_line(buffer);
        char s[] = "│\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }

    terminal_cursor_ij(buffer, state.ruler_pane_height, 1);
    for (unsigned int j = 1; j <= state.header_pane_width - 1; j++)
        array_extend(buffer, "─", sizeof("─") - 1);
    array_extend(buffer, "╆", sizeof("╆") - 1);
    for (unsigned int j = state.header_pane_width + 1; j <= state.terminal_cols; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
}

void display_ruler_pane_ticks(Array *buffer)
{
    unsigned int tick_spacing = 10;
    unsigned int q = state.offset_sequence / tick_spacing;
    unsigned int r = state.offset_sequence % tick_spacing;
    if (r >= 0)
        q++;

    unsigned int x = q * tick_spacing;
    unsigned int j;
    while ((j = x - state.offset_sequence + state.header_pane_width) <= state.terminal_cols)
    {
        terminal_cursor_ij(buffer, state.ruler_pane_height, j);
        array_extend(buffer, "┷", sizeof("┷") - 1);

        char c[2];
        unsigned int n = x;
        unsigned int d;
        unsigned int i = state.ruler_pane_height - 1;
        do
        {
            d = n % 10;
            n = n / 10;
            snprintf(c, 2, "%d", d);
            terminal_cursor_ij(buffer, i--, j);
            array_append(buffer, c);
        } while (n != 0);

        x += tick_spacing;
    }
}

void display_sequence_pane(Array *buffer)
{
    for (unsigned int i = state.ruler_pane_height + 1; i <= state.terminal_rows - 2; i++)
    {
        unsigned int record_index = i + state.offset_record - state.ruler_pane_height - 1;

        terminal_cursor_ij(buffer, i, state.header_pane_width + 1);
        terminal_clear_line_right(buffer);
        if (record_index < state.record_array.len)
        {
            SeqRecord record = state.record_array.records[record_index];
            int cols = state.terminal_cols - state.header_pane_width;
            int len = record.len - state.offset_sequence;
            if (state.offset_sequence > 0)
                array_append(buffer, "<");
            else
                array_append(buffer, record.seq + state.offset_sequence);
            if (len > cols)
            {
                array_extend(buffer, record.seq + state.offset_sequence + 1, cols - 2);
                array_append(buffer, ">");
            }
            else
                array_extend(buffer, record.seq + state.offset_sequence + 1, len - 1);
        }

        terminal_cursor_ij(buffer, state.terminal_rows - 1, state.header_pane_width + 1);
        for (unsigned int j = state.header_pane_width + 1; j <= state.terminal_cols; j++)
            array_extend(buffer, "━", sizeof("━") - 1);
    }
}

void display_cursor(Array *buffer)
{
    unsigned int record_index = state.cursor_record_i + state.offset_record;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int sequence_position = state.cursor_sequence_j + state.offset_sequence;
    unsigned int display_position = (record.len > sequence_position) ? sequence_position : record.len;
    unsigned int cursor_i = state.cursor_record_i + state.ruler_pane_height + 1;
    if (display_position > state.offset_sequence)
        terminal_cursor_ij(buffer, cursor_i, display_position - state.offset_sequence + state.header_pane_width + 1);
    else
        terminal_cursor_ij(buffer, cursor_i, state.header_pane_width + 1);
}
