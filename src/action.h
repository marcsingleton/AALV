#ifndef ACTION_H
#define ACTION_H

/*
 * Action functions
 *
 * Functions in this module may modify the global State variable. They should refrain from displaying any changes and
 * should instead mark panes to be refreshed as necessary.
 */

#include <stddef.h>

typedef enum
{
    PAGE_SIZE_FULL,
    PAGE_SIZE_HALF
} PageSize;

void action_next_file(void);
void action_previous_file(void);
void action_move_up(size_t x);
void action_move_down(size_t x);
void action_move_right(size_t x);
void action_move_left(size_t x);
void action_move_page_up(PageSize page_size);
void action_move_page_down(PageSize page_size);
void action_move_page_right(PageSize page_size);
void action_move_page_left(PageSize page_size);
void action_move_line_start(void);
void action_move_line_end(void);
void action_move_first_record(void);
void action_move_last_record(void);
void action_move_to_record(size_t x);
void action_move_bottom_edge(void);
void action_move_top_edge(void);
void action_move_left_edge(void);
void action_move_right_edge(void);
void action_move_vertical_middle(void);
void action_move_horizontal_middle(void);
void action_increase_header_pane_width(void);
void action_decrease_header_pane_width(void);
void action_increase_ruler_pane_height(void);
void action_decrease_ruler_pane_height(void);
void action_increase_tick_spacing(void);
void action_decrease_tick_spacing(void);

#endif // ACTION_H
