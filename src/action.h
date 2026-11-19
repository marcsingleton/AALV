#ifndef ACTION_H
#define ACTION_H

/*
 * Action implementations
 *
 * Functions in this module may modify the global State variable. They should refrain from displaying any changes and
 * should instead mark panes to be refreshed as necessary.
 *
 * An int argument are taken directly from the parsing accumulator. Each function should interpret the meaning of this
 * value accordingly.
 */

#include <stddef.h>

#include "scroller.h"
#include "state.h"

typedef union
{
    void (*void_arg)(State *);
    void (*int_arg)(State *, int);
    void (*page_size_arg)(State *, PageSize);
} ActionFunction;

typedef enum
{
    VOID_ARG = 0,
    INT_ARG,
} ActionArguments;

typedef struct
{
    ActionFunction fn;
    ActionArguments args;
} Action;

void action_move_up(State *state, int x);
void action_move_down(State *state, int x);
void action_move_right(State *state, int x);
void action_move_left(State *state, int x);
void action_move_page_up(State *state);
void action_move_half_page_up(State *state);
void action_move_page_down(State *state);
void action_move_half_page_down(State *state);
void action_move_page_right(State *state);
void action_move_half_page_right(State *state);
void action_move_page_left(State *state);
void action_move_half_page_left(State *state);
void action_move_first_column(State *state);
void action_move_middle_column(State *state);
void action_move_last_column(State *state);
void action_move_to_column(State *state, int x);
void action_move_first_non_gap_or_non_whitespace(State *state);
void action_move_last_non_gap_or_non_whitespace(State *state);
void action_move_first_row(State *state);
void action_move_last_row(State *state);
void action_move_to_row(State *state, int x);
void action_move_top_edge(State *state);
void action_move_bottom_edge(State *state);
void action_move_right_edge(State *state);
void action_move_left_edge(State *state);
void action_move_vertical_middle(State *state);
void action_move_horizontal_middle(State *state);
void action_move_row_to_center(State *state);
void action_increase_header_sequence_divider(State *state);
void action_decrease_header_sequence_divider(State *state);
void action_increase_ruler_records_divider(State *state);
void action_decrease_ruler_records_divider(State *state);
void action_increase_tick_spacing(State *state);
void action_decrease_tick_spacing(State *state);
void action_enter_command_mode(State *state);
void action_set_header_pane_active(State *state);
void action_set_sequence_pane_active(State *state);
void action_raise_sigtstp(State *state);

#endif // ACTION_H
