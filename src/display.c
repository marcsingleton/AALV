#include <stdio.h>
#include <string.h>
#include <wchar.h>

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
    FileState *active_file = state.active_file;
    unsigned int record_panes_height = state_get_record_panes_height(&state);

    terminal_cursor_ij(buffer, active_file->ruler_pane_height + 1, active_file->header_pane_width);
    for (unsigned int i = 0; i < record_panes_height; i++)
    {
        terminal_clear_line_left(buffer);
        char s[] = "┃\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }

    for (unsigned int i = 0; i < record_panes_height; i++)
    {
        unsigned int record_index = i + active_file->offset_record;
        terminal_cursor_ij(buffer, i + active_file->ruler_pane_height + 1, 1);
        if (record_index < active_file->record_array.len)
        {
            SeqRecord record = active_file->record_array.records[record_index];
            unsigned int len = strnlen(record.header, active_file->header_pane_width);
            if (len < active_file->header_pane_width)
                array_extend(buffer, record.header, len);
            else
            {
                array_extend(buffer, record.header, active_file->header_pane_width - 4);
                array_extend(buffer, HEADER_PANE_ELLIPSES, sizeof(HEADER_PANE_ELLIPSES) - 1);
            }
        }
        else
            array_extend(buffer, "~", sizeof("~") - 1);
    }
}

void display_ruler_pane(Array *buffer)
{
    FileState *active_file = state.active_file;
    terminal_cursor_ij(buffer, 1, active_file->header_pane_width);
    for (unsigned int i = 0; i < active_file->ruler_pane_height; i++)
    {
        terminal_clear_line(buffer);
        char s[] = "┃\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    terminal_cursor_ij(buffer, active_file->ruler_pane_height, 1);
    for (unsigned int j = 0; j < active_file->header_pane_width - 1; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
    if (state.terminal_rows <= active_file->ruler_pane_height)
        array_extend(buffer, "┻", sizeof("┻") - 1);
    else
        array_extend(buffer, "╋", sizeof("╋") - 1);
    for (unsigned int j = 0; j < sequence_pane_width; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
}

void display_ruler_pane_ticks(Array *buffer)
{
    FileState *active_file = state.active_file;
    unsigned int tick_spacing = active_file->tick_spacing;
    unsigned int x0 = active_file->offset_sequence + active_file->record_array.offset;
    unsigned int q = x0 / tick_spacing;
    unsigned int r = x0 % tick_spacing;
    if (r > 0)
        q++;

    unsigned int x = q * tick_spacing;
    unsigned int j;
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    while ((j = x - active_file->offset_sequence - active_file->record_array.offset) < sequence_pane_width)
    {
        terminal_cursor_ij(buffer, active_file->ruler_pane_height, j + active_file->header_pane_width + 1);
        array_extend(buffer, "┷", sizeof("┷") - 1);

        char c[2];
        unsigned int n = x;
        unsigned int d;
        unsigned int i = active_file->ruler_pane_height - 1;
        do
        {
            terminal_cursor_ij(buffer, i--, j + active_file->header_pane_width + 1);
            d = n % 10;
            n = n / 10;
            snprintf(c, 2, "%d", d);
            array_append(buffer, c); // Excludes null in c
            if (i == 0 && n != 0)
            {
                unsigned int width = wcswidth(RULER_PANE_ELLIPSES, sizeof(RULER_PANE_ELLIPSES));
                for (i = 0; i < width; i++)
                {
                    terminal_cursor_ij(buffer, i + 1, j + active_file->header_pane_width + 1);
                    array_extend(buffer, "·", sizeof("·"));
                }
                break;
            }
        } while (n != 0);

        x += tick_spacing;
    }
}

void display_sequence_pane(Array *buffer)
{
    FileState *active_file = state.active_file;
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    for (unsigned int i = 0; i < record_panes_height; i++)
    {
        unsigned int record_index = i + active_file->offset_record;

        terminal_cursor_ij(buffer, i + active_file->ruler_pane_height + 1, active_file->header_pane_width + 1);
        terminal_clear_line_right(buffer);
        if (record_index < active_file->record_array.len)
        {
            SeqRecord record = active_file->record_array.records[record_index];
            if (active_file->offset_sequence > 0)
                array_append(buffer, "<");
            else
                array_append(buffer, record.seq + active_file->offset_sequence);
            if (record.len > active_file->offset_sequence + sequence_pane_width)
            {
                array_extend(buffer, record.seq + active_file->offset_sequence + 1, sequence_pane_width - 2);
                array_append(buffer, ">");
            }
            else if (record.len > active_file->offset_sequence)
                array_extend(buffer,
                             record.seq + active_file->offset_sequence + 1,
                             record.len - active_file->offset_sequence - 1);
        }
    }
}

void display_command_pane(Array *buffer)
{
    FileState *active_file = state.active_file;
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    if (state.terminal_rows <= active_file->ruler_pane_height)
        return;
    terminal_cursor_ij(buffer, active_file->ruler_pane_height + record_panes_height + 1, 1);
    for (unsigned int j = 0; j < active_file->header_pane_width - 1; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
    array_extend(buffer, "┻", sizeof("┻") - 1);
    for (unsigned int j = 0; j < sequence_pane_width; j++)
        array_extend(buffer, "━", sizeof("━") - 1);

    if (state.terminal_rows <= active_file->ruler_pane_height + 1)
        return;
    char text[256];
    int n = snprintf(text, sizeof(text),
                     "ROW %d COL %d",
                     active_file->offset_record + active_file->cursor_record_i,
                     active_file->offset_sequence + active_file->cursor_sequence_j + active_file->record_array.offset);
    terminal_cursor_ij(buffer, active_file->ruler_pane_height + record_panes_height + 2, state.terminal_cols - n + 1);
    terminal_clear_line(buffer);
    array_extend(buffer, text, n);
}

void display_cursor(Array *buffer)
{
    FileState *active_file = state.active_file;
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        return;

    unsigned render_index_i = (active_file->cursor_record_i >= record_panes_height) ? record_panes_height - 1
                                                                                    : active_file->cursor_record_i;
    unsigned int cursor_i = render_index_i + active_file->ruler_pane_height + 1;

    unsigned int record_index = render_index_i + active_file->offset_record;
    unsigned int sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
    SeqRecord record = active_file->record_array.records[record_index];
    unsigned int render_index_j = (record.len > sequence_index) ? sequence_index : record.len - 1;
    unsigned int cursor_j;
    if (render_index_j > active_file->offset_sequence)
        cursor_j = render_index_j - active_file->offset_sequence + active_file->header_pane_width + 1;
    else
        cursor_j = active_file->header_pane_width + 1;

    terminal_cursor_ij(buffer, cursor_i, cursor_j);
    terminal_cursor_show(buffer);
}
