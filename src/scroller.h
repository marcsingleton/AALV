#ifndef SCROLLER_H
#define SCROLLER_H

/*
 * Pane scroller
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct
{
    size_t offset_i;
    size_t cursor_i;
    size_t *offsets_j;
    size_t *cursors_j;
    unsigned int npanes;
    unsigned int active_pane_index;
    unsigned int h;
    unsigned int *ws;
    bool *refreshes;
} RowLinkedScroller;

typedef enum
{
    PAGE_SIZE_FULL = 0,
    PAGE_SIZE_HALF
} PageSize;

int scroller_init(RowLinkedScroller *scroller, unsigned int npanes);
void scroller_deinit(RowLinkedScroller *scroller);

void scroller_set_offset_i(RowLinkedScroller *scroller, size_t i);
void scroller_set_cursor_i(RowLinkedScroller *scroller, size_t i);
void scroller_set_offset_j(RowLinkedScroller *scroller, size_t j);
void scroller_set_offsets_j(RowLinkedScroller *scroller, size_t j);
void scroller_set_cursor_j(RowLinkedScroller *scroller, size_t j);
void scroller_set_cursors_j(RowLinkedScroller *scroller, size_t j);
void scroller_set_active_pane(RowLinkedScroller *scroller, unsigned int index);
void scroller_set_refresh(RowLinkedScroller *scroller, bool refresh);
void scroller_set_refreshes(RowLinkedScroller *scroller, bool refresh);

void scroller_cursor_clamp(RowLinkedScroller *scroller);
void scroller_move_up(RowLinkedScroller *scroller, size_t nrows, size_t x);
void scroller_move_down(RowLinkedScroller *scroller, size_t nrows, size_t x);
void scroller_move_right(RowLinkedScroller *scroller, size_t nrows, size_t ncols, size_t x);
void scroller_move_left(RowLinkedScroller *scroller, size_t nrows, size_t ncols, size_t x);
void scroller_move_page_up(RowLinkedScroller *scroller, size_t nrows, PageSize page_size);
void scroller_move_page_down(RowLinkedScroller *scroller, size_t nrows, PageSize page_size);
void scroller_move_page_right(RowLinkedScroller *scroller, size_t nrows, size_t ncols, PageSize page_size);
void scroller_move_page_left(RowLinkedScroller *scroller, size_t nrows, PageSize page_size);
void scroller_move_row_start(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_row_end(RowLinkedScroller *scroller, size_t nrows, size_t ncols);
void scroller_move_to_column(RowLinkedScroller *scroller, size_t nrows, size_t ncols, size_t x);
void scroller_move_first_row(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_last_row(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_to_row(RowLinkedScroller *scroller, size_t nrows, size_t x);
void scroller_move_top_edge(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_bottom_edge(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_right_edge(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_left_edge(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_vertical_middle(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_horizontal_middle(RowLinkedScroller *scroller, size_t nrows);
void scroller_move_row_to_center(RowLinkedScroller *scroller, size_t nrows);

#endif // SCROLLER_H
