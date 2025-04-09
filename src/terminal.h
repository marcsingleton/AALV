#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

int terminal_get_termios(struct termios *termios_p);
int terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios);
void terminal_disable_raw_mode(struct termios *old_termios);
void terminal_use_alternate_buffer(void);
void terminal_use_normal_buffer(void);
void terminal_cursor_up(Array *buffer);
void terminal_cursor_down(Array *buffer);
void terminal_cursor_right(Array *buffer);
void terminal_cursor_left(Array *buffer);
int terminal_get_window_size(int *rows, int *cols);

#endif // TERMINAL_H
