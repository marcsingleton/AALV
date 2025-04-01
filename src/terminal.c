#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"

int terminal_get_termios(struct termios *termios_p)
{
    return tcgetattr(STDIN_FILENO, termios_p);
}

int terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios)
{
    *raw_termios = *old_termios;
    cfmakeraw(raw_termios);
    return tcsetattr(STDIN_FILENO, TCSAFLUSH, raw_termios);
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

int terminal_get_window_size(int *rows, int *cols)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
        return -1;
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
