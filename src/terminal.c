#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"

void terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios)
{
    tcgetattr(STDIN_FILENO, old_termios);
    *raw_termios = *old_termios;
    cfmakeraw(raw_termios);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, raw_termios);
}

void terminal_disable_raw_mode(struct termios *old_termios)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, old_termios);
}

void terminal_use_alternate_buffer(void)
{
    write(STDOUT_FILENO, "\x1b[?1049h\x1b[H", 11); // Use alternate buffer and move cursor to top left
}

void terminal_use_normal_buffer(void)
{
    write(STDOUT_FILENO, "\x1b[?1049l", 8);
}

void terminal_cursor_up(void)
{
    write(STDOUT_FILENO, "\x1b[A", 3);
}

void terminal_cursor_down(void)
{
    write(STDOUT_FILENO, "\x1b[B", 3);
}

void terminal_cursor_right(void)
{
    write(STDOUT_FILENO, "\x1b[C", 3);
}

void terminal_cursor_left(void)
{
    write(STDOUT_FILENO, "\x1b[D", 3);
}