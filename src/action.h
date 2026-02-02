#ifndef ACTION_H
#define ACTION_H

/*
 * Action functions
 *
 * Functions in this module may modify the global State variable. They should refrain from displaying any changes and
 * should instead mark panes to be refreshed as necessary.
 */

#include <stddef.h>

#include "scroller.h"

typedef union
{
    void (*void_arg)(void);
    void (*size_t_arg)(size_t);
    void (*page_size_arg)(PageSize);
} ActionFunction;

typedef enum
{
    VOID_ARG = 0,
    SIZE_T_ARG,
    PAGE_SIZE_ARG,
} ActionArguments;

typedef struct
{
    ActionFunction fn;
    ActionArguments args;
} Action;

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
void action_move_first_non_gap_or_non_whitespace(void);
void action_move_last_non_gap_or_non_whitespace(void);
void action_move_first_record(void);
void action_move_last_record(void);
void action_move_to_record(size_t x);
void action_move_bottom_edge(void);
void action_move_top_edge(void);
void action_move_left_edge(void);
void action_move_right_edge(void);
void action_move_vertical_middle(void);
void action_move_horizontal_middle(void);
void action_increase_header_sequence_divider(void);
void action_decrease_header_sequence_divider(void);
void action_increase_ruler_records_divider(void);
void action_decrease_ruler_records_divider(void);
void action_increase_tick_spacing(void);
void action_decrease_tick_spacing(void);
void action_enter_command_mode(void);
void action_set_header_pane_active(void);
void action_set_sequence_pane_active(void);

#endif // ACTION_H
