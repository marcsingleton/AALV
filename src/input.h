#ifndef INPUT_H
#define INPUT_H

/*
 * Input functions
 */

#include "array.h"

int input_get_action(void);
int input_process_action(int action, Array *buffer);
void input_buffer_flush(Array *buffer);
void input_move_up(void);
void input_move_down(void);
void input_move_right(void);
void input_move_left(void);
void input_move_line_start(void);
void input_move_line_end(void);
void input_increase_header_pane_width(void);
void input_decrease_header_pane_width(void);

#endif // INPUT_H
