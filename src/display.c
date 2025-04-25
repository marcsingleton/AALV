#include <stdio.h>
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
        unsigned int record_index = i + state.offset_y - state.ruler_pane_height - 1;
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
        terminal_clear_line(buffer);
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

void display_ruler_pane_ticks(Array *buffer)
{
    unsigned int tick_spacing = 10;
    unsigned int q = state.offset_x / tick_spacing;
    unsigned int r = state.offset_x % tick_spacing;
    if (r >= 0)
        q++;

    unsigned int x = q * tick_spacing;
    unsigned int j;
    while ((j = x - state.offset_x + state.header_pane_width) <= state.terminal_cols)
    {
        terminal_cursor_ij(buffer, state.ruler_pane_height, j);
        array_extend(buffer, "┷", sizeof("┷") - 1);

        char c[2];
        unsigned int n = x;
        unsigned int d;
        do
        {
            d = n % 10;
            n = n / 10;
            snprintf(c, 2, "%d", d);
            terminal_cursor_up(buffer);
            terminal_cursor_left(buffer);
            array_append(buffer, c);
        } while (n != 0);

        x += tick_spacing;
    }
}

void display_sequence_pane(Array *buffer)
{
    for (unsigned int i = state.ruler_pane_height + 1; i <= state.terminal_rows; i++)
    {
        unsigned int record_index = i + state.offset_y - state.ruler_pane_height - 1;
        terminal_cursor_ij(buffer, i, state.header_pane_width + 1);
        terminal_clear_line_right(buffer);
        if (record_index < state.record_array.len)
        {
            SeqRecord record = state.record_array.records[record_index];
            int cols = state.terminal_cols - state.header_pane_width;
            int len = record.len - state.offset_x;
            len = cols > len ? len : cols;
            array_extend(buffer, record.seq + state.offset_x, len);
        }
    }
}

void display_cursor(Array *buffer)
{
    unsigned int record_index = state.cursor_i + state.offset_y - state.ruler_pane_height - 1;
    SeqRecord record = state.record_array.records[record_index];
    unsigned int max_j = record.len - state.offset_j + state.header_pane_width;
    unsigned int j = (max_j > state.cursor_j) ? state.cursor_j : max_j;
    terminal_cursor_ij(buffer, state.cursor_i, j);
}
