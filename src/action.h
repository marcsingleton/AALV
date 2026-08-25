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
#include "state.h"

typedef union
{
    void (*void_arg)(State *);
    void (*size_t_arg)(State *, size_t);
    void (*page_size_arg)(State *, PageSize);
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

void action_move_up(State *state, size_t x);
void action_move_down(State *state, size_t x);
void action_move_right(State *state, size_t x);
void action_move_left(State *state, size_t x);
void action_move_page_up(State *state, PageSize page_size);
void action_move_page_down(State *state, PageSize page_size);
void action_move_page_right(State *state, PageSize page_size);
void action_move_page_left(State *state, PageSize page_size);
void action_move_line_start(State *state);
void action_move_line_middle(State *state);
void action_move_line_end(State *state);
void action_move_to_column(State *state, size_t x);
void action_move_first_non_gap_or_non_whitespace(State *state);
void action_move_last_non_gap_or_non_whitespace(State *state);
void action_move_first_record(State *state);
void action_move_last_record(State *state);
void action_move_to_record(State *state, size_t x);
void action_move_top_edge(State *state);
void action_move_bottom_edge(State *state);
void action_move_right_edge(State *state);
void action_move_left_edge(State *state);
void action_move_vertical_middle(State *state);
void action_move_horizontal_middle(State *state);
void action_move_line_to_center(State *state);
void action_increase_header_sequence_divider(State *state);
void action_decrease_header_sequence_divider(State *state);
void action_increase_ruler_records_divider(State *state);
void action_decrease_ruler_records_divider(State *state);
void action_increase_tick_spacing(State *state);
void action_decrease_tick_spacing(State *state);
void action_enter_command_mode(State *state);
void action_set_header_pane_active(State *state);
void action_set_sequence_pane_active(State *state);

#endif // ACTION_H
