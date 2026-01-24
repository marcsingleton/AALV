#include "scroller.h"

int scroller_init(RowLinkedScroller *scroller, unsigned int npanes)
{
    if (!scroller || npanes == 0)
        return 1;

    size_t *offsets_j = NULL;
    size_t *cursors_j = NULL;
    unsigned int *ws = NULL;
    bool *refreshes = NULL;

    offsets_j = calloc(npanes, sizeof(size_t));
    cursors_j = calloc(npanes, sizeof(size_t));
    ws = calloc(npanes, sizeof(unsigned int));
    refreshes = calloc(npanes, sizeof(bool));
    if (!offsets_j || !cursors_j || !ws || !refreshes)
    {
        free(offsets_j);
        free(cursors_j);
        free(ws);
        free(refreshes);
        return 1;
    }

    scroller->offset_i = 0;
    scroller->cursor_i = 0;
    scroller->offsets_j = offsets_j;
    scroller->cursors_j = cursors_j;
    scroller->npanes = npanes;
    scroller->active_pane_index = 0;
    scroller->h = 0;
    scroller->ws = ws;
    scroller->refreshes = refreshes;

    return 0;
}

void scroller_deinit(RowLinkedScroller *scroller)
{
    if (!scroller)
        return;

    free(scroller->cursors_j);
    free(scroller->offsets_j);
    free(scroller->ws);
    free(scroller->refreshes);

    scroller->offset_i = 0;
    scroller->cursor_i = 0;
    scroller->offsets_j = NULL;
    scroller->cursors_j = NULL;
    scroller->npanes = 0;
    scroller->active_pane_index = 0;
    scroller->h = 0;
    scroller->ws = NULL;
    scroller->refreshes = NULL;
}

void scroller_set_offset_i(RowLinkedScroller *scroller, size_t i)
{
    if (!scroller)
        return;
    scroller->offset_i = i;
    scroller_set_refreshes(scroller, true);
}

void scroller_set_cursor_i(RowLinkedScroller *scroller, size_t i)
{
    if (!scroller)
        return;
    scroller->cursor_i = i;
}

void scroller_set_offset_j(RowLinkedScroller *scroller, size_t j)
{
    if (!scroller)
        return;
    unsigned int index = scroller->active_pane_index;
    scroller->offsets_j[index] = j;
    scroller->refreshes[index] = true;
}

void scroller_set_offsets_j(RowLinkedScroller *scroller, size_t j)
{
    if (!scroller)
        return;
    for (unsigned int index = 0; index < scroller->npanes; index++)
    {
        scroller->offsets_j[index] = j;
        scroller->refreshes[index] = true;
    }
}

void scroller_set_cursor_j(RowLinkedScroller *scroller, size_t j)
{
    if (!scroller)
        return;
    unsigned int index = scroller->active_pane_index;
    scroller->cursors_j[index] = j;
}

void scroller_set_cursors_j(RowLinkedScroller *scroller, size_t j)
{
    if (!scroller)
        return;
    for (unsigned int index = 0; index < scroller->npanes; index++)
        scroller->cursors_j[index] = j;
}

void scroller_set_active_pane(RowLinkedScroller *scroller, unsigned int index)
{
    if (!scroller)
        return;
    if (index + 1 > scroller->npanes)
        return;
    scroller->active_pane_index = index;
}

void scroller_set_refresh(RowLinkedScroller *scroller, bool refresh)
{
    if (!scroller)
        return;
    unsigned int index = scroller->active_pane_index;
    scroller->refreshes[index] = refresh;
}

void scroller_set_refreshes(RowLinkedScroller *scroller, bool refresh)
{
    if (!scroller)
        return;
    for (unsigned int index = 0; index < scroller->npanes; index++)
        scroller->refreshes[index] = refresh;
}

void scroller_cursor_clamp(RowLinkedScroller *scroller)
{
    if (!scroller)
        return;
    unsigned int pane_height = scroller->h;
    if (pane_height == 0)
        pane_height = 1; // Treat collapsed pane as single row

    unsigned int active_index = scroller->active_pane_index;
    unsigned int pane_width = scroller->ws[active_index];
    if (pane_width == 0)
        pane_width = 1; // Treat collapsed pane as single column

    size_t cursor_i = scroller->cursor_i;
    size_t cursor_j = scroller->cursors_j[active_index];
    if (cursor_i + 1 > pane_height)
        scroller_set_cursor_i(scroller, pane_height - 1);
    if (cursor_j + 1 > pane_width)
        scroller_set_cursor_j(scroller, pane_width - 1);
}

void scroller_move_up(RowLinkedScroller *scroller, size_t nlines, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    size_t index_i = scroller->cursor_i + scroller->offset_i;
    if (x > index_i)
        x = index_i;
    if (x > scroller->cursor_i)
    {
        scroller_set_offset_i(scroller, index_i - x);
        scroller_set_cursor_i(scroller, 0);
    }
    else
        scroller_set_cursor_i(scroller, scroller->cursor_i - x);
}

void scroller_move_down(RowLinkedScroller *scroller, size_t nlines, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int pane_height = scroller->h;
    if (pane_height == 0)
        pane_height = 1; // Treat collapsed pane as single row

    size_t index_i = scroller->cursor_i + scroller->offset_i;
    if (x + index_i + 1 >= nlines)
        x = nlines - index_i - 1;
    if (x + scroller->cursor_i + 1 > pane_height)
    {
        x = (x + scroller->cursor_i + 1) - pane_height;
        scroller_set_offset_i(scroller, scroller->offset_i + x);
        scroller_set_cursor_i(scroller, pane_height - 1);
    }
    else
        scroller_set_cursor_i(scroller, scroller->cursor_i + x);
}

void scroller_move_right(RowLinkedScroller *scroller, size_t nlines, size_t line_len, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int pane_width = scroller->ws[active_index];
    if (pane_width == 0)
        pane_width = 1; // Treat collapsed pane as single column

    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;

    // Snap to end
    if (index_j + 1 > line_len)
    {
        index_j = (line_len > 0) ? line_len - 1 : 0;
        if (index_j < offset_j)
        {
            scroller_set_offset_j(scroller, index_j);
            scroller_set_cursor_j(scroller, 0);
        }
        else
            scroller_set_cursor_j(scroller, index_j - offset_j);
    }
    offset_j = scroller->offsets_j[active_index];
    cursor_j = scroller->cursors_j[active_index];
    index_j = offset_j + cursor_j;

    // Move
    if (line_len == 0)
        return;
    if (x + index_j + 1 >= line_len)
        x = line_len - index_j - 1;
    if (x + cursor_j + 1 > pane_width)
    {
        x = (x + cursor_j + 1) - pane_width;
        scroller_set_offset_j(scroller, offset_j + x);
        scroller_set_cursor_j(scroller, pane_width - 1);
    }
    else
        scroller_set_cursor_j(scroller, cursor_j + x);
}

void scroller_move_left(RowLinkedScroller *scroller, size_t nlines, size_t line_len, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;

    // Snap to end
    if (index_j + 1 > line_len)
    {
        index_j = (line_len > 0) ? line_len - 1 : 0;
        if (index_j < offset_j)
        {
            scroller_set_offset_j(scroller, index_j);
            scroller_set_cursor_j(scroller, 0);
        }
        else
            scroller_set_cursor_j(scroller, index_j - offset_j);
    }
    offset_j = scroller->offsets_j[active_index];
    cursor_j = scroller->cursors_j[active_index];
    index_j = offset_j + cursor_j;

    // Move
    if (x > index_j)
        x = index_j;
    if (x > cursor_j)
    {
        scroller_set_offset_j(scroller, index_j - x);
        scroller_set_cursor_j(scroller, 0);
    }
    else
        scroller_set_cursor_j(scroller, cursor_j - x);
}

void scroller_move_page_up(RowLinkedScroller *scroller, size_t nlines, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int pane_height = scroller->h;
    if (pane_height == 0)
        pane_height = 1; // Treat collapsed pane as single row

    size_t x = pane_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (scroller->offset_i == 0)
        return;
    else if (x > scroller->offset_i)
        scroller_set_offset_i(scroller, 0);
    else
        scroller_set_offset_i(scroller, scroller->offset_i - x);
}

void scroller_move_page_down(RowLinkedScroller *scroller, size_t nlines, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int pane_height = scroller->h;
    if (pane_height == 0)
        pane_height = 1; // Treat collapsed pane as single row

    unsigned int x = pane_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (scroller->offset_i + 1 == nlines)
        return;
    else if (scroller->offset_i + x + 1 > nlines)
        scroller_set_offset_i(scroller, nlines - 1);
    else
        scroller_set_offset_i(scroller, scroller->offset_i + x);
    if (scroller->offset_i + scroller->cursor_i + 1 > nlines)
        scroller_set_cursor_i(scroller, nlines - scroller->offset_i - 1);
}

void scroller_move_page_right(RowLinkedScroller *scroller, size_t nlines, size_t maxlen, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int pane_width = scroller->ws[active_index];
    if (pane_width == 0)
        pane_width = 1; // Treat collapsed pane as single column

    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];

    unsigned int x = pane_width;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (offset_j + x + 1 > maxlen)
        scroller_set_offset_j(scroller, maxlen - 1);
    else
        scroller_set_offset_j(scroller, offset_j + x);
    if (cursor_j + scroller->offsets_j[active_index] + 1 > maxlen) // Check for cursor exceeding line end
    {
        size_t index_j = (maxlen > 0) ? maxlen - scroller->offsets_j[active_index] - 1 : 0;
        scroller_set_cursor_j(scroller, index_j);
    }
}

void scroller_move_page_left(RowLinkedScroller *scroller, size_t nlines, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int pane_width = scroller->ws[active_index];
    if (pane_width == 0)
        pane_width = 1; // Treat collapsed pane as single column

    size_t offset_j = scroller->offsets_j[active_index];

    unsigned int x = pane_width;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (x > offset_j)
        scroller_set_offset_j(scroller, 0);
    else
        scroller_set_offset_j(scroller, offset_j - x);
}

void scroller_move_line_start(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    scroller_set_offset_j(scroller, 0);
    scroller_set_cursor_j(scroller, 0);
}

void scroller_move_line_end(RowLinkedScroller *scroller, size_t nlines, size_t line_len)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;
    size_t x = (line_len > 0) ? line_len - 1 - index_j : 0;
    scroller_move_right(scroller, nlines, line_len, x);
}

void scroller_move_first_line(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    scroller_set_offset_i(scroller, 0);
    scroller_set_cursor_i(scroller, 0);
}

void scroller_move_last_line(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    size_t index_i = scroller->cursor_i + scroller->offset_i;
    size_t x = nlines - index_i - 1;
    scroller_move_down(scroller, nlines, x);
}

void scroller_move_to_line(RowLinkedScroller *scroller, size_t nlines, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    size_t index_i = scroller->cursor_i + scroller->offset_i;
    if (x > index_i)
        scroller_move_down(scroller, nlines, x - index_i);
    else if (x < index_i)
        scroller_move_up(scroller, nlines, index_i - x);
}

void scroller_move_bottom_edge(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int pane_height = scroller->h;
    if (pane_height == 0)
        pane_height = 1; // Treat collapsed pane as single row

    if (scroller->offset_i + pane_height > nlines)
        scroller_set_cursor_i(scroller, nlines - scroller->offset_i - 1);
    else
        scroller_set_cursor_i(scroller, pane_height - 1);
}

void scroller_move_top_edge(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    scroller_set_cursor_i(scroller, 0);
}

void scroller_move_left_edge(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    scroller_set_cursor_j(scroller, 0);
}

void scroller_move_right_edge(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int pane_width = scroller->ws[active_index];
    if (pane_width == 0)
        pane_width = 1; // Treat collapsed pane as single column

    scroller_set_cursor_j(scroller, pane_width - 1);
}

void scroller_move_vertical_middle(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int pane_height = scroller->h;
    if (pane_height == 0)
        pane_height = 1; // Treat collapsed pane as single row

    if (scroller->offset_i + pane_height > nlines)
        scroller_set_cursor_i(scroller, (nlines - scroller->offset_i - 1) / 2);
    else
        scroller_set_cursor_i(scroller, (pane_height - 1) / 2);
}

void scroller_move_horizontal_middle(RowLinkedScroller *scroller, size_t nlines)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nlines == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int pane_width = scroller->ws[active_index];
    if (pane_width == 0)
        pane_width = 1; // Treat collapsed pane as single column

    scroller_set_cursor_j(scroller, pane_width / 2);
}
