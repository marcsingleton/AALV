#ifndef INPUT_H
#define INPUT_H

/*
 * Input functions
 *
 * Functions in this module may modify the global State variable. They should refrain from displaying any changes and
 * should instead mark panes to be refreshed as necessary.
 */

#include "array.h"

int input_get_action(void);
int input_process_action(int action, Array *buffer);
void input_buffer_flush(Array *buffer);
void input_move_up(unsigned int x);
void input_move_down(unsigned int x);
void input_move_right(unsigned int x);
void input_move_left(unsigned int x);
void input_move_line_start(void);
void input_move_line_end(void);
void input_increase_header_pane_width(void);
void input_decrease_header_pane_width(void);
void input_increase_ruler_pane_height(void);
void input_decrease_ruler_pane_height(void);
void input_increase_tick_spacing(void);
void input_decrease_tick_spacing(void);

#endif // INPUT_H
