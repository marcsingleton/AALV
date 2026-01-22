#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "color.h"
#include "display.h"
#include "pane.h"
#include "state.h"
#include "terminal.h"

extern State state;

void display_refresh(Array *buffer)
{
    terminal_cursor_hide(buffer);
    state.refresh_command_pane = true;

    unsigned int rows, cols;
    terminal_get_window_size(&rows, &cols);
    if (state.terminal_rows != rows || state.terminal_cols != cols)
    {
        state.terminal_rows = rows;
        state.terminal_cols = cols;
        state.refresh_window = true;
    }
    if (state.refresh_window)
    {
        terminal_clear_screen(buffer);
        state_set_divider_limits(&state);
        state_set_records_command_divider_i(&state, state.terminal_rows - 2);
        state_set_header_sequence_divider_j(&state, state.active_file->layout.header_sequence_divider_j); // Triggers automatic re-sizes
        state_set_ruler_records_divider_i(&state, state.active_file->layout.ruler_records_divider_i);
        state.active_file->layout.command_pane.w = state.terminal_cols;
        state.active_file->layout.ruler_pane.w = state.terminal_cols;

        state.refresh_window = false;
    }
    if (state.refresh_ruler_pane)
    {
        display_ruler_pane(buffer);
        display_ruler_pane_ticks(buffer);
        state.refresh_ruler_pane = false;
    }
    if (state.refresh_header_pane)
    {
        display_header_pane(buffer);
        state.refresh_header_pane = false;
    }
    if (state.refresh_sequence_pane)
    {
        display_sequence_pane(buffer);
        state.refresh_sequence_pane = false;
    }
    if (state.refresh_command_pane)
    {
        display_command_pane(buffer);
        state.refresh_command_pane = false;
    }
    display_cursor(buffer);
}

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
    Pane *pane = &active_file->layout.header_pane;

    unsigned int ellipses_width = wcswidth(DISPLAY_HEADER_PANE_ELLIPSES, sizeof(DISPLAY_HEADER_PANE_ELLIPSES));
    for (unsigned int i = 0; i < pane->h; i++)
    {
        // Record
        size_t record_index = i + active_file->offset_record;
        pane_cursor_ij(pane, buffer, i, 0);
        if (record_index < active_file->record_array.len)
        {
            // Has record
            SeqRecord record = active_file->record_array.data[record_index];
            size_t len = strnlen(record.header, pane->w);
            if (len < pane->w)
            {
                array_extend(buffer, record.header, len);
                for (unsigned int j = len; j < pane->w - 1; j++)
                    array_append(buffer, " ");
            }
            else
            {
                array_extend(buffer, record.header, pane->w - ellipses_width - 1);
                array_extend(buffer, DISPLAY_HEADER_PANE_ELLIPSES, sizeof(DISPLAY_HEADER_PANE_ELLIPSES) - 1);
            }
        }
        else
        {
            // No record
            array_extend(buffer, "~", sizeof("~") - 1);
            for (unsigned int j = 1; j < pane->w - 1; j++)
                array_append(buffer, " ");
        }

        // Header-sequencer divider
        char s[] = "┃\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }
}

void display_ruler_pane(Array *buffer)
{
    FileState *active_file = state.active_file;
    Pane *pane = &active_file->layout.ruler_pane;
    unsigned int header_sequence_divider_j = active_file->layout.header_sequence_divider_j;

    // Header-sequence divider continuation
    pane_cursor_ij(pane, buffer, 0, header_sequence_divider_j);
    for (unsigned int i = 0; i < pane->h - 1; i++)
    {
        terminal_clear_line(buffer);
        char s[] = "┃\n\b";
        array_extend(buffer, s, sizeof(s) - 1);
    }

    // Lower pane boundary
    pane_cursor_ij(pane, buffer, pane->h - 1, 0);
    for (unsigned int j = 0; j < header_sequence_divider_j; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
    if (state.active_file->layout.ruler_records_divider_i < state.active_file->layout.records_command_divider_i)
        array_extend(buffer, "╋", sizeof("╋") - 1);
    else
        array_extend(buffer, "┻", sizeof("┻") - 1);
    for (unsigned int j = header_sequence_divider_j + 1; j < pane->w; j++)
        array_extend(buffer, "━", sizeof("━") - 1);
}

void display_ruler_pane_ticks(Array *buffer)
{
    FileState *active_file = state.active_file;
    Pane *pane = &active_file->layout.ruler_pane;
    unsigned int header_sequence_divider_j = active_file->layout.header_sequence_divider_j;

    unsigned int tick_spacing = active_file->tick_spacing;
    size_t x0 = active_file->offset_sequence + active_file->records_offset;
    size_t q = x0 / tick_spacing;
    size_t r = x0 % tick_spacing;
    if (r > 0)
        q++;

    size_t x = q * tick_spacing;
    size_t j = pane->j + header_sequence_divider_j + 1;
    j += x - active_file->offset_sequence - active_file->records_offset;
    unsigned int ellipses_width = wcswidth(DISPLAY_RULER_PANE_ELLIPSES, sizeof(DISPLAY_RULER_PANE_ELLIPSES));
    while (j < pane->w)
    {
        pane_cursor_ij(pane, buffer, pane->h - 1, j);
        array_extend(buffer, "┷", sizeof("┷") - 1);

        char c[2];
        size_t n = x;
        unsigned int d;
        unsigned int i = pane->i + pane->h - 2;
        do
        {
            // Write numbers from tick to top
            pane_cursor_ij(pane, buffer, i--, j);
            d = n % 10;
            n = n / 10;
            snprintf(c, 2, "%d", d);
            array_append(buffer, c); // Excludes null in c
            if (i == 0 && n != 0)
            {
                for (i = 0; i < ellipses_width; i++)
                {
                    pane_cursor_ij(pane, buffer, i, j);
                    array_extend(buffer, "·", sizeof("·") - 1);
                }
                break;
            }
        } while (n != 0);

        x += tick_spacing;
        j += tick_spacing;
    }
}

void display_sequence_pane(Array *buffer)
{
    FileState *active_file = state.active_file;
    Pane *pane = &active_file->layout.sequence_pane;

    for (unsigned int i = 0; i < pane->h; i++)
    {
        size_t record_index = i + active_file->offset_record;

        pane_cursor_ij(pane, buffer, i, 0);
        if (record_index < active_file->record_array.len)
        {
            SeqRecord record = active_file->record_array.data[record_index];
            unsigned int left_continuation = 0;
            unsigned int right_continuation = 0;
            size_t start = active_file->offset_sequence;
            unsigned int len;
            if (active_file->offset_sequence > 0)
            {
                left_continuation = 1;
                start++;
            }
            if (record.len <= active_file->offset_sequence)
                len = 0;
            else if ((record.len > active_file->offset_sequence + pane->w))
            {
                right_continuation = 1;
                len = pane->w - left_continuation - right_continuation; // Difference should always fit into int
            }
            else
                len = record.len - active_file->offset_sequence - left_continuation;

            if (left_continuation)
                array_append(buffer, "<");
            if (len > 0)
                display_sequence(buffer, &record, start, len);
            if (right_continuation)
                array_append(buffer, ">");
            else
                for (unsigned int j = left_continuation + len; j < pane->w; j++)
                    array_append(buffer, " ");
        }
        else
            for (unsigned int j = 0; j < pane->w; j++)
                array_append(buffer, " ");
    }
}

void display_command_pane(Array *buffer)
{
    FileState *active_file = state.active_file;
    Pane *pane = &active_file->layout.command_pane;
    unsigned int header_sequence_divider_j = active_file->layout.header_sequence_divider_j;

    // Records-command divider
    if (state.active_file->layout.ruler_records_divider_i < state.active_file->layout.records_command_divider_i)
    {
        pane_cursor_ij(pane, buffer, 0, 0);
        for (unsigned int j = 0; j < header_sequence_divider_j; j++)
            array_extend(buffer, "━", sizeof("━") - 1);
        array_extend(buffer, "┻", sizeof("┻") - 1);
        for (unsigned int j = header_sequence_divider_j + 1; j < pane->w; j++)
            array_extend(buffer, "━", sizeof("━") - 1);
    }

    if (pane->h < 2)
        return;

    // Status line
    switch (state.mode)
    {
    case NORMAL:
    {
        char status[256];
        unsigned int n_status = 0;
        size_t record_index = active_file->offset_record + active_file->cursor_record_i;
        size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
        UnalignedIndices *unaligned_indices = active_file->indices_array.data + record_index;

        int n = 0;

        if (unaligned_indices->indices && unaligned_indices->len > 0)
        {
            size_t unaligned_index = (sequence_index + 1 > unaligned_indices->len) ? unaligned_indices->len - 1
                                                                                   : sequence_index;
            n = snprintf(status + n_status, sizeof(status) - n_status,
                         "POS %zu/%zu  ",
                         unaligned_indices->indices[unaligned_index],
                         unaligned_indices->indices[unaligned_indices->len - 1]);
        }
        else
        {
            n = snprintf(status + n_status, sizeof(status) - n_status,
                         "POS ?/?  ");
        }
        if (n < 0)
            return;
        n_status += n;

        n = snprintf(status + n_status, sizeof(status) - n_status,
                     "ROW %zu/%zu  ",
                     record_index + 1, // 1-based indexing
                     active_file->record_array.len);
        if (n < 0)
            return;
        n_status += n;

        n = snprintf(status + n_status, sizeof(status) - n_status,
                     "COL %zu/%zu",
                     sequence_index + active_file->records_offset,
                     active_file->records_maxlen);
        if (n < 0)
            return;
        n_status += n;

        unsigned int n_file_name = strnlen(active_file->file_path, 256);

        pane_cursor_ij(pane, buffer, 1, 0);
        if (n_file_name + n_status + 4 <= pane->w)
        {
            array_extend(buffer, active_file->file_path, n_file_name);
            for (unsigned int i = n_file_name; i + n_status < pane->w; i++)
                array_append(buffer, " ");
            array_extend(buffer, status, n_status);
        }
        else if (n_status <= pane->w)
        {
            for (unsigned int i = 0; i + n_status < pane->w; i++)
                array_append(buffer, " ");
            array_extend(buffer, status, n_status);
        }
        break;
    }
    case COMMAND:
        break;
    }
}

void display_cursor(Array *buffer)
{
    FileState *active_file = state.active_file;
    Pane *pane = &active_file->layout.sequence_pane;

    if (pane->h == 0)
        return;
    if (active_file->record_array.len == 0)
        return;

    switch (state.mode)
    {
    case NORMAL:
    {
        // Row
        unsigned cursor_i;
        if (active_file->cursor_record_i >= pane->h)
            cursor_i = pane->h - 1;
        else
            cursor_i = active_file->cursor_record_i;

        // Column
        size_t record_index = cursor_i + active_file->offset_record;
        size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
        SeqRecord record = active_file->record_array.data[record_index];
        size_t display_index;
        if (record.len == 0)
            display_index = 0;
        else if (sequence_index >= record.len)
            display_index = record.len - 1;
        else
            display_index = sequence_index;
        unsigned int cursor_j;
        if (display_index > active_file->offset_sequence)
            cursor_j = display_index - active_file->offset_sequence;
        else
            cursor_j = 0;

        pane_cursor_ij(pane, buffer, cursor_i, cursor_j);
        terminal_cursor_show(buffer);
        break;
    }
    case COMMAND:
    {
        pane = &active_file->layout.command_pane;
        pane_cursor_ij(pane, buffer, 0, 0);
        terminal_cursor_show(buffer);
    }
    }
}

void display_sequence(Array *buffer, SeqRecord *record, size_t start, size_t len)
{
    SeqTypeState *seq_type = state.seq_types + record->type;
    const Alphabet *alphabet = seq_type->alphabet;
    ColorScheme *color_scheme = seq_type->color_scheme;
    if (state.ncolors > 1 && color_scheme)
    {
        if (color_scheme->type == COLOR_4_BIT)
        {
            for (size_t i = start; i < start + len; i++)
            {
                char sym = record->seq[i];
                int index = alphabet->index_map[(unsigned int)sym]; // Skip negativity check b/c already checked type
                if (color_scheme->mask.fg[index] && color_scheme->mask.bg[index])
                {
                    Color4Bit fg_color = color_scheme->map.b4.fg[index];
                    Color4Bit bg_color = color_scheme->map.b4.bg[index];
                    terminal_set_color_4bit(buffer, fg_color, bg_color);
                }
                else if (color_scheme->mask.fg[index])
                {
                    Color4Bit fg_color = color_scheme->map.b4.fg[index];
                    terminal_set_foreground_color_4bit(buffer, fg_color);
                }
                else if (color_scheme->mask.bg[index])
                {
                    Color4Bit bg_color = color_scheme->map.b4.bg[index];
                    terminal_set_background_color_4bit(buffer, bg_color);
                }
                else
                    terminal_set_color_default(buffer);
                array_append(buffer, &sym);
            }
            terminal_set_color_default(buffer);
        }
        else if (color_scheme->type == COLOR_8_BIT)
        {
            for (size_t i = start; i < start + len; i++)
            {
                char sym = record->seq[i];
                int index = alphabet->index_map[(unsigned int)sym]; // Skip negativity check b/c already checked type
                if (color_scheme->mask.fg[index] && color_scheme->mask.bg[index])
                {
                    Color8Bit fg_color = color_scheme->map.b8.fg[index];
                    Color8Bit bg_color = color_scheme->map.b8.bg[index];
                    terminal_set_color_8bit(buffer, fg_color, bg_color);
                }
                else if (color_scheme->mask.fg[index])
                {
                    Color8Bit fg_color = color_scheme->map.b8.fg[index];
                    terminal_set_foreground_color_8bit(buffer, fg_color);
                }
                else if (color_scheme->mask.bg[index])
                {
                    Color8Bit bg_color = color_scheme->map.b8.bg[index];
                    terminal_set_background_color_8bit(buffer, bg_color);
                }
                else
                    terminal_set_color_default(buffer);
                array_append(buffer, &sym);
            }
            terminal_set_color_default(buffer);
        }
    }
    else
        array_extend(buffer, record->seq + start, len);
}
