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
    size_t *offsets_i;
    size_t *offsets_j;
    size_t *cursors_i;
    size_t *cursors_j;
    unsigned int *groups_i;
    unsigned int *groups_j;
    unsigned int *heights;
    unsigned int *widths;
    bool *refreshes;
    unsigned int npanes;
    unsigned int active_pane_index;
} LinkedScroller;

typedef enum
{
    PAGE_SIZE_FULL = 0,
    PAGE_SIZE_HALF
} PageSize;

int scroller_init(LinkedScroller *scroller, unsigned int npanes);
void scroller_deinit(LinkedScroller *scroller);

size_t scroller_get_offset_i(const LinkedScroller *scroller);
size_t scroller_get_offset_j(const LinkedScroller *scroller);
size_t scroller_get_cursor_i(const LinkedScroller *scroller);
size_t scroller_get_cursor_j(const LinkedScroller *scroller);
size_t scroller_get_index_i(const LinkedScroller *scroller);
size_t scroller_get_index_j(const LinkedScroller *scroller);
unsigned int scroller_get_height(const LinkedScroller *scroller);
unsigned int scroller_get_width(const LinkedScroller *scroller);

void scroller_set_offset_i(LinkedScroller *scroller, size_t i);
void scroller_set_offset_j(LinkedScroller *scroller, size_t j);
void scroller_set_cursor_i(LinkedScroller *scroller, size_t i);
void scroller_set_cursor_j(LinkedScroller *scroller, size_t j);
void scroller_set_group_i(LinkedScroller *scroller, unsigned int i);
void scroller_set_group_j(LinkedScroller *scroller, unsigned int j);
void scroller_set_height(LinkedScroller *scroller, unsigned int height);
void scroller_set_width(LinkedScroller *scroller, unsigned int width);
void scroller_set_active_pane(LinkedScroller *scroller, unsigned int index);
void scroller_set_refresh(LinkedScroller *scroller, bool refresh);
void scroller_set_refreshes(LinkedScroller *scroller, bool refresh);

void scroller_cursor_clamp_viewport(LinkedScroller *scroller);
void scroller_cursor_clamp_row(LinkedScroller *scroller, size_t ncols);

void scroller_move_up(LinkedScroller *scroller, size_t nrows, size_t x);
void scroller_move_down(LinkedScroller *scroller, size_t nrows, size_t x);
void scroller_move_right(LinkedScroller *scroller, size_t nrows, size_t ncols, size_t x);
void scroller_move_left(LinkedScroller *scroller, size_t nrows, size_t ncols, size_t x);
void scroller_move_page_up(LinkedScroller *scroller, size_t nrows, PageSize page_size);
void scroller_move_page_down(LinkedScroller *scroller, size_t nrows, PageSize page_size);
void scroller_move_page_right(LinkedScroller *scroller, size_t nrows, size_t ncols, PageSize page_size);
void scroller_move_page_left(LinkedScroller *scroller, size_t nrows, PageSize page_size);
void scroller_move_row_start(LinkedScroller *scroller, size_t nrows);
void scroller_move_row_end(LinkedScroller *scroller, size_t nrows, size_t ncols);
void scroller_move_to_column(LinkedScroller *scroller, size_t nrows, size_t ncols, size_t x);
void scroller_move_first_row(LinkedScroller *scroller, size_t nrows);
void scroller_move_last_row(LinkedScroller *scroller, size_t nrows);
void scroller_move_to_row(LinkedScroller *scroller, size_t nrows, size_t x);
void scroller_move_top_edge(LinkedScroller *scroller, size_t nrows);
void scroller_move_bottom_edge(LinkedScroller *scroller, size_t nrows);
void scroller_move_right_edge(LinkedScroller *scroller, size_t nrows);
void scroller_move_left_edge(LinkedScroller *scroller, size_t nrows);
void scroller_move_vertical_middle(LinkedScroller *scroller, size_t nrows);
void scroller_move_horizontal_middle(LinkedScroller *scroller, size_t nrows);
void scroller_move_row_to_center(LinkedScroller *scroller, size_t nrows);

#endif // SCROLLER_H