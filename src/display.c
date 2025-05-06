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
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    terminal_cursor_ij(buffer, state.ruler_pane_height + 1, state.header_pane_width);
    for (unsigned int i = 0; i < record_panes_height; i++)
    {
        terminal_clear_line_left(buffer);
        char s[] = "┃\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }

    for (unsigned int i = 0; i < record_panes_height; i++)
    {
        unsigned int record_index = i + state.offset_record;
        terminal_cursor_ij(buffer, i + state.ruler_pane_height + 1, 1);
        if (record_index < state.record_array.len)
        {
            SeqRecord record = state.record_array.records[record_index];
            unsigned int len = strnlen(record.header, state.header_pane_width);
            if (len < state.header_pane_width)
                array_extend(buffer, record.header, len);
            else
            {
                array_extend(buffer, record.header, state.header_pane_width - 4);
                array_extend(buffer, HEADER_PANE_ELLIPSES, sizeof(HEADER_PANE_ELLIPSES) - 1);
            }
        }
        else
            array_extend(buffer, "~", sizeof("~") - 1);
    }
}

void display_ruler_pane(Array *buffer)
{
    terminal_cursor_ij(buffer, 1, state.header_pane_width);
    for (unsigned int i = 0; i < state.ruler_pane_height; i++)
    {
        terminal_clear_line(buffer);
        char s[] = "┃\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    terminal_cursor_ij(buffer, state.ruler_pane_height, 1);
    for (unsigned int j = 0; j < state.header_pane_width - 1; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
    if (state.terminal_rows <= state.ruler_pane_height)
        array_extend(buffer, "┻", sizeof("┻") - 1);
    else
        array_extend(buffer, "╋", sizeof("╋") - 1);
    for (unsigned int j = 0; j < sequence_pane_width; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
}

void display_ruler_pane_ticks(Array *buffer)
{
    unsigned int tick_spacing = state.tick_spacing;
    unsigned int x0 = state.offset_sequence + state.record_array.offset;
    unsigned int q = x0 / tick_spacing;
    unsigned int r = x0 % tick_spacing;
    if (r > 0)
        q++;

    unsigned int x = q * tick_spacing;
    unsigned int j;
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    while ((j = x - state.offset_sequence - state.record_array.offset) < sequence_pane_width)
    {
        terminal_cursor_ij(buffer, state.ruler_pane_height, j + state.header_pane_width + 1);
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
            terminal_cursor_ij(buffer, i--, j + state.header_pane_width + 1);
            array_append(buffer, c);
        } while (n != 0);

        x += tick_spacing;
    }
}

void display_sequence_pane(Array *buffer)
{
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    for (unsigned int i = 0; i < record_panes_height; i++)
    {
        unsigned int record_index = i + state.offset_record;

        terminal_cursor_ij(buffer, i + state.ruler_pane_height + 1, state.header_pane_width + 1);
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
    }
}

void display_command_pane(Array *buffer)
{
    if (state.terminal_rows <= state.ruler_pane_height)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    terminal_cursor_ij(buffer, state.ruler_pane_height + record_panes_height + 1, 1);
    for (unsigned int j = 0; j < state.header_pane_width - 1; j++)
    {
        array_extend(buffer, "━", sizeof("━") - 1);
    }
    array_extend(buffer, "┻", sizeof("┻") - 1);
    for (unsigned int j = 0; j < sequence_pane_width; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
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
