#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>
#include <unistd.h>

extern int TERMINAL_FILENO;

int terminal_get_termios(struct termios *termios_p);
int terminal_get_window_size(unsigned int *rows, unsigned int *cols);
int terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios);
int terminal_disable_raw_mode(struct termios *old_termios);
void terminal_use_alternate_buffer(void);
void terminal_use_normal_buffer(void);
void terminal_cursor_up(Array *buffer);
void terminal_cursor_down(Array *buffer);
void terminal_cursor_right(Array *buffer);
void terminal_cursor_left(Array *buffer);
void terminal_cursor_origin(Array *buffer);
void terminal_cursor_ij(Array *buffer, const unsigned int i, const unsigned int j);
void terminal_cursor_hide(Array *buffer);
void terminal_cursor_show(Array *buffer);
void terminal_clear_screen(Array *buffer);
void terminal_clear_line(Array *buffer);
void terminal_clear_line_left(Array *buffer);
void terminal_clear_line_right(Array *buffer);

#endif // TERMINAL_H
