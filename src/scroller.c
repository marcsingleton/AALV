#include "scroller.h"

int scroller_init(RowLinkedScroller *scroller, unsigned int npanes)
{
    if (!scroller || npanes == 0)
        return -1;

    size_t *offsets_i = NULL;
    size_t *offsets_j = NULL;
    size_t *cursors_i = NULL;
    size_t *cursors_j = NULL;
    unsigned int *groups_i = NULL;
    unsigned int *groups_j = NULL;
    unsigned int *heights = NULL;
    unsigned int *widths = NULL;
    bool *refreshes = NULL;

    offsets_i = calloc(npanes, sizeof(size_t));
    offsets_j = calloc(npanes, sizeof(size_t));
    cursors_i = calloc(npanes, sizeof(size_t));
    cursors_j = calloc(npanes, sizeof(size_t));
    groups_i = calloc(npanes, sizeof(unsigned int));
    groups_j = calloc(npanes, sizeof(unsigned int));
    heights = calloc(npanes, sizeof(unsigned int));
    widths = calloc(npanes, sizeof(unsigned int));
    refreshes = calloc(npanes, sizeof(bool));
    if (!offsets_i || !offsets_j ||
        !cursors_i || !cursors_j ||
        !groups_i || !groups_j ||
        !heights || !widths ||
        !refreshes)
    {
        free(offsets_i);
        free(offsets_j);
        free(cursors_i);
        free(cursors_j);
        free(groups_i);
        free(groups_j);
        free(heights);
        free(widths);
        free(refreshes);
        return -1;
    }

    for (unsigned int index = 0; index < npanes; index++)
        groups_i[index] = index;
    for (unsigned int index = 0; index < npanes; index++)
        groups_j[index] = index;

    scroller->offsets_i = offsets_i;
    scroller->offsets_j = offsets_j;
    scroller->cursors_i = cursors_i;
    scroller->cursors_j = cursors_j;
    scroller->groups_i = groups_i;
    scroller->groups_j = groups_j;
    scroller->heights = heights;
    scroller->widths = widths;
    scroller->refreshes = refreshes;
    scroller->npanes = npanes;
    scroller->active_pane_index = 0;

    return 0;
}

void scroller_deinit(RowLinkedScroller *scroller)
{
    if (!scroller)
        return;

    free(scroller->offsets_i);
    free(scroller->offsets_j);
    free(scroller->cursors_i);
    free(scroller->cursors_j);
    free(scroller->groups_i);
    free(scroller->groups_j);
    free(scroller->heights);
    free(scroller->widths);
    free(scroller->refreshes);

    scroller->offsets_i = NULL;
    scroller->offsets_j = NULL;
    scroller->cursors_i = NULL;
    scroller->cursors_j = NULL;
    scroller->groups_i = NULL;
    scroller->groups_j = NULL;
    scroller->heights = NULL;
    scroller->widths = NULL;
    scroller->refreshes = NULL;
    scroller->npanes = 0;
    scroller->active_pane_index = 0;
}

void scroller_set_offset_i(RowLinkedScroller *scroller, size_t i)
{
    if (!scroller)
        return;

    unsigned int group = scroller->groups_i[scroller->active_pane_index];
    for (unsigned int index = 0; index < scroller->npanes; index++)
    {
        if (scroller->groups_i[index] != group)
            continue;
        scroller->offsets_i[index] = i;
        scroller->refreshes[index] = true;
    }
}

void scroller_set_offset_j(RowLinkedScroller *scroller, size_t j)
{
    if (!scroller)
        return;

    unsigned int group = scroller->groups_j[scroller->active_pane_index];
    for (unsigned int index = 0; index < scroller->npanes; index++)
    {
        if (scroller->groups_j[index] != group)
            continue;
        scroller->offsets_j[index] = j;
        scroller->refreshes[index] = true;
    }
}

void scroller_set_cursor_i(RowLinkedScroller *scroller, size_t i)
{
    if (!scroller)
        return;

    unsigned int group = scroller->groups_i[scroller->active_pane_index];
    for (unsigned int index = 0; index < scroller->npanes; index++)
    {
        if (scroller->groups_i[index] == group)
            scroller->cursors_i[index] = i;
    }
}

void scroller_set_cursor_j(RowLinkedScroller *scroller, size_t j)
{
    if (!scroller)
        return;

    unsigned int group = scroller->groups_j[scroller->active_pane_index];
    for (unsigned int index = 0; index < scroller->npanes; index++)
    {
        if (scroller->groups_j[index] == group)
            scroller->cursors_j[index] = j;
    }
}

void scroller_set_group_i(RowLinkedScroller *scroller, unsigned int i)
{
    if (!scroller)
        return;

    scroller->groups_i[scroller->active_pane_index] = i;
}

void scroller_set_group_j(RowLinkedScroller *scroller, unsigned int j)
{
    if (!scroller)
        return;

    scroller->groups_j[scroller->active_pane_index] = j;
}

void scroller_set_height(RowLinkedScroller *scroller, unsigned int height)
{
    if (!scroller)
        return;

    unsigned int index = scroller->active_pane_index;
    scroller->heights[index] = height;
    scroller->refreshes[index] = true;
}

void scroller_set_width(RowLinkedScroller *scroller, unsigned int width)
{
    if (!scroller)
        return;

    unsigned int index = scroller->active_pane_index;
    scroller->widths[index] = width;
    scroller->refreshes[index] = true;
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

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    unsigned int scroller_width = scroller->widths[active_index];
    if (scroller_width == 0)
        scroller_width = 1; // Treat collapsed scroller as single column

    size_t cursor_i = scroller->cursors_i[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];

    if (cursor_i + 1 > scroller_height)
        scroller_set_cursor_i(scroller, scroller_height - 1);
    if (cursor_j + 1 > scroller_width)
        scroller_set_cursor_j(scroller, scroller_width - 1);
}

void scroller_move_up(RowLinkedScroller *scroller, size_t nrows, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_i = scroller->offsets_i[active_index];
    size_t cursor_i = scroller->cursors_i[active_index];
    size_t index_i = cursor_i + offset_i;

    if (x > index_i)
        x = index_i;
    if (x > cursor_i)
    {
        scroller_set_offset_i(scroller, index_i - x);
        scroller_set_cursor_i(scroller, 0);
    }
    else
        scroller_set_cursor_i(scroller, cursor_i - x);
}

void scroller_move_down(RowLinkedScroller *scroller, size_t nrows, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    size_t offset_i = scroller->offsets_i[active_index];
    size_t cursor_i = scroller->cursors_i[active_index];
    size_t index_i = cursor_i + offset_i;

    if (x + index_i + 1 >= nrows)
        x = nrows - index_i - 1;
    if (x + cursor_i + 1 > scroller_height)
    {
        x = (x + cursor_i + 1) - scroller_height;
        scroller_set_offset_i(scroller, offset_i + x);
        scroller_set_cursor_i(scroller, scroller_height - 1);
    }
    else
        scroller_set_cursor_i(scroller, cursor_i + x);
}

void scroller_move_right(RowLinkedScroller *scroller, size_t nrows, size_t ncols, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_width = scroller->widths[active_index];
    if (scroller_width == 0)
        scroller_width = 1; // Treat collapsed scroller as single column

    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;

    // Snap to end
    if (index_j + 1 > ncols)
    {
        index_j = (ncols > 0) ? ncols - 1 : 0;
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
    if (ncols == 0)
        return;
    if (x + index_j + 1 >= ncols)
        x = ncols - index_j - 1;
    if (x + cursor_j + 1 > scroller_width)
    {
        x = (x + cursor_j + 1) - scroller_width;
        scroller_set_offset_j(scroller, offset_j + x);
        scroller_set_cursor_j(scroller, scroller_width - 1);
    }
    else
        scroller_set_cursor_j(scroller, cursor_j + x);
}

void scroller_move_left(RowLinkedScroller *scroller, size_t nrows, size_t ncols, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;

    // Snap to end
    if (index_j + 1 > ncols)
    {
        index_j = (ncols > 0) ? ncols - 1 : 0;
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

void scroller_move_page_up(RowLinkedScroller *scroller, size_t nrows, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    size_t offset_i = scroller->offsets_i[active_index];
    size_t x = scroller_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (offset_i == 0)
        return;
    else if (x > offset_i)
        scroller_set_offset_i(scroller, 0);
    else
        scroller_set_offset_i(scroller, offset_i - x);
}

void scroller_move_page_down(RowLinkedScroller *scroller, size_t nrows, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    size_t offset_i = scroller->offsets_i[active_index];
    size_t cursor_i = scroller->cursors_i[active_index];

    size_t x = scroller_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (offset_i + 1 == nrows)
        return;
    else if (offset_i + x + 1 > nrows)
        scroller_set_offset_i(scroller, nrows - 1);
    else
        scroller_set_offset_i(scroller, offset_i + x);

    offset_i = scroller->offsets_i[active_index];
    if (offset_i + cursor_i + 1 > nrows)
        scroller_set_cursor_i(scroller, nrows - offset_i - 1);
}

void scroller_move_page_right(RowLinkedScroller *scroller, size_t nrows, size_t ncols, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_width = scroller->widths[active_index];
    if (scroller_width == 0)
        scroller_width = 1; // Treat collapsed scroller as single column

    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];

    unsigned int x = scroller_width;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (offset_j + x + 1 > ncols)
        scroller_set_offset_j(scroller, (ncols > 0) ? ncols - 1 : 0);
    else
        scroller_set_offset_j(scroller, offset_j + x);

    offset_j = scroller->offsets_i[active_index];
    if (cursor_j + offset_j + 1 > ncols) // Check for cursor exceeding ncols
    {
        size_t index_j = (ncols > 0) ? ncols - offset_j - 1 : 0;
        scroller_set_cursor_j(scroller, index_j);
    }
}

void scroller_move_page_left(RowLinkedScroller *scroller, size_t nrows, PageSize page_size)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_width = scroller->widths[active_index];
    if (scroller_width == 0)
        scroller_width = 1; // Treat collapsed scroller as single column

    size_t offset_j = scroller->offsets_j[active_index];

    unsigned int x = scroller_width;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (x > offset_j)
        scroller_set_offset_j(scroller, 0);
    else
        scroller_set_offset_j(scroller, offset_j - x);
}

void scroller_move_row_start(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    scroller_set_offset_j(scroller, 0);
    scroller_set_cursor_j(scroller, 0);
}

void scroller_move_row_end(RowLinkedScroller *scroller, size_t nrows, size_t ncols)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;

    size_t x = (ncols > 0) ? ncols - 1 - index_j : 0;
    scroller_move_right(scroller, nrows, ncols, x);
}

void scroller_move_to_column(RowLinkedScroller *scroller, size_t nrows, size_t ncols, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_j = scroller->offsets_j[active_index];
    size_t cursor_j = scroller->cursors_j[active_index];
    size_t index_j = offset_j + cursor_j;

    if (x > index_j)
        scroller_move_right(scroller, nrows, ncols, x - index_j);
    else if (x < index_j)
        scroller_move_left(scroller, nrows, ncols, index_j - x);
}

void scroller_move_first_row(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    scroller_set_offset_i(scroller, 0);
    scroller_set_cursor_i(scroller, 0);
}

void scroller_move_last_row(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_i = scroller->offsets_i[active_index];
    size_t cursor_i = scroller->cursors_i[active_index];
    size_t index_i = cursor_i + offset_i;

    size_t x = nrows - index_i - 1;
    scroller_move_down(scroller, nrows, x);
}

void scroller_move_to_row(RowLinkedScroller *scroller, size_t nrows, size_t x)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    size_t offset_i = scroller->offsets_i[active_index];
    size_t cursor_i = scroller->cursors_i[active_index];
    size_t index_i = cursor_i + offset_i;

    if (x > index_i)
        scroller_move_down(scroller, nrows, x - index_i);
    else if (x < index_i)
        scroller_move_up(scroller, nrows, index_i - x);
}

void scroller_move_top_edge(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    scroller_set_cursor_i(scroller, 0);
}

void scroller_move_bottom_edge(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    size_t offset_i = scroller->offsets_i[active_index];
    if (offset_i + scroller_height > nrows)
        scroller_set_cursor_i(scroller, nrows - offset_i - 1);
    else
        scroller_set_cursor_i(scroller, scroller_height - 1);
}

void scroller_move_right_edge(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_width = scroller->widths[active_index];
    if (scroller_width == 0)
        scroller_width = 1; // Treat collapsed scroller as single column

    scroller_set_cursor_j(scroller, scroller_width - 1); // Not (scroller_width - 1) b/c column positions intrinsically offset
}

void scroller_move_left_edge(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    scroller_set_cursor_j(scroller, 0);
}

void scroller_move_vertical_middle(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    size_t offset_i = scroller->offsets_i[active_index];
    if (offset_i + scroller_height > nrows)
        scroller_set_cursor_i(scroller, (nrows - offset_i - 1) / 2);
    else
        scroller_set_cursor_i(scroller, (scroller_height - 1) / 2);
}

void scroller_move_horizontal_middle(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_width = scroller->widths[active_index];
    if (scroller_width == 0)
        scroller_width = 1; // Treat collapsed scroller as single column

    scroller_set_cursor_j(scroller, scroller_width / 2); // Not (scroller_width - 1) b/c column positions intrinsically offset
}

void scroller_move_row_to_center(RowLinkedScroller *scroller, size_t nrows)
{
    if (!scroller)
        return;
    scroller_cursor_clamp(scroller);

    if (nrows == 0)
        return;

    unsigned int active_index = scroller->active_pane_index;
    unsigned int scroller_height = scroller->heights[active_index];
    if (scroller_height == 0)
        scroller_height = 1; // Treat collapsed scroller as single row

    size_t cursor_i = scroller->cursors_i[active_index];
    size_t offset_i = scroller->offsets_i[active_index];
    size_t index_i = cursor_i + offset_i;

    if (index_i < (scroller_height - 1) / 2)
        return;

    if (cursor_i > (scroller_height - 1) / 2)
        offset_i += cursor_i - (scroller_height - 1) / 2;
    else
        offset_i -= (scroller_height - 1) / 2 - cursor_i; // Above index_i check ensures this is non-negative
    scroller_set_offset_i(scroller, offset_i);
    scroller_set_cursor_i(scroller, (scroller_height - 1) / 2);
}